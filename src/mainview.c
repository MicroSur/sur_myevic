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
uint16_t        AwakeTimer;

const uint8_t *ProfileName[] = { String_Profile1, String_Profile2, String_Profile3, String_Profile4, String_Profile5 };
    
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
		//Screen = 1;
		//ScreenDuration = ScrMainTimes[dfScrMainTime]; //GetMainScreenDuration();
                SetScreen( 1, ScrMainTimes[dfScrMainTime] );
                gFlags.screen_on = 1;
                
	}
	else // dfStatus.off
	{
		if ( gFlags.battery_charging ) //&& dfStealthOn != 1 )
		{
			ChargeView();
		}
		else
		{
			//Screen = 0;
			//SleepTimer = 0;
                        Sleep0Screen();
		}
	}
	//gFlags.refresh_display = 1;
}

//=============================================================================

__myevic__ void DrawVapeAwakeLine ()
{
    // puffs & time to auto mod off
    int at = dfAwakeTimer? dfAwakeTimer - AwakeTimer : 0 ;
    int pc = dfPuffsOff ? dfPuffsOff - PuffsOffCount : 0;
    
        DrawValueRight( 13, 0, pc, 0, 0x0B, 2 );
        
        DrawImage( 16, 0, 0xBA ); //Off
        
	//DrawValueRight( 20, 1, AwakeTimer / 3600, 0, 0x0B, 0 );
	//DrawImage( 20, 1, 0xD7 );
	DrawValue( 37, 0, at / 60 % 60, 0, 0x0B, 2 );
	DrawImage( 49, 0, 0xD7 );
	DrawValue( 52, 0, at % 60, 0, 0x0B, 2 );
        
}


//=============================================================================
__myevic__ void DrawMode()
{
    //first line
       
    if ( dfAwakeTimer || dfPuffsOff )
    {
        if ( !EditModeTimer && LinePuffAwakeTimer ) //EditModeTimer && ( EditItemIndex 
        {
            DrawVapeAwakeLine();
            return;
        }
    }
    LinePuffAwakeTimer = 0;
                                  
    if ( EditModeTimer || ( ProfileName[dfProfile][0] == 0xBC ) )
    {                      
	if ( !BLINKITEM(0) )
	{
		switch ( dfMode )
		{
			case 0:
                        case 1:
                        case 2:
				DrawString( String_TEMP, 10, 0 );
				break;
			case 3:
				DrawString( String_TCR, 10, 0 );
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
				break;
			case 3:
				DrawValue( 35, 0, dfTCRM[dfTCRIndex], 0, 0x0B, 3 ); // 26 TCR value
				//DrawImage( 49, 0, 0xA8 ); //M
				//DrawValue( 57, 0, dfTCRIndex + 1, 0, 0x0B, 1 ); // 1-2-3
				break;
			default:
				break;
		}
	}
    }
    else
    {
        DrawStringCentered( ProfileName[dfProfile], 0 ); //show custom profile name
    }
}

//=============================================================================

__myevic__ void DrawPwrLine( int pwr, int line )
{
    //small info line
    
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
            y += 2; //3;
            x = 63;
            yoff = 8;
        }
        
        DrawString( String_PWR_s, 0, y+yoff );

	if ( pwr < 1000 ) // < 100 w
	{
		DrawValueRight( x, y, pwr, 1, fset, 0 );
                //DrawValueRight( x, y, dfStatus.onewatt? pwr/10 : pwr, dfStatus.onewatt? 0 : 1, fset, 0 );
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
            y += 2; //3;
            x = 63;
            yoff = 8;
        }
        
        DrawString( String_TEMP_s, 0, y+yoff );

	if ( Screen == 2 ) //fire
	{
                DrawValueRight( x, y, dfStatus.IsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp, 0, fset, 3 );  
/*
		if ( dfStatus.IsCelsius )
		{
			DrawValueRight( x, y, FarenheitToC( AtoTemp ), 0, fset, 3 );
		}
		else
		{
			DrawValueRight( x, y, AtoTemp, 0, fset, 3 );
		}
*/
	}
	else
	{
		DrawValueRight( x, y, dfTemp, 0, fset, 3 );
	}
               
        if ( dfUIVersion == 0 )
                DrawImage( x+1, y+yoff, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
}


//=============================================================================

