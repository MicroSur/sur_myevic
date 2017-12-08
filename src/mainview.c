#include "myevic.h"
#include "dataflash.h"
#include "screens.h"
#include "display.h"
#include "battery.h"
#include "myrtc.h"
#include "atomizer.h"
#include "miscs.h"
#include "timers.h"
#include "events.h"

//=============================================================================

uint16_t	BypassVolts;
uint8_t		HideLogo;
//uint8_t		ShowProfNum;
uint8_t		SplashTimer;


//=============================================================================
//----- (00001654) --------------------------------------------------------
__myevic__ void MainView()
{
	if ( !dfStatus.off )
	{
		if ( Screen != 1 || !PD3 || !PD2 || !PE0 )
		{
			HideLogo = dfHideLogo;
		}
		Screen = 1;
		ScreenDuration = GetMainScreenDuration();
                gFlags.screen_on = 1;
                //gFlags.asleep = 0;
	}
	else
	{
		if ( gFlags.battery_charging && dfStealthOn != 1 )
		{
			ChargeView();
		}
		else
		{
			Screen = 0;
			SleepTimer = 0;
		}
	}
	gFlags.refresh_display = 1;
}


//=============================================================================
__myevic__ void DrawMode()
{
	if ( !BLINKITEM(0) )
	{
		switch ( dfMode )
		{
			case 0:
				DrawString( String_TEMP, 10, 0 );
				break;
			case 1:
				DrawString( String_TEMP, 10, 0 );
				break;
			case 2:
				DrawString( String_TEMP, 10, 0 );
				break;
			case 3:
				DrawString( String_TCR, 1, 0 );
				break;
			case 4:
				DrawStringCentered( String_POWER, 0 );
				break;
			case 5:
				DrawStringCentered( String_BYPASS, 0 );
				break;
			case 6:
				DrawStringCentered( String_SMART, 0 );
				break;
			default:
				break;
		}
	}

	if ( !BLINKITEM(1) )
	{
		switch ( dfMode )
		{
			case 0:
				DrawString( String_NI, 42, 0 );
				break;
			case 1:
				DrawString( String_TI, 42, 0 );
				break;
			case 2:
				DrawString( String_SS, 42 , 0 );
				//DrawImage( 48, 3, 0x04 );
				//DrawImage( 54, 3, 0x02 );
				//DrawImage( 59, 3, 0x07 );
				break;
			case 3:
				DrawValue( 26, 0, dfTCRM[dfTCRIndex], 0, 0x0B, 3 );
				DrawImage( 49, 0, 0xA8 );
				DrawValue( 57, 0, dfTCRIndex + 1, 0, 0x0B, 1 );
				break;
			default:
				break;
		}
	}
}

//=============================================================================

__myevic__ void DrawPwrLine( int pwr, int line )
{
	if ( BLINKITEM(2) && PD2 && PD3 )
		return;
        
        int fset, x, y, yoff;
        fset = 0x1F;
        y = line;
        x = 55;
        yoff = 2;
        
        if ( dfUIVersion == 1 )
        {
            fset = 0x29;
            y += 3;
            x = 63;
            yoff = 8;
        }
        
        DrawString( String_PWR_s, 0, y+yoff );

	if ( pwr < 1000 ) // < 100 w
	{
		DrawValueRight( x, y, pwr, 1, fset, 0 );
		//DrawImage( x+1, y+yoff, 0x98 );
	}
	else // > 99 w
	{
		//if ( EditModeTimer && ( EditItemIndex == 2 ) && ( !PD2 || !PD3 ) && !dfStatus.onewatt )
		//{
		//	DrawValueRight( 64, line, pwr, 1, 0x1F, 0 ); //100.0
		//}
		//else
		//{
			DrawValueRight( x, y, pwr / 10, 0, fset, 3 ); //100
			//DrawImage( x+1, y+yoff, 0x98 );
		//}
	}
        
        if ( dfUIVersion == 0 ) 
            DrawImage( x+1, y+yoff, 0x98 );
}


//=============================================================================

