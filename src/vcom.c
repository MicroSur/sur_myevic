//#include "vcom.h"
#include "myevic.h"

//=========================================================================
// VCOM
//-------------------------------------------------------------------------

#define RXBUFSIZE	512 /* RX buffer size */
volatile uint8_t comRbuf[RXBUFSIZE];
volatile uint16_t comRbytes = 0;
volatile uint16_t comRtail = 0;

//-------------------------------------------------------------------------
// need for USB in games but WHY? ///
__myevic__ void NeedHelp( uint8_t c ) //VCOM_Cout
{
	__set_PRIMASK(1);
	comRbuf[comRtail++] = c;
	if ( comRtail >= RXBUFSIZE )
		comRtail = 0;

	++comRbytes;
	__set_PRIMASK(0);
}
