#include "myevic.h"
#include "dataflash.h"
#include "timers.h"
#include "display.h"


//=========================================================================
//----- (00005714) --------------------------------------------------------
__myevic__ void SSD1306_Refresh()
{
	uint8_t *sb;

	sb = ScreenBuffer;

	for ( int i = 0 ; i < 16 ; ++i )
	{
		DisplaySendCommand( 0xB0 + i );
		DisplaySendCommand( 0 );
		DisplaySendCommand( ( dfStatus.flipped ) ? 0x12 : 0x10 );
                
		DisplaySendData( sb, 64 ); //64 * 16 = 1024 = 64 * 128 / 8 ;bytes
		sb += 64;
	}
}


//=========================================================================
//----- (000055D8) --------------------------------------------------------
__myevic__ void SSD1306_ClearBuffer()
{
	uint8_t *v0;
	int v1;
	int v2;

	v0 = ScreenBuffer;
	v1 = 0;
	do
	{
		v2 = 0;
		do
		{
			++v2;
			*v0++ = 0;
		}
		while ( v2 < 0x40 );
		++v1;
	}
	while ( v1 < 0x10 );
}


//=========================================================================
//----- (00005594) --------------------------------------------------------
__myevic__ void SSD1306_CLS()
{
	SSD1306_ClearBuffer();
	SSD1306_Refresh();
}


//=========================================================================
//----- (000055A4) --------------------------------------------------------
__myevic__ void SSD1306_PowerOn()
{
	PA1 = 1;
	PC4 = 1;
	WaitOnTMR2( 1 );
	PA0 = 0;
	WaitOnTMR2( 1 );
	PA0 = 1;
	WaitOnTMR2( 10 );
}


//=========================================================================

const uint8_t SSD1306_InitSeq[] =
	{	0xAE, //Set Display ON/OFF (AFh/AEh) 
		0xA8, //Set Multiplex Ratio...
		0x3F, //...to 63 (all scr visible)
		0xD5, //Set Display Clock Divide Ratio/Oscillator Frequency, SETDISPLAYCLOCKDIV
		0xF1, // ... to
		0xC8, // Set COM Output Scan Direction (C0h/C8h) vertical flip / no flip
		0xD3, // Set Display Offset ...
		0x20, // ... to
		0xDC,
		0x00, //---set low column address ? ---set high column address? --set start line address ?
		0x20, //Set Memory Addressing Mode ?
		0x81, // Set Contrast (0~255) ...
		0x2F, // ... to
		0xA1, //Set Segment Re-map (A0h/A1h) hor flip / no flip
		//0xA4, //def //Entire Display ON, DISPLAYALLON_RESUME,  Output follows RAM content 
		0xA6, //Set Normal/Inverse Display (A6h/A7h)
		//0xAD, // for SH1106 SH1106_SET_PUMP_MODE 0XAD
		//0x8A, // SH1106_PUMP_OFF 0X8A
		//0xD9, //def //Set Pre-charge Period
		//0x22, //DCLK for phase 1 and 2 
        //0xD9,
        //0x2F,
		0xDB, // Set VCOMH Deselect Level, adjusts the VCOMH regulator output, SETVCOMDETECT
	        0x30 //35? 00h 20h_def 30h // 00 - darker but artefacts when lighter
        };
//----- (00005530) --------------------------------------------------------
__myevic__ void SSD1306_Init()
{
	SSD1306_PowerOn();

	for ( int i = 0 ; i < sizeof( SSD1306_InitSeq ) ; ++i )
	{
		DisplaySendCommand( SSD1306_InitSeq[i] );
	}

	if ( dfStatus.flipped )
	{
		DisplaySendCommand( 0xC0 ); // Set COM Output Scan Direction (C0h/C8h) vert flip
		DisplaySendCommand( 0xD3 ); // Set Display Offset ...
		DisplaySendCommand( 0x60 ); // ... to
		DisplaySendCommand( 0xDC );
		DisplaySendCommand( 0x20 );
		DisplaySendCommand( 0xA0 ); //Set Segment Re-map (A0h/A1h) hor flip
	}

	SSD1306_CLS();
	DisplaySendCommand( 0xAF ); //Display On
	WaitOnTMR2( 20 );
}


//=========================================================================
__myevic__ void SSD1306_SetContrast( const uint8_t c )
{
	DisplaySendCommand( 0x81 );
	DisplaySendCommand( c );
}


//=========================================================================
__myevic__ void SSD1306_SetInverse( const uint8_t i )
{
	DisplaySendCommand( i ? 0xA7 : 0xA6 );
}


//=========================================================================
//----- (000056E0) --------------------------------------------------------
__myevic__ void SSD1306_ScreenOff()
{
	DisplaySendCommand( 0xAE );
	PC4 = 0;
	WaitOnTMR2( 100 );
	PA1 = 0;
	WaitOnTMR2( 100 );
	PA0 = 0;
	WaitOnTMR2( 100 );
}


//=========================================================================
//----- (00005500) --------------------------------------------------------
__myevic__ void SSD1306_Plot( int x, int y, int color )
{
    //for DrawPoint()
    
	uint8_t mask;
	uint32_t i;

	if (( x < 0 ) || ( x >  63 )) return;
	if (( y < 0 ) || ( y > 127 )) return;

	mask = 1 << ( y & 7 );
	i = x + ( ( y & ~7 ) << 3 );

	if ( color == 1 )
	{
		ScreenBuffer[i] |= mask;
	}
	else if ( color == 0 )
	{
		ScreenBuffer[i] &= ~mask;
	}
	else
	{
		ScreenBuffer[i] ^= mask;
	}
}