__myevic__ void DrawTempLine( int line )
{
	if ( BLINKITEM(2) && PD2 && PD3 )
		return;

        int fset, x, y, yoff;
        fset = 0x1F;
        y = line;
        x = 55;
        yoff = 2;
        
        if ( dfUIVersion == 1 )
        {
            fset = 0x29;
            y += 3;
            x = 63;
            yoff = 8;
        }
        
        DrawString( String_TEMP_s, 0, y+yoff );

	if ( Screen == 2 ) //fire
	{
		if ( dfIsCelsius )
		{
			DrawValueRight( x, y, FarenheitToC( AtoTemp ), 0, fset, 3 );
		}
		else
		{
			DrawValueRight( x, y, AtoTemp, 0, fset, 3 );
		}
	}
	else
	{
		DrawValueRight( x, y, dfTemp, 0, fset, 3 );
	}
               
        if ( dfUIVersion == 0 )
            DrawImage( x+1, y+yoff, dfIsCelsius ? 0xC9 : 0xC8 );
}


//=============================================================================

__myevic__ void DrawVoltsLine( int volts, int line )
{
        int fset, x, y, yoff;
        fset = 0x1F;
        y = line;
        x = 55;
        yoff = 2;
        
        if ( dfUIVersion == 1 )
        {
            fset = 0x29;
            y += 3;
            x = 63;
            yoff = 8;
        }
        
        if ( dfStatus.vvlite && dfMode == 4 )
        {
            DrawImage( 0, y+yoff, 0xF8 );   
        } 
        else
        {
            DrawString( String_VOLT_s, 0, y+yoff );
        }
	// for real bypass if ( volts > MaxVolts ) volts = MaxVolts;
	DrawValueRight( x, y, volts, 2, fset, 3 );
        
        if ( dfUIVersion == 0 )
            DrawImage( x+1, y+yoff, 0x7D );
}


//=============================================================================

__myevic__ void DrawCoilLine( int line )
{
	unsigned int rez;

	if ( BLINKITEM(3) )
		return;

        int fset, x, y, yoff;
        fset = 0x1F;
        y = line;
        x = 55;
        yoff = 2;
        
        if ( dfUIVersion == 1 )
        {
            fset = 0x29;
            y += 10;
            x = 63;
            yoff = 8;
        }
        
                DrawString( String_COIL_s, 0, y+yoff );

            	if ( Set_NewRez_dfRez || !AtoRez )
		{
			rez = AtoRez;
		}
		else
		{
			rez = dfResistance;
		}

        DrawValueRight( x, y, rez, 2, fset, 3 );

	if     ((( dfMode == 0 ) && ( dfRezLockedNI ))
	||	(( dfMode == 1 ) && ( dfRezLockedTI ))
	||	(( dfMode == 2 ) && ( dfRezLockedSS ))
	||	(( dfMode == 3 ) && ( dfRezLockedTCR )))
	{
            if ( dfUIVersion == 1 )
            {
                DrawImage( 0, y+yoff, 0xC3 ); //lock 9
            }
            else
            {    
		DrawImage( x+1, y+yoff, 0xC3 ); //lock
            }
	}
	else
	{
            if ( dfUIVersion == 0 )
		DrawImage( x+1, y+yoff, 0xC0 );
	}

	if ( rez )
	{ //blink if res not for this mode
		if (   ( ISMODETC(dfMode) && ( rez > 150 ) )
			|| ( ISMODEVW(dfMode) && ( rez < 5 ) ) )
		{
			if ( gFlags.osc_1hz )
			{
				DrawFillRect( 23, y-1, 63, y+yoff+8, 2 );
			}
			ScreenRefreshTimer = 5; //blink 5 times
		}
	}
}

//=============================================================================