__myevic__ void DrawVoltsLine( int volts, int line )
{
    	if ( BLINKITEM(2) )
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
        
        if ( dfStatus.vvlite && dfMode == 4 )
        {
            DrawImage( 0, y+yoff, 0xF8 ); //vv
        } 
        else
        {
            DrawString( String_VOLT_s, 0, y+yoff );
        }
	// for real bypass if ( volts > MaxVolts ) volts = MaxVolts;
	DrawValueRight( x, y, volts, 2, fset, 3 );
        
        if ( dfUIVersion == 0 )
            DrawImage( x+2, y+yoff, 0x7D );
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
            y += 9; //10;
            x = 63;
            yoff = 8;
        }
                  
        int lock = GetLockState();
 
        DrawString( String_COIL_s, 0, y+yoff );

        if ( ( !lock && Set_NewRez_dfRez ) || !AtoRez ) //Set_NewRez_dfRez show AtoRez on start
	{
		rez = AtoRez;
	}
	else
	{
		rez = dfResistance;
	}

        DrawValueRight( x, y, rez, 2, fset, 3 );
                               
//	if     ((( dfMode == 0 ) && ( dfRezLockedNI ))
//	||	(( dfMode == 1 ) && ( dfRezLockedTI ))
//	||	(( dfMode == 2 ) && ( dfRezLockedSS ))
//	||	(( dfMode == 3 ) && ( dfRezLockedTCR )))
//	{
        if ( lock )
            {
            if ( dfUIVersion == 1 )
            {
                DrawImage( 6, y+yoff, 0xC3 ); //lock glyph on left side over
            }
            else
            {    
		DrawImage( x+2, y+yoff, 0xC3 ); //lock
            }
	}
	else
	{
            if ( dfUIVersion == 0 )
		DrawImage( x+2, y+yoff, 0xC0 );
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

__myevic__ void DrawVapeHoldLine ()
{
    // hold vape timer
	//DrawString( String_TIME_s, 0, line );
        DrawFillRect( 0, 0, 63, 9, 0 );
	DrawValueRight( 20, 1, VapeDelayTimer / 3600, 0, 0x0B, 0 );
	DrawImage( 20, 1, 0xD7 );
	DrawValue( 23, 1, VapeDelayTimer / 60 % 60, 0, 0x0B, 2 );
	DrawImage( 35, 1, 0xD7 );
	DrawValue( 38, 1, VapeDelayTimer % 60, 0, 0x0B, 2 );
        
        //short img = 0xBD; // _
        
        switch ( SwitchOffCase )
        {
            case 1:
                //img = 0xAB;
                DrawImage( 1, 1, 0xAB ); //P
                break;
                
            case 2:
                //img = 0xAF;
                DrawImage( 1, 1, 0xAF ); //T
                break;
                
            //case 3:
            //    img = 0xA1; //F
            //    break;                    
        }
        
        //DrawImage( 1, 1, img );
                
        InvertRect( 0, 0, 63, 9 );
}

//=============================================================================

__myevic__ void DrawTimeCounterLine ( int line )
{
	DrawString( String_TIME_s, 0, line );
	//	DrawValue( 24, line, dfTimeCount / 10, 0, 0x1F, 5 );
	//	DrawString( String_PUFF_s, 0, line );
	DrawValueRight( 34, line, dfTimeCount / 36000, 0, 0x0B, 0 );
	DrawImage( 34, line, 0xD7 );
	DrawValue( 37, line, dfTimeCount / 600 % 60, 0, 0x0B, 2 );
	DrawImage( 49, line, 0xD7 );
	DrawValue( 52, line, dfTimeCount / 10 % 60, 0, 0x0B, 2 );
}

//=============================================================================

__myevic__ void DrawEnergyLine ( int line )
{
        uint32_t vv;
        int dp;
        
        vv = ( MilliJoulesEnergy / 3600 ) / 100;
        
        dp = 2; //99.99
        if ( vv > 99999 ) vv = 99999;
        if ( vv > 9999 ) 
        {
            vv /= 10;
            dp = 1; //999.9
        }
        
        DrawImage( 1, line+2, 0xDE ); //energy
        DrawValueRight( 55, line, vv, dp, 0x1F, 0 );
        DrawImageRight( 64, line, 0x67 ); //wh
}

__myevic__ uint32_t GetVV( uint32_t MJoules, int *dp )
{
    uint32_t vv;
    vv = dfVVRatio * ( MJoules / 1000 );
    vv /= 10;
    vv /= 100;
    vv /= 100;
    
    *dp = 2; //99.99
    if ( vv > 99999 ) vv = 99999;
    if ( vv > 9999 ) 
    {
        vv /= 10;
        *dp = 1; //999.9
    }
        
    return vv;
}

//=============================================================================

__myevic__ void DrawVapedLine ( int line )
{
        uint32_t vv;
        int dp;
        
        if ( !EditModeTimer && VapedLineTimer && dfStatus2.session )
        {
            vv = GetVV( MilliJoules - MilliJoulesVapedOn, &dp );
        }
        else
        {
            vv = GetVV( MilliJoules, &dp );
        }

        DrawImage( 0, line, 0xF9 ); //ml
        DrawValueRight( 55, line-2, vv, dp, 0x1F, 0 );
        DrawImageRight( 64, line, 0xCD ); //flask
}

//=============================================================================

__myevic__ void DrawVapedDayLine ( int line )
{
        uint32_t vv;
        int dp;
        
        if ( !EditModeTimer && VapedLineTimer && dfStatus2.session )
        {
            vv = GetVV( MilliJoules - MilliJoulesVapedOn, &dp );
        }
        else
        {
            vv = GetVV( MilliJoulesDay, &dp );
        }
        

        DrawImage( 0, line, 0xF3 ); //mld
        DrawValueRight( 55, line-2, vv, dp, 0x1F, 0 );
        DrawImageRight( 64, line, 0xCD ); //flask
}

//=============================================================================

__myevic__ void DrawPuffCounterLine ( int line )
{
        uint32_t v;
        
        if ( !EditModeTimer && VapedLineTimer && dfStatus2.session )
        {
            v = SessionPuffs;
        }
        else
        {
            v = dfPuffCount;
        }
            
	DrawString( String_PUFF_s, 0, line );
	DrawValueRight( 64, line-2, v, 0, 0x1F, 0 );
}

__myevic__ void DrawStopwatch( int line )
{
    S_RTC_TIME_DATA_T rtd;
    time_t t;

    RTCGetEpoch( &t );
    t -= startwatch;
    RTCEpochToTime( &rtd, &t );

    if ( rtd.u32Hour == 24 ) RTCGetEpoch( &startwatch ); //reset 24h
    
    DrawValueRight( 19, line, rtd.u32Hour, 0, 0x1F, 2 );
    DrawImage( 20, line+2, 0xDD );
    DrawValue( 24, line, rtd.u32Minute, 0, 0x1F, 2 );
    DrawImage( 41, line+2, 0xDD );
    DrawValue( 45, line, rtd.u32Second, 0, 0x1F, 2 );
    
    InvertRect( 0, line-1, 63, line+10 );
    
/*
    //instead of main power/temp line in idle
    DrawValue( 0, 13, rtd.u32Hour, 0, 0x3D, 2 );
    DrawImage( 23, 20, 0xD8 );
    DrawValue( 25, 13, rtd.u32Minute, 0, 0x3D, 2 );
    DrawValue( 49, 13, rtd.u32Second, 0, 0x1F, 2 );
*/
}

//=============================================================================

__myevic__ void DrawAPTLines()
{       
    // APT - line 4, APT3 - line 3 (i)=1
    int count;
    if ( dfUIVersion == 1 && dfMode != 6 )
        count = 1;
    else
        count = 2;
    
    for ( int i = 0 ; i < count ; ++i )
    {       
	if ( ( i == 0 && BLINKITEM(5) ) || ( i == 1 && BLINKITEM(4) ) )
		continue;
        
        uint8_t a = i? dfAPT3 : dfAPT;
        int line = i? 80 : 97; 
        int ximg = 57;
        // Refresh every second
        ScreenRefreshTimer = 10; 
        
	switch ( a )
	{
		default:
		case 0:	// Current
		{
                    if ( gFlags.battery_charging )
                    {
                    	DrawString( gFlags.firing ? String_AMP_s : String_UCH_s, 0, line+2 );
                        if ( gFlags.firing ) 
                        {
                            //DrawValue( 27, line, AtoCurrent, 1, 0x1F, 3 );
                            DrawValueRight( 55, line, AtoCurrent, 1, 0x1F, 0 );
                        }
                        else 
                        {
                            if ( !gFlags.soft_charge )
                            {
                                DrawImageRight( 63, line+2 , 0xF6 ); // N/A
                                break;
                            } 
                            else
                            {
                                //DrawValue( 27, line, ChargeCurrent / 10, 2, 0x1F, 3 ); 
                                DrawValueRight( 55, line, ChargeCurrent / 10, 2, 0x1F, 3 );
                            }
                        }
                    } 
                    else 
                    {
			DrawString( String_AMP_s, 0, line+2 );
			//DrawValue( 27, line, ( gFlags.firing ) ? AtoCurrent : 0, 1, 0x1F, 3 );  
                        DrawValueRight( 55, line, ( gFlags.firing ) ? AtoCurrent : AtoCurrentMax, 1, 0x1F, 0 );
                        
                    }
                    
			DrawImage( ximg, line+2, 0x68 ); //A
			break;
		}

		case 1:	// Puff counter ( preserved case 1 )
		{
                        DrawPuffCounterLine( line+2 );
			break;
		}

		case 2:	// Time counter ( preserved case 2 )
		{
                        DrawTimeCounterLine( line+2 );
			break;
		}

		case 3:	// Vape Velocity ( preserved case 3 4 5)
                case 4:
                case 5:
		{
			//uint32_t vv;
			//vv = dfVVRatio * ( MilliJoules / 1000 ) / 1000;
			//vv /= 10;
			//if ( vv > 9999 ) vv = 9999;
                        //DrawValue( 0, 0, MilliJoules, 0, 0x01, 0 );
                        
                        if ( a == 5 )                        
                        {
                                //vv = ( MilliJoules / 3600 ) / 10;
                                //if ( vv > 9999 ) vv = 9999;
                                //DrawImage( 0, line+2, 0xDE ); //energy
                                //DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                                //DrawImage( 56, line, 0x67 ); //wh
                                DrawEnergyLine( line );
                        }                       
                        else if ( a == 3 )
                        {
                                //vv = GetVV(MilliJoules);
				//DrawImage( 0, line+2, 0xF9 ); //ml
				//DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                                //DrawImage( 57, line+2, 0xCD ); //flask
                                DrawVapedLine( line+2 );
                        }
                        else 
                        {
                            	// Elasped seconds since last VV reset
                                //uint32_t t;
                                //t = RTCGetEpoch( 0 );
                                //time_t t;
                                //RTCGetEpoch( &t );
                                //DrawValue( 0, 37, t, 0, 0x01, 0 );
                                //t -= RTCReadRegister( RTCSPARE_VV_BASE );
                                //DrawValue( 0, 108, t, 0, 0x01, 0 );
                                //DrawValue( 0, 57, RTCReadRegister( RTCSPARE_VV_BASE ), 0, 0x01, 0 );
				//vv = vv * 86400 / ( t ? : 1 );
                            
                                //vv = GetVV(MilliJoulesDay);
				//DrawImage( 0, line+2, 0xF3 ); //mld
				//DrawValueRight( 55, line, vv, 2, 0x1F, 0 );
                                //DrawImage( 57, line+2, 0xCD ); //flask
                                DrawVapedDayLine( line+2 );
                        }                                                   
			break;
		}
                
		case 6:	// Atomizer voltage
		{
			DrawString( String_VOUT_s, 0, line+2 );
			DrawValue( 27, line, gFlags.firing ? AtoVolts : AtoVoltsMax, 2, 0x1F, 3 );
			DrawImage( ximg, line+2, 0x7D );
			break;
		}
                
		case 7:	// Battery voltage
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
			DrawImage( ximg, line+2, 0x7D );
			break;
		}
                
		case 8:	// Real-time clock ( preserved case 8 )
		{
			//S_RTC_TIME_DATA_T rtd;
			//GetRTC( &rtd );
			//DrawTime( 3, line, &rtd, 0x1F );
                        DrawDigitClock( line, 1 );
			break;
		}
                
		case 9:	// Board temperature
		{             
                        int t;
                        
                        DrawString( String_TEMP_s, 0, line+2 );
                        
                        if ( ISSINFJ200 || ISIKU200 )
                        {
                            //DrawString( String_TEMP_s, 0, line+2 );
                            t = dfStatus.IsCelsius ? AkkuTemp : CelsiusToF( AkkuTemp );
                            DrawValueRight( 37, line+2, t, 0, 0x0B, 0 );
                            //DrawImage( 54, 90, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
                        }
                        //else
                        //{
                        //    DrawString( String_BOARD_s, 0, line+2 );    
                        //}
                        
                        t = dfStatus.IsCelsius ? BoardTemp : CelsiusToF( BoardTemp );
			DrawValue( t>99?31:39, line, t, 0, 0x1F, t>99?3:2 );
			DrawImage( ximg, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
                        //ScreenRefreshTimer = 10;
			break;
		}
                
		case 10:	// Real-time atomizer resistance
		{
			int rez = AtoError ? 0 : AtoRezMilli;
			//int nd = ( rez < 1000 ) ? 3 : 4;
			DrawString( String_RES_s, 0, line+2 );
			DrawValueRight( 55, line, rez, 3, 0x1F, 4 );
			DrawImage( ximg, line+2, 0xC0 );
			// Refresh every second
			//ScreenRefreshTimer = 10;
			break;
		}
                
		case 11:	// BatteryIntRez
		{
			DrawImage( 0, line+2, 0xFA );
			DrawValueRight( 55, line, BatteryIntRez, 3, 0x1F, 4 );
			DrawImage( ximg, line+2, 0xC0 );
			break;
		}
                
		case 12: // coil temp
		{
                    // AtoTemp from last visited TC mode!
			DrawImage( 0, line+2, 0xDB );
                        if ( !AtoRezMilli ) AtoTemp = 32;
                        int t = dfStatus.IsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp;
			DrawValueRight( 55, line, t, 0, 0x1F, 0 ); //t>99?3:2
			DrawImage( ximg, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
			break;
		}
                
                case 13: // batts total
		{
			DrawString( String_BATT_s, 0, line+2 );
                        DrawValueRight( 55, line+2, gFlags.firing?RTBVTotal:BattVoltsTotal, 2, 0x0B, 4 ); 
                        DrawImage( ximg, line+2, 0x7D );
			break;
		}
                
                case 14: // min res on puff
		{
			DrawImage( 0, line+2, 0xED );
                        DrawValueRight( 55, line, AtoRezMilliMin, 3, 0x1F, 4 ); 
                        DrawImage( ximg, line+2, 0xC0 );
			break;
		}          
                case 15: // stopwatch
		{
                        DrawStopwatch( line );
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
/*
        DrawPixel( 0, 0, 0 );
        DrawPixel( 63, 0, 0 );
        DrawPixel( 0, line+9, 0 );
        DrawPixel( 63, line+9, 0 );
*/
	DrawFillRect( 1, line+1, 62, line+8, 0 );
	x = ( FireDuration > dfProtec / 2 ) ? 5 : 38;
	DrawValue( x, line+1, FireDuration, 1, 0xB, 0 );
	DrawImage( x + 15 + 6 * ( FireDuration > 99 ), line+1, 0x94 );
	InvertRect( 1, line+1, 3 + 59 * FireDuration / dfProtec, line+8 );   

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

    
	//if ( Screen == 2 ) //firing
        if ( gFlags.firing )
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
                                    DrawVoltsLine( VWVolts, 46 );
                                
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
                                    DrawVoltsLine( VWVolts, 46 );
                            
				break;
			case 5:
				DrawVoltsLine( BypassVolts, 46 ); //52 );
				break;
			default:
				break;
		}
                
                if ( VapeDelayTimer && !EditModeTimer )
                {
                        DrawVapeHoldLine();
                }
	}

        if ( dfMode == 6 )
        {
                int x1, x2;
                int pMax = 0;
                int p;
                
                for ( int i = 0 ; i < 10 ; ++i )
                {
                    p = dfSavedCfgPwr[i];
                    if ( p > pMax ) pMax = p;
                }
                
                if ( !pMax ) pMax = MaxPower;
                
            	for ( int i = 0 ; i < 10 ; ++i )
                {
                    p = dfSavedCfgPwr[i];
                    
                    if ( i == ConfigIndex && AtoStatus == 4 )
                    {
                        x1 = 3;
                        x2 = 3;
                    }
                    else
                    {
                        x1 = 4;
                        x2 = 1;
                    }
                
                    //DrawFillRect( const int x1, const int y1,const  int x2, const int y2, const int color)
                    DrawFillRect( x1+i*6, 72 - 26 * p / pMax, x1+x2+i*6, 72, 1); //74 = bottom, 30 = height
                    //DrawVLine( i+4, 36 - 20 * p / MaxPower, 36, 1 );
		}
                
        }
        else
        {
        //if ( dfMode != 6 )
            DrawCoilLine( 63 );
        }
                
        DrawAPTLines();
        
}


//=============================================================================
//----- (000068A0) --------------------------------------------------------
/*
__myevic__ void DrawBFLine( int y )
{
	for ( int v = 0 ; v < 13 ; ++v )
	{
		DrawHLine( 5 * v, y, 5 * v + 2, 1 );
		DrawHLine( 5 * v, y + 1, 5 * v + 2, 1 );
	}
}
*/


//=============================================================================

__myevic__ void DrawTemp()
{
	if ( Screen == 2 )
	{
		if ( dfStatus.IsCelsius )
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
		DrawImage( 48, 20, dfStatus.IsCelsius ? 0xE0 : 0xE1 );
	}
        
        if ( ISMODETC(dfMode) )
        {
            if ( dfTCAlgo )
            {
               DrawImage( 51, 10, 0x82 ); //A 68, a 82 (y-2)
            }
        }
}


//=============================================================================

__myevic__ void DrawPower( int pwr, int yp )
{
    //WW line yp = 12
    //Smart line = 60
	int xp, xc; // x coord for preheat
        //int yp = 12;
    
    if ( !dfStatus.onewatt )
    {            
	if ( pwr < 100 )
	{
		xp = 45;

		DrawValue( 5, yp+1, pwr, 1, 0x48, 2 ); //13
		DrawImage( 45, yp+8, 0xB9 ); //W
	}
	else
	{
		xp = 54;

		if ( pwr < 1000 )
		{
			DrawValue( 1, yp+1, pwr, 1, 0x48, 3 ); //0->1 for pc, need big dot 4x24
		}
		else
		{
			DrawValue( 5, yp+1, pwr / 10, 0, 0x48, 3 );
		}

		DrawImage( 54, yp+16, 0x98 ); //w 28
	}
    }
    else //one watt
    {
        uint16_t p = pwr + 5; //round up
        
        if ( p < 100 )
	{
		xp = 33;
		DrawValue( 13, yp+1, p / 10, 0, 0x48, 1 ); //13
		DrawImage( 33, yp+8, 0xB9 ); //W
	}
	else
	{

		if ( p < 1000 )
		{
                    	xp = 47;
			DrawValue( 11, yp+1, p / 10, 0, 0x48, 2 ); //13
                        DrawImage( 47, yp+16, 0xB2 ); //w
		}
		else
		{
                        xp = 54;
			DrawValue( 3, yp+1, p / 10, 0, 0x48, 3 ); //13
                        DrawImage( 54, yp+16, 0x98 ); //w 28
		}
		
	}  
    }

	if ( ISMODEVW(dfMode) )
	{
                if ( dfStatus.keylock && dfStatus2.replay )
                {
                        DrawImage( xp, yp-2, 0x93 ); //93 r (y-2), AD R (y+1)
                        return;
                }
                            
		if ( dfStatus.pcurve )
		{
                    xc = dfStatus.preheat ? 6: 0;
                    
			if ( !CurveDelay || gFlags.osc_1hz )
			{
				DrawImage( xp + xc, yp-2, 0x84 ); //C 6A , c 84  (y-2)
			}
		}
                
		if ( dfStatus.preheat )
		{
                    if ( !PreheatDelay || gFlags.osc_1hz )
                    {
                        int v = GetSmartPreheat();
                        if ( v && NextPreheatTimer && ( NextPreheatTimer < dfPreheatTime ) )
                        {
                            DrawImage( xp, yp-2, 0x94 ); //S 7A , s 94 (y-2)
                        }
                        else
                        {
                            DrawImage( xp, yp-2, 0x91 ); //P 77, p 91
                        }
                    }

		}                
	}
        else if ( ISMODETC(dfMode) )
        {
            if ( dfTCAlgo )
            {
               DrawImage( xp, yp-2, 0x82 ); //A 68, a 82 (y-2)
            }
        }
}

//=============================================================================

__myevic__ void DrawClockMid()
{
    if ( dfStatus.digclk != dfStatus2.digclk2 ) //D 01  M 10 
    {
        //DrawFillRect( 0, 42, 63, 112, 0 );
	DrawDigitClock( 62, 0 ); //60
    }
    else
    {	//00 11 AD aM
        //DrawFillRect( 0, 44, 63, 127, 0 );
        DrawClock( 48 ); //53
    } 
}

__myevic__ void ShowLogo( int place )
{
    int h = GetLogoHeight();
    
    if ( !place ) // 0, middle scr
    {
        if ( !HideLogo && !gFlags.toggleclock )
        {
                        if ( dfStatus2.anim3d ) return;
    			//if ( dfStatus2.anim3d ) //&& !HideLogo )
			//{           
				//anim3d( 0 );
			//}
			if ( dfStatus.clock ) //&& !HideLogo )
			{                   
                                DrawClockMid();
/*
				if ( dfStatus.digclk != dfStatus2.digclk2 ) //D 01  M 10 
				{
                                        //DrawFillRect( 0, 42, 63, 112, 0 );
					DrawDigitClock( 62, 0 ); //60
				}
				else
				{	//00 11 AD aM
                                        DrawFillRect( 0, 44, 63, 127, 0 );
					DrawClock( 53 );
				}       
*/
			}
                        else if ( !dfStatus.nologo && dfStatus.logomid ) //&& !HideLogo ) //mid logo
			{
                                    //DrawFillRect( 0, 42, 63, 112, 0 );
                                    DrawLOGO( 0, 77 - h / 2 );      
                        }
                        else 
                        {
                                DrawInfoLines(); //no firing, no logos
                        } 
        }
        else
        {
            if ( !gFlags.toggleclock )
            {
                DrawInfoLines(); //no firing, HideLogo timer != 0
            }
            else
            {
                //HideLogo = dfHideLogo;
                //DrawFillRect( 0, 42, 63, 112, 0 );
                //DrawDigitClock( 62, 0 );
                DrawClockMid();
            }
        }
    }
    else //1 pic logo top
    {
        //if ( !HideLogo )
        //{
                        int y = 0;
                        int y2;
                        
                        if ( h )
                        {
                            if ( h > 40 )
                            {
                                if ( !( dfStatus2.anim3d || dfStatus.clock || gFlags.toggleclock ) )
                                {
                                    if ( dfMode == 6 )
                                    {
                                        y2 = 72;
                                        y = 10;
                                    }
                                    else if ( dfUIVersion == 0 )
                                    {
                                        y2 = 60;
                                        //y = 7;
                                        y = (62 - h) / 2;
                                        
                                    }
                                    else
                                    {
                                        y2 = 66;
                                        y = 12; //14;
                                    }
                                    
                                    DrawFillRect( 0, 0, 63, y2, 0 );
                                }
                                
                                DrawLOGO( 0, y ); //x y
                            }
                            else // h <= 40
                            {
                                DrawHLineDots( 0, 41, 63, 0 );
                                DrawLOGO( 0, 0 ); //x y
                            }
                        }    
        //}
    }
}

//=============================================================================

__myevic__ void ShowMainView()
{
	unsigned int pwr, amps;
	//unsigned int i, j;

	//unsigned int v15; // r0@93
	//unsigned int v17; // r8@98
	//unsigned int v19; // r3@99
	//unsigned int v20; // r1@99
        
	//unsigned int sm_p; // r2@168
	//int sm_dt; // r3@169
        int btv;
        int numb;

	if ( !gFlags.firing )
	{
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
		pwr = AtoPower( AtoVolts ); //from ADC
	}
	else if ( ISMODEBY(dfMode) )
	{
                if ( !dfStatus2.bybatts ) //0 - 1batt
                {
                    btv = BattVoltsTotal / NumBatteries;
                    numb = 1;
                }
                else
                {
                    btv = BattVoltsTotal;
                    numb = NumBatteries;
                }
                amps = 1000 * btv / ( 10 * AtoRez + numb * BatteryIntRez );
                BypassVolts = AtoRez * amps / 100;
                
                //BypassVolts = BattVoltsTotal / NumBatteries;
                //BypassVolts = BatteryVoltage;
                               
                //if ( BypassVolts > AtoMaxVolts ) BypassVolts = AtoMaxVolts;
                
                //calculated power:
		pwr  = ClampPower( BypassVolts, 1); //0 );
	}

    //if ( Screen != 2 )
    //{
    //    DrawStopwatch();
    //}
    //else
    //{    
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

			DrawPower( pwr, 12 );
		}
		else
		{
			DrawTemp();
		}
                
	}
	//else if (( dfMode == 4 ) || ( dfMode == 5 )) //pwr bypass
        else 
	{
                if ( dfMode == 6 && !gFlags.firing ) //smart
                {
                        pwr = dfSavedCfgPwr[(int)ConfigIndex]; //show fixed power
                }
                
		DrawPower( pwr, 12 );
                
	}
    //}
        
///////////////////////////////////////////////////////////
        
            static int sx = 0; //pacman line
            DrawHLineDots( 0, 41, 63, 1 ); //main first h-lines
            //if ( ( gFlags.firing || gFlags.battery_charging ) && HideLogo ) //dfStatus.nologo )
            if ( ( gFlags.firing || gFlags.battery_charging ) 
                    && ( HideLogo || ( dfStatus.nologo && !dfStatus2.anim3d && !dfStatus.clock ) ) 
                    && !gFlags.toggleclock )
            {
                //DrawHLineDots( 0, 41, 63, 1 ); //main first h-lines
                DrawHLine( sx-5, 41, sx, 0 );
                
                if ( sx % 2 ) 
                {
                    //DrawHLineDots( sx+9, 41, 63, 1 );
                    DrawImage( sx, 37, 0xCC );
                }
                else 
                {
                    //DrawHLineDots( sx+8, 41, 63, 1 );
                    DrawImage( sx, 37, 0xCB );
                }
                sx += 3;
                if ( sx > 57 ) sx = 0;
            } 
            else 
            {
                sx = 0;
                //DrawHLineDots( 0, 41, 63, 1 ); //main first h-lines
            }
            
//////////////
            
            if ( Screen == 2 || Screen == 23 || EditModeTimer ) //fire long_fire_protec
            {
		DrawInfoLines(); //on firing
            }
            else
            {
                ShowLogo( 0 );  //center   
            }

            if (( Screen == 1 ) && !EditModeTimer && !HideLogo && !dfStatus.nologo && !dfStatus.logomid)
            {
                ShowLogo( 1 ); //top pic logo
            }
        
       
        if ( ShowWeakBatFlag )
	{
		//DrawFillRect( 0, 108, 63, 127, 0 );
		ShowWeakBat();
	}

        //if ( gFlags.firing || 
        //    !( !HideLogo && !dfStatus2.anim3d && dfStatus.clock && dfStatus.digclk == dfStatus2.digclk2 ) )
        //{ //when not analog clock logo
                
            DrawHLineDots( 0, 113, 63, 1 ); //second h-line
            ShowBattery();    
        //}

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
        
	//DrawLine( 32, c, 32 + (( sine( s ) * 23 ) >> 16 ), c - (( cosine( s ) * 23 ) >> 16 ), 1, 1 );

        DrawLine( 32 - (( sine( s ) * 6 ) >> 16 ), 
                c + (( cosine( s ) * 6 ) >> 16 ), 
                32 + (( sine( s ) * 23 ) >> 16 ), 
                c - (( cosine( s ) * 23 ) >> 16 ), 
                1, 1 );
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
	 DrawImage( 20 - x, line+2, 0xDD );
	 DrawValue( 24 - x, line, rtd.u32Minute, 0, 0x1F, 2 );
	 DrawImage( 41 - x, line+2, 0xDD );
	 DrawValue( 45 - x, line, rtd.u32Second, 0, 0x1F, 2 );
         
		//DrawTime( 3, line, &rtd, 0x1F );
		//DrawDate( 4, line+16, &rtd, 0x1F );
                y = 16;
	}
        else
	{ //big digits
		//DrawValue( 4, line-3, rtd.u32Hour, 0, 0x29, 2 );
		//DrawValue( 32, line-3, rtd.u32Minute, 0, 0x29, 2 );
                DrawValueRight( 26 - x, line-10, h, 0, 0x3D, 0 ); //2
		DrawValue( 36 - x, line-10, rtd.u32Minute, 0, 0x3D, 2 );
            
		if ( !( rtd.u32Second & 1 ) )
		{
                        DrawImage( 31 - x, line-2, 0xD8 ); // :
			//DrawImage( 28, line-5, 0xDF );
			//DrawImage( 28, line-13, 0xDF );
		}
		//DrawDate( 4, line+18, &rtd, 0x1F );
                y = 18;
	}
        
        if ( !infoline )
                DrawDate( 4, line+y, &rtd, 0x1F ); //and DOW
}

//=========================================================================
__myevic__ void ShowSetJoules()
{
        DrawString( String_Vaped, 4, 5 );
        DrawHLine( 0, 16, 63, 1 );
        
	DrawString( String_mlkJ, 0, 26 );
        DrawValueRight( 63, 24, dfVVRatio, 0, 0x1F, 0 );
        InvertRect( 32, 23, 62, 23+12 ); // ? DrawStringRightInv

        DrawVapedLine( 43 );
        DrawVapedDayLine( 60 );
        DrawEnergyLine( 75 );
        DrawTimeCounterLine( 92 );
        DrawPuffCounterLine( 109 );    
}