//=========================================================================
//----- (000055FC) --------------------------------------------------------
__myevic__ uint32_t SSD1306_Image( int x, int y, uint8_t img, int color )
{
	if ( img == 0x88 || img == 0x8B || img == 0x91 || img == 0x92 || img == 0x9A )
	{
		y += 2;
	}
	return SSD1306_Bitmap( x, y, Images[img - 1], color );
}


//=========================================================================
//----- (00005628) --------------------------------------------------------
__myevic__ uint32_t SSD1306_Bitmap( int x, int y, const image_t *image, int color )
{
	uint32_t shift;
	uint32_t h, w;
	uint32_t bm_ptr;
	uint32_t addr;
	uint32_t lines;
	uint8_t pixels;
        //const uint8_t ByteMaskRight[] = { 0x00, 0x01, 0x03,	0x07, 0x0F, 0x1F, 0x3F,	0x7F };
        //const uint8_t ByteMaskLeft[]  = { 0xFF, 0xFE, 0xFC,	0xF8, 0xF0, 0xE0, 0xC0,	0x80 };
        //#define SCREEN_BUFFER_SIZE 0x400
	shift = y & 7; // 0...7

	bm_ptr = 0;

	lines = image->height >> 3; //  /8 , in bytes

	for ( h = 0 ; h < lines ; ++h )
	{
		addr = 64 * ( ( y >> 3 ) + h ) + x;

		for ( w = 0 ; w < image->width ; ++w )
		{
			pixels = image->bitmap[bm_ptr++]; //byte
                                               
                        //pixels = ( ( pixels >> 1 ) & 0xF8 ) | ( pixels & 7 ) ; shift test 3 6 8 bad
                        
			if ( color ) pixels = ~pixels; //inverse

			if ( shift )
			{
				if ( addr < SCREEN_BUFFER_SIZE )
				{
					ScreenBuffer[ addr ] &= ByteMaskRight[shift];
					ScreenBuffer[ addr ] |= ( pixels << shift ) & ByteMaskLeft[shift];
				}
				if ( addr + 64 < SCREEN_BUFFER_SIZE )
				{
					ScreenBuffer[ addr + 64 ] &= ByteMaskLeft[shift];
					ScreenBuffer[ addr + 64 ] |= ( pixels >> ( 8 - shift )) & ByteMaskRight[shift];
				}
			}
			else
			{
				if ( addr < SCREEN_BUFFER_SIZE )
				{
					ScreenBuffer[ addr ] = pixels;
				}
			}

			++addr;
		}
	}

	return image->width;
        
// 4 12x16
// {12,16,{0,0,0,192,240,124,31,255,255,255,0,0, 0,28,31,31,25,24,24,255,255,255,24,0}};
// 
// 00000000
// 00000000
// 00000000
// 11000000
// 11110000
// 01111100
// 00011111
// 11111111
// 11111111
// 11111111
// 00000000
// 00000000
//          \          =
// 00000000 00000000   ......1111..
// 00011100 00000000   ......1111..
// 00011111 00000000   .....11111..
// 00011111 11000000   .....11111..
// 00011001 11110000   ....111111..
// 00011000 01111100   ....11.111..
// 00011000 00011111   ...111.111..
// 11111111 11111111   ...11..111..
// 11111111 11111111   ..111..111..
// 11111111 11111111   ..11...111..
// 00011000 00000000   .111...111..
// 00000000 00000000   .1111111111.
//                     .1111111111.
//                     .......111..
//                     .......111..
//                     .......111..

              
}


//=========================================================================
//----- (000064C8) --------------------------------------------------------
__myevic__ void SSD1306_WriteBytes( const int isData, const uint8_t data[], const int len )
{
    //PERIPH_BASE          (0x40000000UL)  /*!< (Peripheral) Base Address */
    //APBPERIPH_BASE       (PERIPH_BASE + 0x00040000)
    //SPI0_BASE            (APBPERIPH_BASE + 0x20000)
    // SPI0 = SPI0_BASE = 40060000
    
	register int is_data = ( isData == 0x40 );
	register int byte;

	PE10 = is_data ? 1 : 0;

	for ( int i = 0 ; i < len ; ++i )
	{
		byte = data[i];
		while ( SPI_IS_BUSY( SPI0 ) )
			;
		SPI_WRITE_TX( SPI0, byte );
	}
	while ( SPI_IS_BUSY( SPI0 ) )
		;
}


//=========================================================================
__myevic__ void SSD1306_Screen2Bitmap( uint8_t *pu8Bitmap )
{
	MemClear( pu8Bitmap, SCREEN_BUFFER_SIZE );

	for ( int line = 0 ; line < 16 ; ++line )
	{
		for ( int bit = 0 ; bit < 8 ; ++bit )
		{
			int y = line * 8 + bit;
			int mask = 1 << bit;
			for ( int x = 0 ; x < 64 ; ++x )
			{
				if ( ScreenBuffer[ line * 64 + x ] & mask )
				{
					pu8Bitmap[ y * 8 + ( x >> 3 ) ] |= ( 128 >> ( x & 7 ) );
				}
			}
		}
	}
}