__myevic__ void DrawAPTLines()
{       
    // APT - line 4, APT3 - line 3 (i)=1
    int count;
    if ( dfUIVersion == 1 )
        count = 1;
    else
        count = 2;
    
    for ( int i = 0 ; i < count ; ++i )
    {       
	if ( ( i == 0 && BLINKITEM(5) ) || ( i == 1 && BLINKITEM(4) ) )
		continue;
        
        uint8_t a = i? dfAPT3 : dfAPT;
        int line = i? 80 : 97; 
         
	switch ( a )
	{
		default:
		case 0:	// Current
		{
                    if ( gFlags.battery_charging )
                    {
                    	DrawString( gFlags.firing ? String_AMP_s : String_UCH_s, 0, line+2 );
                        if ( gFlags.firing ) DrawValue( 27, line, AtoCurrent, 1, 0x1F, 3 );    
                        else DrawValue( 27, line, ChargeCurrent / 10, 2, 0x1F, 3 ); 
                    } else {
			DrawString( String_AMP_s, 0, line+2 );
			DrawValue( 27, line, ( gFlags.firing ) ? AtoCurrent : 0, 1, 0x1F, 3 );                        
                    }
			DrawImage( 57, line+2, 0x9C );
			break;
		}

		case 1:	// Puff counter
		{
			DrawString( String_PUFF_s, 0, line+2 );
			DrawValueRight( 24+8*5, line, dfPuffCount, 0, 0x1F, 0 );
			break;
		}

		case 2:	// Time counter
		{
		//	DrawString( String_TIME_s, 0, line+2 );
		//	DrawValue( 24, line, dfTimeCount / 10, 0, 0x1F, 5 );
			DrawString( String_PUFF_s, 0, line+2 );
			DrawValueRight( 34, line+2, dfTimeCount / 36000, 0, 0x0B, 0 );
			DrawImage( 34, line+2, 0xD7 );
			DrawValue( 37, line+2, dfTimeCount / 600 % 60, 0, 0x0B, 2 );
			DrawImage( 49, line+2, 0xD7 );
			DrawValue( 52, line+2, dfTimeCount / 10 % 60, 0, 0x0B, 2 );
			break;
		}

		case 3:	// Vape Velocity
		{
			uint32_t vv, t;
			// Elasped seconds since last VV reset
			t = RTCGetEpoch( 0 );
			t -= RTCReadRegister( RTCSPARE_VV_BASE );

			vv = dfVVRatio * ( MilliJoules / 1000 ) / 1000;
			vv /= 10;
			if ( vv > 9999 ) vv = 9999;
                        
                        if ( dfStatus2.vapedjoules )
                        {
                            vv = ( MilliJoules / 10 ) / 3600;
                            if ( vv > 9999 ) vv = 9999;
                            DrawImage( 0, line+2, 0xDE ); //energy
                            DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                            DrawImage( 56, line, 0x67 ); //wh
                        }                       
                        else
                        {
                            if ( dfStatus.vapedml )
                            {
				//DrawString( String_LIQ_s, 0, line+2 );
				DrawImage( 0, line+2, 0xF9 ); //ml
				DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                            }
                            else
                            {
				vv = vv * 86400 / ( t ? : 1 );
				DrawImage( 0, line+2, 0xF3 ); //mld
				DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                            }
                            DrawImage( 57, line+2, 0xCD ); //flask
                        }
			break;
		}

		case 4:	// Atomizer voltage
		{
			DrawString( String_VOUT_s, 0, line+2 );
			DrawValue( 27, line, gFlags.firing?AtoVolts:0, 2, 0x1F, 3 );
			DrawImage( 57, line+2, 0x7D );
			break;
		}

		case 5:	// Battery voltage
		{
			DrawString( String_BATT_s, 0, line+2 );
			//DrawValue( 27, line, gFlags.firing?RTBattVolts:BatteryVoltage, 2, 0x1F, 3 );
                        if (dfAPT == dfAPT3)
                        {
                            if ( i == 0 )
                            {
                                int b = NumBatteries > 1? 1 : 0;
                                DrawValue( 27, line, gFlags.firing?RTBVolts[b]:BattVolts[b], 2, 0x1F, 3 ); 
                            }
                            else
                            {
                                DrawValue( 27, line, gFlags.firing?RTBVolts[0]:BattVolts[0], 2, 0x1F, 3 );     
                            }
                        }
                        else
                        {
                            DrawValue( 27, line, gFlags.firing?RTBattVolts:BatteryVoltage, 2, 0x1F, 3 );   
                        }
			DrawImage( 57, line+2, 0x7D );
			break;
		}

		case 6:	// Board temperature
		{

                                      
                        int t;
                        if ( ISSINFJ200 )
                        {
                            DrawString( String_TEMP_s, 0, line+2 );
                            t = dfIsCelsius ? AkkuTemp : CelsiusToF( AkkuTemp );
                            DrawValueRight( 37, line+3, t, 0, 0x01, 0 );
                            //DrawImage( 54, 90, dfIsCelsius ? 0xC9 : 0xC8 );
                        }
                        else
                        {
                            DrawString( String_BOARD_s, 0, line+2 );    
                        }
                        
                        t = dfIsCelsius ? BoardTemp : CelsiusToF( BoardTemp );
			DrawValue( t>99?31:39, line, t, 0, 0x1F, t>99?3:2 );
			DrawImage( 56, line+2, dfIsCelsius ? 0xC9 : 0xC8 );
			break;
		}

		case 7:	// Real-time atomizer resistance
		{
			int rez = AtoError ? 0 : AtoRezMilli;
			//int nd = ( rez < 1000 ) ? 3 : 4;
			DrawString( String_RES_s, 0, line+2 );
			DrawValueRight( 55, line, rez, 3, 0x1F, 4 );
			DrawImage( 56, line+2, 0xC0 );
			// Refresh every second
			ScreenRefreshTimer = 10;
			break;
		}

		case 8:	// Real-time clock
		{
			S_RTC_TIME_DATA_T rtd;
			GetRTC( &rtd );
			//DrawTime( 3, line, &rtd, 0x1F );
                        DrawDigitClock( line, 1 );
			break;
		}
		case 9:	// BatteryIntRez
		{
			DrawImage( 0, line+2, 0xFA );
			DrawValueRight( 55, line, BatteryIntRez, 3, 0x1F, 4 );
			DrawImage( 56, line+2, 0xC0 );
			break;
		}
		case 10: // coil temp
		{
			DrawImage( 0, line+2, 0xDB );
                        if ( !AtoRezMilli ) AtoTemp = 32;
                        int t = dfIsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp;
			DrawValueRight( 55, line, t, 0, 0x1F, 0 ); //t>99?3:2
			DrawImage( 56, line+2, dfIsCelsius ? 0xC9 : 0xC8 );
			break;
		}     
                case 11: // batts total
		{
			DrawString( String_BATT_s, 0, line+2 );
                        DrawValueRight( 55, line+2, gFlags.firing?RTBVTotal:BattVoltsTotal, 2, 0x0B, 4 ); 
                        DrawImage( 57, line+2, 0x7D );
			break;
		} 
	}
    }
}

//=============================================================================
__myevic__ void ShowFireDuration( int line )
{
	int x;
	DrawFillRect( 0, line, 63, line+9, 1 );
        DrawPixel( 0, 0, 0 );
        DrawPixel( 63, 0, 0 );
        DrawPixel( 0, line+9, 0 );
        DrawPixel( 63, line+9, 0 );
	DrawFillRect( 1, line+1, 62, line+8, 0 );
	x = ( FireDuration > dfProtec / 2 ) ? 5 : 38;
	DrawValue( x, line+1, FireDuration, 1, 0xB, 0 );
	DrawImage( x + 15 + 6 * ( FireDuration > 99 ), line+1, 0x94 );
	InvertRect( 1, line+1, 3 + 59 * FireDuration / dfProtec, line+8 );   
/*
	//DrawFillRect( 0, line, 63, line+15, 1 );
	DrawFillRect( 0, line, 63, line+10, 0 );
	x = ( FireDuration > dfProtec / 2 ) ? 5 : 38;
	DrawValue( x, line+1, FireDuration, 1, 0xB, 0 );
	DrawImage( x + 15 + 6 * ( FireDuration > 99 ), line+1, 0x94 );
	InvertRect( 2, line, 2 + 59 * FireDuration / dfProtec, line+9 );        

	DrawFillRect( 0, line, 63, line+15, 1 );
	DrawFillRect( 1, line+1, 62, line+14, 0 );
	x = ( FireDuration > dfProtec / 2 ) ? 5 : 38;
	DrawValue( x, line+4, FireDuration, 1, 0xB, 0 );
	DrawImage( x + 15 + 6 * ( FireDuration > 99 ), line+4, 0x94 );
	InvertRect( 2, line+2, 2 + 59 * FireDuration / dfProtec, line+13 );
*/
}


//=============================================================================
__myevic__ void DrawInfoLines()
{
	//if (( gFlags.debug & 1 ) && ( !gFlags.firing ) && ( !EditModeTimer ))
/*
        if (( gFlags.debug & 1 ) && ( !EditModeTimer ))
	{

		//uint32_t flags;
		//MemCpy( &flags, (uint8_t*)&gFlags + 4, sizeof( uint32_t ) );
		//DrawHexLong( 0, 52, flags, 1 );

		//MemCpy( &flags, (void*)&gFlags, sizeof( uint32_t ) );
		//DrawHexLong( 0, 71, flags, 1 );

                //BypassVolts BattVoltsTotal MaxVolts AtoMaxVolts AtoVolts MilliJoules

               // unsigned int amps;  
               // amps = 1000 * BattVoltsTotal / ( 10 * AtoRez + NumBatteries * BatteryIntRez );
               // //amps = 10 * BattVoltsTotal	/ ( 10 * AtoRez + NumBatteries * BatteryIntRez );
     
                DrawValue( 0, 46, MaxPower, 0, 0x1F, 0 );
		DrawValue( 0, 58, dfMaxPower, 0, 0x1F, 0 );
                DrawValue( 0, 70, BattVoltsTotal, 0, 0x1F, 0 );
                DrawValue( 0, 82, AtoRez, 0, 0x1F, 0 );
                DrawValue( 0, 94, ClampPower( BypassVolts, 0 ), 0, 0x1F, 0 );
                
                DrawValue( 33, 46, AtoMaxVolts, 0, 0x1F, 0 );
                DrawValue( 33, 58, AtoVolts, 0, 0x1F, 0 );
                DrawValue( 33, 70, BatteryIntRez, 0, 0x1F, 0 );
                DrawValue( 33, 82, NumBatteries, 0, 0x1F, 0 );
                
		//DrawValueRight( 64, 90, BatteryMaxPwr / 10, 0, 0x1F, 0 );

		return;
	}
*/

      //          DrawValue( 0, 46, BatteryMaxAmp, 0, 0x1F, 0 );
	//	DrawValue( 0, 58, BatteryMaxPwr, 0, 0x1F, 0 );
        //        DrawValue( 0, 70, gFlags.nbcr ? 1 : 0, 0, 0x1F, 0 );
                //DrawValue( 0, 82, gFlags.rtcinit ? 1 : 0, 0, 0x1F, 0 );
                //DrawValue( 0, 94, dfStatus.nbrc ? 1 : 0, 0, 0x1F, 0 );
  
    //return;

    
	if ( Screen == 2 ) //firing
	{
		switch ( dfMode )
		{
			case 0:
			case 1:
			case 2:
			case 3:
				if ( dfStatus.priopwr )
				{
					DrawTempLine( 46 ); //52 );
				}
				else
				{
					DrawPwrLine( AtoPower( AtoVolts ), 46 ); //52 );
				}
				break;
			case 4:
                                if ( dfStatus.vvlite )
                                    DrawVoltsLine( dfVVLockedVolt, 46 );
                                else
                                    DrawVoltsLine( dfVWVolts, 46 );
                                
				break;
			case 5:
			{
                                DrawVoltsLine( BypassVolts, 46 );
				break;
			}                      
			default:
				break;
		}
                ShowFireDuration( 0 );        
	}
	else
	{
		switch ( dfMode )
		{
			case 0:
			case 1:
			case 2:
			case 3:
				if ( dfStatus.priopwr )
				{
					DrawTempLine( 46 ); //52 );
				}
				else
				{
					DrawPwrLine( dfTCPower, 46 ); //52 );
				}
				break;
			case 4:
                                if ( dfStatus.vvlite )
                                    DrawVoltsLine( dfVVLockedVolt, 46 );
                                else
                                    DrawVoltsLine( dfVWVolts, 46 );
                            
				break;
			case 5:
				DrawVoltsLine( BypassVolts, 46 ); //52 );
				break;
			default:
				break;
		}
	}

	DrawCoilLine( 63 );
        DrawAPTLines();;
}


//=============================================================================
//----- (000068A0) --------------------------------------------------------
__myevic__ void DrawBFLine( int y )
{
	for ( int v = 0 ; v < 13 ; ++v )
	{
		DrawHLine( 5 * v, y, 5 * v + 2, 1 );
		DrawHLine( 5 * v, y + 1, 5 * v + 2, 1 );
	}
}


//=============================================================================

__myevic__ void DrawTemp()
{
	if ( Screen == 2 )
	{
		if ( dfIsCelsius )
		{
			int tempc = FarenheitToC( AtoTemp );

			DrawValue( 0, 13, tempc, 0, 0x48, 3 );
			DrawImage( 48, 20, 0xE0 );
		}
		else
		{
			DrawValue( 0, 13, AtoTemp, 0, 0x48, 3 );
			DrawImage( 48, 20, 0xE1 );
		}
	}
	else
	{
		DrawValue( 0, 13, dfTemp, 0, 0x48, 3 );
		DrawImage( 48, 20, dfIsCelsius ? 0xE0 : 0xE1 );
	}
        
        if ( ISMODETC(dfMode) )
        {
            if ( dfTCAlgo )
            {
               DrawImage( 53, 12, 0x68 ); //A 
            }
        }
}


//=============================================================================

__myevic__ void DrawPower( int pwr )
{
	int xp, xc;
        int yp = 12;
        
    if ( !dfStatus.onewatt )
    {            
	if ( pwr < 100 )
	{
		xp = 45;
		//yp = 12;

		DrawValue( 5, 13, pwr, 1, 0x48, 2 );
		DrawImage( 46, 20, 0xB9 ); //W
	}
	else
	{
		xp = 54;
		//yp = 12;

		if ( pwr < 1000 )
		{
			DrawValue( 0, 13, pwr, 1, 0x48, 3 );
		}
		else
		{
			DrawValue( 5, 13, pwr / 10, 0, 0x48, 3 );
		//	DrawValue( 0, 18, pwr, 1, 0x29, 4 );
		}

		DrawImage( 54, 28, 0x98 ); //w
	}
    }
    else
    {
        if ( pwr < 100 )
	{
		xp = 33;
		//yp = 12;
		DrawValue( 13, 13, pwr / 10, 0, 0x48, 1 );
		DrawImage( 33, 20, 0xB9 ); //W
	}
	else
	{
		//yp = 12;
		if ( pwr < 1000 )
		{
                    	xp = 47;
			DrawValue( 11, 13, pwr / 10, 0, 0x48, 2 );
                        DrawImage( 46, 28, 0xB2 ); //w
		}
		else
		{
                        xp = 54;
			DrawValue( 3, 13, pwr / 10, 0, 0x48, 3 );
                        DrawImage( 54, 28, 0x98 ); //w
		}
		
	}  
    }

	if ( ISMODEVW(dfMode) )
	{
		if ( dfStatus.pcurve )
		{
                    xc = dfStatus.preheat ? 6: 0;
                    
			if ( !PreheatDelay || gFlags.osc_1hz )
			{
				DrawImage( xp + xc, yp, 0x6A ); //C
			}
		}
                
		if ( dfStatus.preheat )
		{
			if ( !PreheatDelay || gFlags.osc_1hz )
			{
                            if ( dfStatus2.smart_ph && NextPreheatTimer && ( NextPreheatTimer < dfPreheatTime ) )
                                DrawImage( xp, yp, 0x7A ); //S
                            else DrawImage( xp, yp, 0x77 ); //P
			}
		}
	}
        else if ( ISMODETC(dfMode) )
        {
            if ( dfTCAlgo )
            {
               DrawImage( xp, yp, 0x68 ); //A 
            }
        }
}


//=============================================================================

__myevic__ void ShowMainView()
{
	unsigned int pwr, amps;
	unsigned int i, j;

	unsigned int v15; // r0@93
	unsigned int v17; // r8@98
	unsigned int v19; // r3@99
	unsigned int v20; // r1@99
        
	unsigned int sm_p; // r2@168
	int sm_dt; // r3@169

	if ( !gFlags.firing )
	{
		//if ( gFlags.splash && SplashTimer )
                if ( SplashTimer )                    
		{
			ShowSplash();
			return;
		}
	}

	DrawMode();

	pwr = dfPower;

	if ( gFlags.firing )
	{
		pwr = AtoPower( AtoVolts );
	}
	else if ( ISMODEBY(dfMode) )
	{

		//amps = 10 * BattVoltsTotal / ( 10 * AtoRez + NumBatteries * BatteryIntRez );
                amps = 1000 * BattVoltsTotal / ( 10 * AtoRez + NumBatteries * BatteryIntRez );
		BypassVolts = AtoRez * amps / 100;
                               
                //if ( BypassVolts > AtoMaxVolts ) BypassVolts = AtoMaxVolts;
		pwr  = ClampPower( BypassVolts, 0 );
	}

	if ( ISMODETC(dfMode) )
	{
		if ( dfStatus.priopwr )
		{
			if ( Screen == 2 )
			{
				pwr = AtoPower( TargetVolts );
			}
			else
			{
				pwr = dfTCPower;
			}

			DrawPower( pwr );
		}
		else
		{
			DrawTemp();
		}
	}

	if (( dfMode == 4 ) || ( dfMode == 5 ))
	{
		DrawPower( pwr );
	}

	if ( dfMode == 6 )
	{
		//DrawString( String_MAX_s, 23, 15 );
		//DrawString( String_MIN_s, 23, 97 );
		//DrawHLine( 0, 18, 21, 1 );
		//DrawHLine( 43, 18, 63, 1 );
		//DrawHLine( 0, 100, 21, 1 );
		//DrawHLine( 43, 100, 63, 1 );
                DrawHLineDots( 0, 14, 63, 1 );
                DrawHLineDots( 0, 95, 63, 1 );
            
		v15 = SearchSMARTRez( dfSavedCfgRez[(int)ConfigIndex] );
		if ( v15 > 3 )
		{
			for ( i = 0;
				  dfSavedCfgPwr[(int)ConfigIndex] / ( MaxPower / 15 ) > i;
				  ++i )
			{
				DrawFillRect( 0, 89 - 5*i, 62, 90 - 5*i, 1 ); //94 95
			}
			if ( !i ) DrawBFLine( 89 ); //94
		}
		else
		{
			v20 = SMARTPowers[ 2 * v15 + 1 ] / (MaxPower / 15);
			v17 = 94 - 5 * v20; //99
			for ( j = 0; ; ++j )
			{
				v19 = dfSavedCfgPwr[(int)ConfigIndex] / (MaxPower / 15);
				if ( v19 <= j )
					break;
				if ( v19 >= v20 && v20 - 1 <= j )
					DrawBFLine( 89 - 5*j ); //94
				else
					DrawFillRect( 0, 89 - 5*j, 62, 90 - 5*j, 1 ); //94 95
			}
			if ( !j ) DrawBFLine(89); //94
			DrawBFLine( v17 );
			DrawHLine( 25, v17, 39, 0 ); //40
			DrawHLine( 25, v17 + 1, 39, 0 ); //40
			DrawString( String_BF_s, 27, v17 - 3 );
		}

		if ( !ShowWeakBatFlag )
		{
                        if ( gFlags.firing )
                            ShowFireDuration( 0 );
                        
                        sm_p = dfSavedCfgPwr[(int)ConfigIndex];
			if ( sm_p >= 1000 )
			{
                                sm_p /= 10;
                                sm_dt = 0;
			}
			else
			{
                                sm_dt = 1;
			}
                        
                        DrawValue( 13, 101, sm_p, sm_dt, 0x1F, 3 ); //smart power
			DrawImage( 44, 103, 0xB2 ); // W
                        
                        ShowBattery();
/*
			if ( !( gFlags.firing ) )
			{
				if ( !PD2 || !PD3 || ( KeyUpTimer && !EditModeTimer ) )
				{
					sm_p = dfSavedCfgPwr[(int)ConfigIndex];
					if ( sm_p >= 1000 )
					{
						sm_p /= 10;
						sm_dt = 0;
					}
					else
					{
						sm_dt = 1;
					}
					DrawValue( 11, 116, sm_p, sm_dt, 0x1F, 3 ); //smart power
					DrawImage( 42, 118, 0xB2 );
				}
				else
				{
					ShowBattery();
				}
			}
			else
			{
                                ShowBattery();
                                ShowFireDuration( 0 ); 
				//DrawValue( 10, 110, FireDuration, 1, 0x29, 2 );
				//DrawImage( 40, 110, 0xB7 );
			}
*/
		}
	}

	if ( dfMode != 6 )
	{
            
            static uint8_t sx = 0; //pacman line
                
            if ( ( gFlags.firing || gFlags.battery_charging ) && dfStatus.nologo )
            {
                if ( sx % 2 ) 
                {
                    DrawHLineDots( sx+9, 41, 63, 1 );
                    DrawImage( sx, 37, 0xCC );
                }
                else 
                {
                    DrawHLineDots( sx+8, 41, 63, 1 );
                    DrawImage( sx, 37, 0xCB );
                }
                sx += 3;
                if ( sx > 57 ) sx = 0;
            } 
            else 
            {
                sx = 0;
                DrawHLineDots( 0, 41, 63, 1 ); //main first h-lines
            }

		DrawHLineDots( 0, 113, 63, 1 ); //second h-line

		ShowBattery();

		if ( Screen == 2 || EditModeTimer )
		{
			DrawInfoLines();
		}
		else
		{
			if ( dfStatus2.anim3d && !HideLogo )
			{
				anim3d( 1 );
			}
			else if ( dfStatus.clock && !HideLogo )
			{                                 
				if ( dfStatus.digclk != dfStatus2.digclk2 ) //D 01  M 10 
				{
					DrawDigitClock( 62, 0 ); //60
				}
				else
				{	//00 11 AD aM
                                        DrawFillRect( 0, 44, 63, 127, 0 );
					DrawClock( 54 );                         
				}       
			}
                        else if ( !dfStatus.nologo && dfStatus.logomid && !HideLogo ) //mid logo
			{
                               int h = GetLogoHeight();

                                if ( h )
                                {
                                    if ( h > 40 ) 
                                    {
                                        DrawLOGO( 0, 50 );
                                    }
                                    else 
                                    {
                                        DrawLOGO( 0, 56 );
                                    } 
                                }
                        }
                        else 
                        {
                                DrawInfoLines();
                        }                              
                }

		if (( Screen == 1 ) && !HideLogo && !dfStatus.nologo && !dfStatus.logomid)
		{
			int h = GetLogoHeight();
                        int y = 0;
                        
                        if ( h )
                        {
                            if ( h > 40 )
                            {
                                if ( !( dfStatus2.anim3d || dfStatus.clock ) )
                                {
                                    if ( dfUIVersion == 0 )
                                    {
                                        DrawFillRect( 0, 45, 63, 60, 0 );   //erase 1-st info line
                                        y += 10;
                                    }
                                    else
                                    {
                                        DrawFillRect( 0, 10, 63, 66, 0 );   //erase space
                                        y += 15;
                                    }
                                }
                                DrawLOGO( 0, y ); //x y
                            }
                            else
                            {
                                DrawLOGO( 0, 0 ); //x y
                            }
                        }
		}
        }
        
//	if ( ShowProfNum )
//	{
//		DrawFillRect( 0, 108, 63, 127, 0 );
//		DrawString( String_Profile, 12, 114 );
//		DrawImage( 47, 114, dfProfile + 0x0C );
//	}
//	else 
        
        if ( ShowWeakBatFlag )
	{
		DrawFillRect( 0, 108, 63, 127, 0 );
		ShowWeakBat();
	}
}


//=========================================================================
// Analog clock
//-------------------------------------------------------------------------
__myevic__ void DrawClock( int line )
{
    // circle
	S_RTC_TIME_DATA_T rtd;
	GetRTC( &rtd );

	int c = line + 32;

	DrawImage( 0, line, 0xFE );
	DrawCircle( 32, c, 2, 1, 1 );

	int32_t h = ( rtd.u32Hour % 12 ) * 30 + ( rtd.u32Minute >> 1 );
	int32_t m = ( rtd.u32Minute ) * 6;
	int32_t s = ( rtd.u32Second ) * 6;

	DrawLine( 32, c, 32 + (( sine( h ) * 15 ) >> 16 ), c - (( cosine( h ) * 15 ) >> 16 ), 1, 3 );
	DrawLine( 32, c, 32 + (( sine( m ) * 21 ) >> 16 ), c - (( cosine( m ) * 21 ) >> 16 ), 1, 2 );
	DrawLine( 32, c, 32 + (( sine( s ) * 23 ) >> 16 ), c - (( cosine( s ) * 23 ) >> 16 ), 1, 1 );
}

__myevic__ void DrawDigitClock( int line, int infoline )
{
	S_RTC_TIME_DATA_T rtd;
	GetRTC( &rtd );
        int y;
        int x = 0;
        
        int h = rtd.u32Hour;
        if ( dfStatus2.digclk2 )
        { 
            h = h >= 13 ? h - 12 : (h < 1 ? h + 12 : h); //24 -> 12
        }
        
        if ( h < 10 ) x = 3;
        
        if ( !dfStatus.timebig || infoline )
	{ // small
	 DrawValueRight( 19 - x, line, h, 0, 0x1F, 0 ); //2
	 DrawImage( 20 - x, line, 0xDD );
	 DrawValue( 24 - x, line, rtd.u32Minute, 0, 0x1F, 2 );
	 DrawImage( 41 - x, line, 0xDD );
	 DrawValue( 45 - x, line, rtd.u32Second, 0, 0x1F, 2 );
                
		//DrawTime( 3, line, &rtd, 0x1F );
		//DrawDate( 4, line+16, &rtd, 0x1F );
                y = 16;
	}
        else
	{
		//DrawValue( 4, line-3, rtd.u32Hour, 0, 0x29, 2 );
		//DrawValue( 32, line-3, rtd.u32Minute, 0, 0x29, 2 );
                DrawValueRight( 26 - x, line-10, h, 0, 0x3D, 0 ); //2
		DrawValue( 36 - x, line-10, rtd.u32Minute, 0, 0x3D, 2 );
            
		if ( !( rtd.u32Second & 1 ) )
		{
                        DrawImage( 31 - x, line-1, 0xD8 ); // :
			//DrawImage( 28, line-5, 0xDF );
			//DrawImage( 28, line-13, 0xDF );
		}
		//DrawDate( 4, line+18, &rtd, 0x1F );
                y = 18;
	}
        
        if ( !infoline )
                DrawDate( 4, line+y, &rtd, 0x1F ); //and DOW
}
