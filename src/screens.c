#include "myevic.h"
#include "screens.h"
#include "display.h"
#include "menus.h"
#include "myrtc.h"
#include "dataflash.h"
#include "atomizer.h"
#include "timers.h"
#include "battery.h"
#include "miscs.h"
#include "events.h"


//=========================================================================

uint8_t		Screen;
uint16_t	ScreenDuration;
uint16_t	ScreenRefreshTimer;

//ScrSaveTimes 3 first in sec
const uint8_t ScrSaveTimes[8] = { 5, 15, 30, 1, 5, 15, 30, 0 }; //, 255 }; //{ 1, 2, 5, 10, 15, 20, 30, 0 };
const uint8_t ScrMainTimes[6] = { 30, 60, 5, 10, 15, 20 };
const uint8_t ScrChargeTimes[4] = { 0, 10, 30, 60 };

uint8_t		EditItemIndex;
uint16_t	EditModeTimer;

uint8_t		ShowWeakBatFlag;
uint8_t		BatAnimLevel;
uint8_t         SwitchOffCase;

//=========================================================================
// Change Screen
void SetScreen( int screen, int duration )
{
	Screen = screen;
	ScreenDuration = duration;
	gFlags.refresh_display = 1;
}

//=========================================================================
void DarkScreen()
{
    	Screen = 0;
	SleepTimer = dfDimOffTimeout * 100; //18000;
	gFlags.refresh_display = 1;
}


//=========================================================================
void Sleep0Screen()
{
        Screen = 0;
	SleepTimer = 0;
        gFlags.refresh_display = 1;
}

//=========================================================================
// Called at a frequency of 10Hz except when firing in TC modes.
// Called at a frequency of 2Hz when firing in TC modes.

__myevic__ void DrawScreen()
{
	static uint8_t	TenthOfSecs = 0;
	static uint16_t	CurrentFD = 0;
        static uint8_t scrSaveOnce = 1;
        static uint8_t StealthPuffs = 0; //dfStealthPuffsCnt

	if ( ( !PE0 || AutoPuffTimer ) && Screen == 2 && FireDuration && FireDuration != CurrentFD )
	{
		CurrentFD = FireDuration;
		//ScreenDuration = ISMODETC(dfMode) ? 1 : 2;
                ScreenDuration = dfFireScrDuration; //2;                
		TenthOfSecs = 0;
		gFlags.refresh_display = 1;
	}
	else if ( ScreenRefreshTimer && !--ScreenRefreshTimer )
	{
		if ( Screen != 2 ) 
                    gFlags.refresh_display = 1;
	}
        else if ( !dfFireScrDuration && Screen == 2 && !gFlags.firing )
        {
                    TenthOfSecs = 10; //no wait sec           
                    gFlags.refresh_display = 1;                    
        }

	if ( gFlags.refresh_display )
	{
		gFlags.refresh_display = 0;
                
                if ( Screen == 1 && !HideLogo && dfStatus2.anim3d && !EditModeTimer )
                {
                    DrawFillRect( 0, 0, 63, 44, 0 ); //clear spaces, no flick in 3d
                    DrawFillRect( 0, 113, 63, 127, 0 ); //
                }
                else
                {
                    ClearScreenBuffer();
                }

		switch ( Screen )
		{
			case 0: // Black
                                if ( dfStatus.off ) StealthPuffs = 0;
                                
				break;

			case 1: // Main view
			//case  3: // Main view (?)
			//case  4: // (unused?)

                                if ( dfStealthOn == 1 && LastEvent == 15 && FireDuration < 4 ) //2 if KeyPressTime = 6
                                {
                                        StealthPuffs = dfStealthPuffsCnt;       
                                }
                        
				ShowMainView(); 
				break;

			case 2: // Firing

				if ( dfStealthOn != 2 && !gFlags.MainContrast )
				{
                                        DisplaySetContrast( dfContrast );
                                        gFlags.MainContrast = 1;
                                }
                                else if ( dfStealthOn == 2 && gFlags.MainContrast )
                                {
                                        DisplaySetContrast( dfContrast2 );
                                        gFlags.MainContrast = 0;
                                }
                                                                   
                                if ( ( dfStealthOn != 1 || StealthPuffs ) || ShowWeakBatFlag ) 
                                {
                                        ShowMainView();
                                }

				break;

			case 5: // charge screen
                            
                                if ( gFlags.MainContrast )
                                {
                                    DisplaySetContrast( dfContrast2 );
                                    gFlags.MainContrast = 0;
                                }
 
				ShowBatCharging();
				break;

			case 20: // No Atomizer Found
				ShowNoAtoFound();
				break;

			case 21: // Atomizer Short
				ShowAtoShort();
				break;
                                
                        case 70: // Atomizer Short Current
				ShowAtoShortCurrent();
				break;
                                
                        case 71: // Atomizer Short Bad
				ShowAtoShortBad();
				break;
                                
			case 22: // Atomizer Low
				ShowAtoLow();
				break;

			case 23: // 10s Protection
				Show10sProtec();
				break;

			case 24: // Battery Low
				ShowBatLow();
				break;

			case 25: // Battery Low Lock
				ShowBatLowLock();
				break;

			case 28: // Key Lock
				ShowKeyLock();
				break;

			case 29: // Device too hot
				ShowDevTooHot();
				break;

			case 31: // Key UnLock
				ShowKeyUnLock();
				break;

			//case 37: // Board Temp
			//	ShowBoardTemp();
			//	break;

			case 40: // Stealth ON/OFF
				ShowStealthMode();
				break;

			case 50: // FW Version
				ShowVersion();
				break;

			case 51: // New Coil
				ShowNewCoil();
				break;

			case 54: // Battery Voltage
				ShowBattVolts();
				break;

			case 55: // Imbalanced Batteries
				ShowImbBatts();
				break;

			case 56: // Check Battery
				ShowCheckBattery();
				break;

			case 57: // Check USB Adapter
				ShowCheckUSB();
				break;

			case 58: // Charge Error
				ShowChargeError();
				break;

			case 60: // Screen Saver
                            
                                if ( dfStatus.off )
                                {
                                    DisplaySetContrast( dfContrast ); //main contrast in off state fire clock 
                                    gFlags.MainContrast = 1;
                                }
                                else
                                {
                                    //if ( gFlags.MainContrast )
                                    //{
                                        DisplaySetContrast( dfContrast2 );
                                        gFlags.MainContrast = 0;
                                    //}
                                }
                               
                                scrSaveOnce = 0;
                                ScreenRefreshTimer = 0;
                                
                                if ( dfStatus.off || !ISANIMSAVER(dfScreenSaver) 
                                        || ( ISANIMSAVER(dfScreenSaver) && !gFlags.animready ) )
                                {
                                    //call anim once, other in AnimateScreenSaver()
                                    //call non anim while duration
                                    // dfStatus.off for clock in off state
                                    ShowScreenSaver();
                                }

				break;
                                
			case 61: //goodbye
                                DisplaySetContrast( dfContrast2 );
                                gFlags.MainContrast = 0;
                                ShowGoodBye();
				break;
                                
			//case 100:
			//	ShowInfos();
			//	break;

			case 101:
				ShowContrast();
				break;

			case 102:
				//ShowMenus();
                                DrawMenu();
				break;

			case 103:
				ShowRTCSpeed();
				break;

			case 104:
				ShowRTCAdjust();
				break;

			case 105:
				ShowSetTimeDate();
				break;

			//case 106:
			//	ShowSetDate();
			//	break;

			case 107:
				ShowPowerCurve();
				break;
                                
                        case EVENT_SET_JOULES: //scr = event
				ShowSetJoules();
				break;        
                                                               
			default:
				break;
		}
                
		DisplayRefresh();
	}



//onscreen debug
        
// scr edges x,y       
//        
// (DrawValue)   0,0 ...   64,0(DrawValueRight)
//
//
//
// (DrawValue) 0,108 ... 64,108(DrawValueRight)
        
//DrawValue( 0, 108, Screen, 0, 0x01, 0 );
//DrawValue( 20, 108, LastEvent, 0, 0x01, 0 );
//DrawValueRight( 38, 108, gFlags.inverse, 0, 0x01, 0 );
//DrawValueRight( 64, 108, dfTempAlgo, 0, 0x01, 0 );
        
//DrawValue( 0, 108, gFlags.apuff ? 1 : 0, 0, 0x01, 0 );    
//DrawValueRight( 64, 108, dfStatus.endlessfire, 0, 0x01, 0 );
//DrawValue( 0, 0, StealthPuffs, 0, 0x01, 0 );
//DrawValueRight( 64, 0, FireDuration ? : 0, 0, 0x01, 0 );
//DrawValueRight( 64, 0, LastEvent, 0, 0x01, 0 ); 
//NextPreheatTimer UserInputs AutoPuffTimer FireDuration
//UserInputs LastInputs TargetVolts AtoRez
//FireClicksEvent  FireClickCount CurrentFD ScreenDuration PreheatDelay SleepTimer KeyUpTimer
//dfResistance AtoRez dfTempAlgo StealthPuffs
//gFlags.apuff && dfStatus.endlessfire

//    time_t t,mn;
//    RTCGetEpoch ( &t );
//    mn = RTCReadRegister( RTCSPARE_MIDNIGHT );
//    DrawValue( 0, 37, mn, 0, 0x01, 0 );
//    DrawValue( 0, 108, t, 0, 0x01, 0 );
// dfStatus2.reztype ? 1: 0
//DrawValue( 0, 0, dfStatus2.reztype ? 1: 0, 0, 0x0B, 0 ); //BoostDuty  BuckDuty PWMCycles
//DrawValueRight( 64, 108, sleep_ticks, 0, 0x0B, 0 );     //gFlags.asleep ? 1 : 0      dfTempAlgo
//DrawValue( 0, 108, GetLockState(), 0, 0x0B, 0 ); //SwitchOffCase AwakeTimer TCR SetTimeRTD.u32Day
        
//DrawValue( 0, 0, BatteryStatus, 0, 0x0B, 0 );
//DrawValueRight( 64, 0, ChargeStatus, 0, 0x0B, 0 );         
//DrawValueRight( 64, 108, ChargerDuty, 0, 0x0B, 0 );
//DrawValue( 0, 108, GetLockState(), 0, 0x0B, 0 );
        
//DrawValue( 0, 0, AtoProbeCount, 0, 0x0B, 0 ); //gFlags.user_idle ? 1 : 0
//DrawValueRight( 64, 0, AtoRezMilli, 0, 0x0B, 0 );  //SleepTimer PreheatPower
//DrawValueRight( 64, 108, TargetVolts, 0, 0x0B, 0 ); //NextPreheatPower

//DisplayRefresh(); //uncomment too


	if ( ( Screen == 1 || Screen == 60 ) && ( ScreenDuration <= 4 ) )
	{
		if ( !gFlags.fading  )
		{
			FadeOutTimer = 300;
			gFlags.fading = 1;
		}
	}
	else if ( gFlags.fading )
	{
		FadeOutTimer = 0;
		//DisplaySetContrast( dfContrast );
                gFlags.MainContrast = 0; //need for interrupt fading
		gFlags.fading = 0;
	}

	if (( gFlags.firing ) && ISMODETC(dfMode))
		TenthOfSecs += 5;
	else
		TenthOfSecs += 1;

	if ( TenthOfSecs < 10 )
		return;

	TenthOfSecs = 0;
        //                              every second now:

	if (  100 * ScreenDuration < EditModeTimer )
		ScreenDuration = EditModeTimer / 100 + 1;

	if ( ScreenDuration && --ScreenDuration )
		return;

	switch ( Screen ) //                EXIT from screens 
	{
		case   0: // Black
			if ( dfStatus.off )
                        {
				SleepTimer = 0;
                        }
			else
			{
                                uint16_t spt = GetScreenProtection();
                                
				if (( !gFlags.firing )
				&&	( dfStealthOn != 1 )
				&&	( SleepTimer > 0 )
				&&	( dfScreenSaver > 0 )
				&&	( spt > 0 )
                                &&	( scrSaveOnce ) 
                                        
                                        )
				{
                                        gFlags.animready = 0;
                                        SetScreen( 60, spt );
					//Screen = 60;
					//ScreenDuration = GetScreenProtection();
					//gFlags.refresh_display = 1;
                                        //scrSaveOnce = 0;
				}
			}
			break;
                        
                //case  28: // Key Lock
                //        scrSaveOnce = 0;
                //        DarkScreen();
                //        break;
    
		case   2: // Firing
                        if ( dfStealthOn == 1 && StealthPuffs && FireDuration > 1 ) //CurrentFD
                        {
                            --StealthPuffs;
                        }
                        //no brake
		case  28: // Key Lock
                case  31: // Key UnLock
		case  40: // Stealth ON/OFF
                       
                        if ( dfStealthOn != 1 )
                        {
                                StealthPuffs = 0;
                        }
                        
			if ( dfStealthOn == 1 && !StealthPuffs )
			{
                                DarkScreen();
			}
			else
			{
                                LinePuffAwakeTimer = 300; //3s
                                //not here VapedLineTimer = dfFireScrDuration * 100;
				MainView();
			}
			break;

		case   5: // charge screen
                        if ( ScrChargeTimes[dfScrChargeTime] ) //|| dfStealthOn != 1 )
                        {
                            gFlags.screen_on = 0;
                        }
                    
                    	break;
                        
/*                          
                            // always stay on
		case  22: // Atomizer Low
		case  24: // Battery Low
		case  25: // Battery Low Lock
		//case  50: // FW Version
			break; 
*/

		case  23: // 10s Protection
			if ( !dfStatus.off )
			{
				// Switch box off after xx sec if no response
				// have been given to a long fire.
				if ( !PE0 && gFlags.user_idle )
				{
                                        SwitchOffCase = 3;
					Event = 17; //on-off mod
				}
				else if ( PE0 )
				{
                                        scrSaveOnce = 1;
                                        MainView();
                                        //DarkScreen();
				}
			}
			break;

                case  22: // Atomizer Low
		case  24: // Battery Low
		case  25: // Battery Low Lock
                    
		case  20: // No Atomizer Found
		case  21: // Atomizer Short
		case  29: // Device too hot
		//case  31: // Key UnLock
		case  51: // New Coil
		case  55: // Imbalanced Batteries
		case  56: // Check Battery
		case  57: // Check USB Adapter
		case  58: // Charge Error
                case  70: // Atomizer Short current error
                case  71: // Atomizer Short contacts check
			MainView();
			break;

		case 101: // Contrast Menu
		case 102: // Menus
		case 103: // RTC Speed
		case 104: // Adjust Clock
		case 105: // Set TimeDate
		//case 106: // Set Date
		case 107: // Power Curve
                case EVENT_SET_JOULES:    //screen 123
			EditModeTimer = 0;
			gFlags.edit_capture_evt = 0;
			gFlags.edit_value = 0;
			//LEDOff();
			UpdateDataFlash();
                        //DisplaySetContrast( dfContrast );
                        //scrSaveOnce = 1;
			//NOBREAK
		case   1: // Main view
                case  50: // FW Version
		case  37: // Board Temp
		case  54: // Battery Voltage
		case 100: // Infos page
                        scrSaveOnce = 1;
                        //NOBREAK
                case  60: // Screen Saver                        
			if ( gFlags.battery_charging )
			{
				ChargeView();

				if ( dfStealthOn == 1 || !gFlags.screen_on )
				{
					ScreenDuration = 0;
				}
			}
			else
			{
                                DarkScreen();
			}
                         
			break;
                        
                case  61: // goodbye
			//Screen = 0;
			//SleepTimer = 0;
                        //gFlags.refresh_display = 1;
                        Sleep0Screen();
                        break;
                        
		default:
			break;
	}

	return;
}

//=========================================================================

__myevic__ uint16_t GetScreenProtection()
{
    if ( dfScreenProt < 3 ) 
    {
        return ( ScrSaveTimes[dfScreenProt] ); //in sec
    }
    else
    {
        return ( 60 * ScrSaveTimes[dfScreenProt] );  //in min
    }
}

/*
__myevic__ uint16_t GetMainScreenDuration()
{
	//return dfDimTimeout ? : ScrMainTimes[dfScrMainTime];
        return ScrMainTimes[dfScrMainTime];
}
*/


//=========================================================================

__myevic__ int convert_string1( uint8_t *strbuf, const char *s )
{
	int i;
	char c;

	i = 0;
	while (( c = *s++ ) && ( i < 20 ))
	{
		if (( c >= '0' ) && ( c <= '9' ))
		{
			strbuf[i++] = c - '0' + 0x0B;
		}
		else if (( c >= 'A' ) && ( c <= 'Z' ))
		{
			strbuf[i++] = c - 'A' + 0x9C;
		}
		else if (( c >= 'a' ) && ( c <= 'z' ))
		{
			strbuf[i++] = c - 'a' + 0x82;
		}
		else if ( c == '/' )
		{
			strbuf[i++] = c - '/' + 0xD6;
		}
		else                    
		{
			strbuf[i++] = 0xBC;
		}
	}
	strbuf[i] = 0;
	return i;
}


//=========================================================================

__myevic__ void ChargeView()
{
    //ShowBatCharging();
	Screen = 5;
	ScreenDuration = ScrChargeTimes[dfScrChargeTime];
        SleepTimer = dfDimOffTimeout * 100; //fo reset session counters
	gFlags.refresh_display = 1;  
}


//=========================================================================

/*
__myevic__ void ShowInfos()
{
	uint8_t strbuf[20];

	convert_string1( strbuf, "Ferox" );
	DrawStringCentered( strbuf, 71 );
        convert_string1( strbuf, "MicroSur" );
        DrawStringCentered( strbuf, 82 );
	convert_string1( strbuf, "were" );
	DrawStringCentered( strbuf, 92 );
	convert_string1( strbuf, "here" );
	DrawStringCentered( strbuf, 102 );

	return;
}
*/

//=========================================================================

__myevic__ void ShowContrast()
{
	int pc, nd, x, t;
        uint8_t dfc;

        dfc = ContrastNum ? dfContrast2 : dfContrast;   

        DrawValueRight( 63, 5, ContrastNum+1, 0, 0x0B, 1 );
	DrawString( String_Contrast, 4, 5 );
	DrawHLine( 0, 16, 63, 1 );

	pc = ( ( 100 * dfc ) / 255 );
	nd = ( pc < 100 ? pc < 10 ? 1 : 2 : 3 );
	x = ( 64 - ( 6 * nd + 9 )) / 2;
	DrawValue( x, 20, pc, 0, 0x0B, 0 );
	DrawImage( x + 6 * nd, 20, 0xC2 );

	DrawFillRect( 0, 32, 63, 44, 1 );
	DrawFillRect( 1, 33, 62, 43, 0 );
	if ( dfc )
	{
		DrawFillRect( 2, 34, 2 + ( ( 59 * dfc ) / 255 ), 42, 1 );
	}

	DrawStringCentered( String_LongFire, 53 );
	DrawStringCentered( String_Exit, 64 );
        
        t = dfStatus.nologo;
	dfStatus.nologo = 0;
        DrawLOGO( 0, 80 );
        dfStatus.nologo = t;
}


//=========================================================================

/*
__myevic__ void ShowMenus()
{
	DrawMenu();
}
*/


//=========================================================================

__myevic__ void ShowRTCSpeed()
{
	unsigned int cs;
	S_RTC_TIME_DATA_T rtd;

	DrawString( String_ClkSpeed, 4, 5 );
	DrawHLine( 0, 16, 63, 1 );

	GetRTC( &rtd );
	//DrawTimeSmall( 10, 25, &rtd, 0x1F );
        DrawTime( 3, 29, &rtd, 0x1F );

	if ( gFlags.has_x32 )
	{
		DrawString( String_X32, 11, 48 );
		DrawString( String_On, 37, 48 );
	}
	else
	{
		cs = RTCGetClockSpeed();
		DrawValue( 12, 48, cs, 0, 0x1F, 5 );
	}
}


//=========================================================================

__myevic__ int IsClockOnScreen()
{
        return (  ( ( Screen == 1 ) && ( ( dfAPT == 8 ) || ( dfAPT3 == 8 ) ) )
	//return (  ((( Screen == 1 ) || ( Screen == 2 )) && ( ( dfAPT == 8 ) || ( dfAPT3 == 8 ) ) )
			|| (( Screen == 1 ) && ( dfStatus.clock || VapeDelayTimer ))
			|| (( Screen == 60 ) && ( dfScreenSaver == SSAVER_CLOCK ))
			||  ( Screen == 103 )
			||  ( Screen == 104 )
			);
}


//=========================================================================
//----- (000067C8) --------------------------------------------------------
__myevic__ void ShowBattery()
{
    //draw battery line
    
        int i;	
        int x0;
        int y0;
        int y;
        
        int line = ISINVOKE ? 116 : 118; //ISINVOKE
        
        if ( BLINKITEM(6) ) return;

        uint16_t bv = gFlags.firing ? RTBattVolts : BatteryVoltage;
        
        if ( dfBattLine == 2 ) //volts + small 
		{
			//uint16_t bv = gFlags.firing ? RTBattVolts : BatteryVoltage;
			DrawValueRight(	21, line, bv, 2, 0x0B, 0 );
			DrawImage( 22, line, 0x97 ); //v
		}
	else if ( dfBattLine == 1 ) //percent + small
		{
			DrawValueRight(	17, line, BatteryPercent, 0, 0x0B, 0 );
			DrawImage( 18, line, 0xC2 ); //%
		}
        else if ( dfBattLine == 0 ) //percent+volts (was big)
        {
                DrawValueRight(	17, line, BatteryPercent, 0, 0x0B, 0 ); 
                DrawImage( 19, line, 0xC2 ); //%
                
                //uint16_t bv = gFlags.firing ? RTBattVolts : BatteryVoltage;
		DrawValueRight(	56, line, bv, 2, 0x0B, 0 );
		DrawImage( 57, line, 0x97 ); //v
                
                // later for 10pc blink DrawVLineDots( 31, 114, 127 ); //vert line
                
        }

        // draw batts icons
        
	if ( gFlags.battery_10pc && !gFlags.battery_charging && !gFlags.draw_battery )
	{
/*
		if ( gFlags.draw_battery )
		{
			if ( dfBattLine == 1 || dfBattLine == 2 )
			{
				DrawImage( 30, line, 0xE2 ); //small empty
			}
			else if ( dfBattLine == 0 )
			{
				//DrawImage( 8, 114, 0xC4 ); //big empty
                                DrawVLine( 31, 114, 127, 0 );
			}
                        else if ( dfBattLine == 3 )
                        {
                                DrawImage( 1, line, 0xE2 ); //2 small empty
                                DrawImage( 33, line, 0xE2 );    
                        }
                }
*/
	}
	else if ( gFlags.draw_battery_charging && gFlags.battery_charging )
	{
		if ( dfBattLine == 1 || dfBattLine == 2 )
		{
			DrawImage( 30, line, 0xE3 ); //small charging
		}
		else if ( dfBattLine == 0 )
		{
			DrawVLine( 31, 114, 127, 1 );
		}
                else if ( dfBattLine == 3 )
		{
			DrawImage( 1, line, 0xE3 ); //2 small charging
                        DrawImage( 33, line, 0xE3 );
		}
	}
	else
	{
		if ( dfBattLine == 1 || dfBattLine == 2 )
		{
			DrawImage( 30, line, 0xE2 ); // 1 small empty
			if ( gFlags.batteries_ooe && gFlags.draw_battery )
			{
                                DrawImage( 37, line, 0xC6 ); //imba ico
			}
			else if ( BatteryTenth )
			{
				DrawFillRectLines( 32, line+2, (25 * BatteryTenth / 10 + 31), line+5, 1 ); //120 line+2, line+5 123
			}
		}
		else if ( dfBattLine == 0 )
		{
		//	DrawImage( 8, 114, 0xC4 );
			if ( gFlags.batteries_ooe && gFlags.draw_battery )
			{
                //                DrawImage( 23, 117, 0xC6 );
                                DrawVLine( 31, 114, 127, 1 );
			}
                        DrawVLineDots( 31, 114, 127 ); //vert line
		//	else if ( BatteryTenth )
		//	{
		//		DrawFillRectLines( 10, 119, (4 * BatteryTenth + 9), 123, 1 );
		//	}
		}
		else if ( dfBattLine == 3 )
		{
                        DrawImage( 1, line, 0xE2 ); //2 small empty
                        DrawImage( 33, line, 0xE2 ); 
			if ( gFlags.batteries_ooe && gFlags.draw_battery )
			{
                                DrawImage( 40, line, 0xC6 );
			}
			else 
			{
                            for ( i = 0 ; i < NumBatteries ; ++i )
                            {
                                if ( i == 0 || i == 1 )
                                {
                                    y0 = line+1; //119;
                                    y = line+3; //121;
                                    if ( i == 0 ) x0 = 2;
                                    else x0 = 34;
                                }
                                else
                                {
                                    y0 = line+4; //122;
                                    y = line+6; //124;
                                    if ( i == 2 ) x0 = 2;
                                    else x0 = 34;   
                                }
                
                                if ( BatteryTenthAll[i] )
                                {
                                    if ( NumBatteries == 2 )
                                        DrawFillRectLines( x0+1, y0+1, (25 * BatteryTenthAll[i] / 10 + x0), line+5, 1 ); //123 line+5
                                    else
                                        DrawFillRect( x0, y0, (25 * BatteryTenthAll[i] / 10 + x0+2), y, 1 );               
                                }
                            }				
			}
		}                
	}
}


//=========================================================================
//----- (00006764) --------------------------------------------------------
__myevic__ void ShowBatCharging()
{
    //Charge screen
    
	//if ( ( dfStealthOn == 1 && ScreenDuration == 0 ) || !gFlags.screen_on )
        if ( !gFlags.screen_on )            
	{
		return;
	}
               
        int line = ISINVOKE ? 116 : 118;
        
        if ( !gFlags.soft_charge )
            DrawDigitClock( 40, 0 );
        
/*
	switch ( dfScreenSaver )
	{
		case SSAVER_CLOCK:
		case SSAVER_LOGO:
			ShowScreenSaver();
			break;

		default:
			break;
	}
*/
  
	DrawValueRight(	18, line, BatteryPercent, 0, 0x0B, 0 );
	DrawImage( 19, line, 0xC2 );
	DrawImage( 30, line, 0xE2 );

        
	if ( BatteryTenth != 10 )
	{
		if ( BatAnimLevel )
		{
			DrawFillRectLines( 32, line+2, (25 * BatAnimLevel / 10 + 31), line+5, 1 );
		}
	}
	else if ( gFlags.draw_battery_charging )
	{
		DrawFillRectLines( 32, line+2, 56, line+5, 1 );
	}

/*
	if (( dfScreenSaver == SSAVER_CLOCK ) || ( dfScreenSaver == SSAVER_LOGO ))
	{
		DrawValue(  1, 104, BatteryVoltage, 2, 0x0B, 3 );
		DrawImage( 27, 104, 0x7D );
	}
	else
	{
*/
		for ( int i = 0 ; i < NumBatteries ; ++i )
		{
			DrawValue(  3, line-14 - i * 14, BattVolts[NumBatteries - i - 1], 2, 0x0B, 3 );
			DrawImage( 25, line-14 - i * 14, ( BBBits & (1 << i) ) ? 0x7D : 0x97 );
		}
                
                if ( NumBatteries > 1 )
                {
                DrawString( String_USB, 6, 0 );
                DrawValue(  31, 9, USBVolts, 2, 0x0B, 3 );
                DrawImage( 52, 9, 0x7D );
        
                DrawString( String_Charge, 6, 20 );
                DrawValue(  31, 34, ChargeCurrent / 10, 2, 0x0B, 3 );
                DrawImage( 52, 34, 0x68 ); //A
                } 
                else
                {
                    DrawString( String_Charge, 6, 0 );    
                }
        
	//}
        int t;
        if ( ISSINFJ200 || ISIKU200 )
        {
                t = dfStatus.IsCelsius ? AkkuTemp : CelsiusToF( AkkuTemp );
                DrawValueRight( 52, line-28, t, 0, 0x0B, 0 ); //90
                DrawImage( 54, line-28, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
        }
                        
	t = dfStatus.IsCelsius ? BoardTemp : CelsiusToF( BoardTemp );

	DrawValueRight( 52, line-14, t, 0, 0x0B, 0 );
	DrawImage( 54, line-14, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
}

//=========================================================================
//----- (0000683C) --------------------------------------------------------
__myevic__ void ShowBattVolts()
{
    //Event = 34;
    //SetScreen( 54
    int x, y, r;
    const BatV2P_t *v2p = Battery->V2P;
    
    DrawStringCentered( NumBatteries > 1? String_Batteries : String_Battery, 0 );
    
    //DrawVLineDots( const int x, const int y1, const int y2, const int color )
    DrawVLineDots( 0, 11, 43 );            
    //DrawHLineDots( const int x1, const int y, const int x2, const int color )
    DrawHLineDots( 0, 43, 63, 1 );
    for ( int i = 11 ; i >= 0 ; --i )
    {
    //int x = BatteryVoltsToPercent( v2p[i] );
    x = 60 - v2p[i].percent * 57 / 100;
    //x = map( v2p[i].percent, 0, 100, 60, 3);
    y = 43 - ( v2p[i].voltage - 270 ) * 30 / 153;
    //y = map( v2p[i].voltage, 270, 423, 43, 13);
    //DrawPoint( )
    r = BatteryPercent <= v2p[i].percent ? 1 : 2;
    //DrawCircle( int x_centre, int y_centre, int r, int color, int fill )
    DrawCircle( x, y, r, 1, 1 );
    }

    for ( int i = 0 ; i < NumBatteries ; ++i )
    {
        DrawValue(  8, 49+21*i, BattVolts[i], 2, 0x29, 3 );
        DrawImage( 50, 57+21*i, 0xB1 );
    }
}


//=========================================================================
//----- (00006874) --------------------------------------------------------
/*
__myevic__ void ShowBoardTemp()
{
	DrawStringCentered( String_Temp, 88 );
	DrawValue( 16, 102, BoardTemp, 0, 0x48, 2 );
}
*/

__myevic__ void DrawBoxNameCenter( int y )
{
        uint8_t strbuf[10];
        convert_string1( strbuf, BoxName );
        DrawStringCentered( strbuf, y );

}

//=========================================================================
//----- (00007684) --------------------------------------------------------
__myevic__ void ShowVersion()
{
    //about
    //info
    
	uint8_t buf[12];

        DrawStringCentered( String_SME, 2 );
	//DrawStringCentered( String_myevic, 24 );

	DrawString( String_Build, 2, 17 );
	Value2Str( buf, __BUILD1, 0, 0x1F, 0 );
	DrawString( buf, 15, 30 );

	DrawString( String_Version, 2, 47 );
	DrawValue( 35, 60, FWVERSION, 2, 0x1F, 3 );
        
        //
        
        DrawBoxNameCenter( 81 );
//        uint8_t strbuf[20];
//        convert_string1( strbuf, BoxName );
//        DrawStringCentered( strbuf, 81 );
        
        DrawString( String_Hard, 1, 96 );
        DrawString( String_Version, 26, 96 );
        DrawValue( 35, 109, dfHWVersion, 2, 0x1F, 3 );

}


//=========================================================================
//----- (000068D4) --------------------------------------------------------
__myevic__ void ShowNewCoil()
{
    //screen 51
    //event 32
	int rez;

	DrawStringCentered( String_NewCoil, 32 );
        DrawStringCentered( String_SameCoil, 66 );
	DrawImage( 43, 48, 0xC0 );
        DrawImage( 43, 82, 0xC0 );
	//DrawStringCentered( String_Right, 74 );
        DrawImage( 50, 32, 0xD4 );
        DrawImage( 11, 66, 0xD5 );

        DrawValue( 14, 46, dfResistance, 2, 0x1F, 3 );
        
	//if ( ISMODETC( dfMode ) )
	//{
		if ( dfMode == 0 )
		{
			rez = dfRezNI;
		}
		else if ( dfMode == 1 )
		{
			rez = dfRezTI;
		}
		else if ( dfMode == 2 )
		{
			rez = dfRezSS;
		}
		else //( dfMode == 3 )
		{
			rez = dfRezTCR;
		}
        
		DrawValue( 14, 80, rez, 2, 0x1F, 3 );
	//}
	
}


//=========================================================================
//----- (000076AC) --------------------------------------------------------
__myevic__ void ShowStealthMode()
{
	DrawStringCentered( String_Stealth, 88 );
        //DrawStringCentered( dfStealthOn ? String_ON : String_OFF, 102 );
        if ( !dfStealthOn )
            DrawStringCentered( String_Off, 102 );
        else if ( dfStealthOn == 1 )
            DrawStringCentered( String_On, 102 );
        else
            DrawStringCentered( String_Contrast, 102 );
}


//=========================================================================
//----- (000076DC) --------------------------------------------------------
__myevic__ void ShowDevTooHot()
{
	DrawStringCentered( String_Device, 88 );
	DrawStringCentered( String_TooHot, 102 );
        
/*
        int t;
        if ( ISSINFJ200 || ISIKU200 )
        {
                t = dfIsCelsius ? AkkuTemp : CelsiusToF( AkkuTemp );
                DrawValueRight( 52, 0, t, 0, 0x0B, 0 );
                DrawImage( 54, 0, dfIsCelsius ? 0xC9 : 0xC8 );
        }
                        
	t = dfIsCelsius ? BoardTemp : CelsiusToF( BoardTemp );

	DrawValueRight( 52, 14, t, 0, 0x0B, 0 );
	DrawImage( 54, 14, dfIsCelsius ? 0xC9 : 0xC8 );
*/
}


//=========================================================================
//----- (00007718) --------------------------------------------------------
__myevic__ void ShowAtoLow()
{
	DrawStringCentered( String_Atomizer, 88 );
	DrawStringCentered( String_Low, 102 );
}


//=========================================================================
//----- (00007734) --------------------------------------------------------
__myevic__ void ShowAtoShort()
{
	DrawStringCentered( String_Atomizer, 88 );
	DrawStringCentered( String_Short, 102 );
}
__myevic__ void ShowAtoShortCurrent()
{
    //from read ato current 70
	DrawStringCentered( String_Atomizer, 88 );
	DrawStringCentered( String_Error, 102 );
}
__myevic__ void ShowAtoShortBad()
{
    //bad contacts 71
        DrawStringCentered( String_Check, 88 );
	DrawStringCentered( String_Atomizer, 102 );
}

__myevic__ void ClearUpLine()
{
    ShowMainView();
    DrawFillRect( 0, 0, 63, 9, 0 );
}

//=========================================================================
//----- (00007750) --------------------------------------------------------
__myevic__ void ShowBatLow()
{
    //screen 24, event 28
	//DrawStringCentered( String_Battery, 88 );
	//DrawStringCentered( String_Low, 102 );
    
    //ShowMainView();
    //DrawFillRect( 0, 0, 63, 9, 0 );
    
    ClearUpLine();

    DrawString( String_Low, 7, 0 );    
    DrawImage( 37, 0, 0xEA ); //battery   
}


//=========================================================================
//----- (0000776C) --------------------------------------------------------
__myevic__ void ShowBatLowLock()
{
    //screen 25 event 1
//	DrawStringCentered( String_Battery, 82 );
//	DrawStringCentered( String_Low, 92 );
//	DrawStringCentered( String_Lock, 102 );
        
    ClearUpLine();
    
    DrawString( String_Lock, 7, 0 );
    DrawImage( 37, 0, 0xEA ); //battery   
}


//=========================================================================
//----- (00007794) --------------------------------------------------------
__myevic__ void ShowKeyLock()
{
    if ( dfStatus2.replay && ISMODEVW(dfMode) )
        DrawStringCentered( String_Repeat, 18 );
    
    DrawStringCentered( String_Key, 88 );
    DrawStringCentered( String_Lock, 102 );
}


//=========================================================================
//----- (000077B0) --------------------------------------------------------
__myevic__ void ShowKeyUnLock()
{
	DrawStringCentered( String_Key, 88 );
	DrawStringCentered( String_UnLock, 102 );
}


//=========================================================================
//----- (000077CC) --------------------------------------------------------
__myevic__ void ShowNoAtoFound()
{
	DrawStringCentered( String_No, 82 );
	DrawStringCentered( String_Atomizer, 92 );
	//DrawStringCentered( String_Found, 102 );
}


//=========================================================================
//----- (000077F4) --------------------------------------------------------
__myevic__ void Show10sProtec()
{
    //screen 23, event 24
//    ShowMainView();
//    DrawFillRect( 0, 0, 63, 9, 0 );
    
    ClearUpLine();
    DrawStringCentered( String_LongFire, 0 ); //116   
}


//=========================================================================
//----- (00007810) --------------------------------------------------------
__myevic__ void ShowWeakBat()
{
	//DrawStringCentered( String_Weak, 110 );
	//DrawStringCentered( String_Battery, 119 );
    //    DrawString( String_Weak, 8, 114 );
    //    DrawString( String_BATT_s, 37, 114 );
    
    //no ClearUpLine(); 
    DrawFillRect( 0, 0, 63, 9, 0 );
    DrawString( String_Weak, 7, 0 );
    DrawImage( 37, 0, 0xEA ); //battery   	
}


//=========================================================================
__myevic__ void ShowRTCAdjust()
{
	S_RTC_TIME_DATA_T rtd;

	DrawString( String_ClkAdjust, 4, 5 );
	DrawHLine( 0, 16, 63, 1 );

	GetRTC( &rtd );
	DrawTime( 3, 40, &rtd, 0x1F );
}


//=========================================================================
__myevic__ void ShowGoodBye()
{
    //screen = 61
    //DrawStringCentered( String_Off, 63 );
    //SetScreen( 61, SwitchOffCase ? 3 : 2 ); //goodbye
    
    DrawStringCentered( ScreenDuration > 1 ? String_On : String_Off, 63 );
    
    switch ( SwitchOffCase )
    {                       
                case 1:
                    DrawStringCentered( String_PuffsOff, 100 );
                    break;
                    
                case 2:
                    DrawStringCentered( String_VapeTimeOff, 100 );
                    break;
                    
                case 3:
                    DrawStringCentered( String_LongFire, 100 );
                    break;
                    
                case 4:
                    DrawStringCentered( String_Sleep, 100 );
                    break;
                    
		default:
			break;                    
    }
  
    gFlags.refresh_display = 1;
}

//=========================================================================
__myevic__ void ShowScreenSaver()
{
        if ( dfStatus.off )
        {
                DrawDigitClock( 82, 0 );
                DrawClock( 0 );
                gFlags.refresh_display = 1;
                return;
        }
        
        // and for anim init, other call in AnimateScreenSaver()
        
	switch ( dfScreenSaver )
	{
		case SSAVER_CLOCK:
                        
                        DrawDigitClock( 82, 0 );
                        DrawClock( 0 );           
			break;

		//case SSAVER_3D: //not need init...
		//	anim3d( 1 );
		//	break;

		case SSAVER_LOGO:
		{
			int h = GetLogoHeight();
			if ( h )
			{
				DrawLOGO( 0, 32 - ( h - 48 )/2 );
			}
			break;
		}

		//case SSAVER_QIX:
		//	qix( 1 );
		//	break;

		//case SSAVER_SNOW:
		//	Snow( 1 );
		//	break;
                
		//case SSAVER_SF:
		//	StarField( 1 );
		//	break;
                
		case SSAVER_SPLASH:
                        ShowSplash();
			break;

		default:
			break;
        }
        
        SetRandSeed( TMR1Counter );
        gFlags.animready = 1; //allow AnimateScreenSaver() anti first-flick
              
}


//=========================================================================
__myevic__ void AnimateScreenSaver()
{
            
	switch ( dfScreenSaver )
	{
		case SSAVER_3D:
			anim3d();
			break;

		case SSAVER_QIX:
			qix();
			break;

		case SSAVER_SNOW:
			Snow();
			break;
                        
                case SSAVER_SF:
			StarField();
			break;

		default:
			break;
	}
}

//=========================================================================
__myevic__ void ShowSetTimeDate()
{
    //105 screen
    
	DrawString( String_Set, 4, 5 );
	DrawHLine( 0, 16, 63, 1 );
        
    //or debug
/*
    S_RTC_TIME_DATA_T rtd;
    GetRTC( &SetTimeRTD );
    time_t t, ref;
    uint32_t cs;
    cs  = RTCGetClockSpeed();
    RTC_GetDateAndTime( &rtd );
    RTCTimeToEpoch( &t, &rtd );
    ref = RTCReadRegister( RTCSPARE_REF_DATE );
    DrawValue( 0, 0, t, 0, 0x0B, 0 );
    DrawValue( 0, 10, ref, 0, 0x0B, 0 );    
    DrawValue( 0, 20, cs, 0, 0x0B, 0 ); 
    DrawValue( 0, 30, ClockCorrection, 0, 0x0B, 0 ); 
*/
    // end debug
    
        uint8_t EdItInd;
        int line = 29;
        
        //__myevic__ void DrawTime( int x, int y, S_RTC_TIME_DATA_T *rtd, int colors )

        if ( EditItemIndex > 2 ) 
        {
            EdItInd = EditItemIndex - 3;
        
            DrawTime( 3, line, &SetTimeRTD, 0x1F & ~( 1 << ( EdItInd << 1 ) ) );
            DrawDate( 4, line+15, &SetTimeRTD, 0x1F );
        }
        else
        {
            //date
            const uint8_t cols[4][3] =
            {
            	{ 0x1E, 0x1B, 0x0F },
            	{ 0x1E, 0x0F, 0x1B },
            	{ 0x1E, 0x1B, 0x0F },
		{ 0x0F, 0x1B, 0x1E }
            };

            int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
            int col = cols[f][EditItemIndex];

            //S_RTC_TIME_DATA_T rtd;
            //GetRTC( &rtd );
            //DrawString( String_Date, 4, 5 );
            //DrawHLine( 0, 16, 63, 1 );

            DrawTime( 3, line, &SetTimeRTD, 0x1F ); //rdt
            DrawDate( 4, line+15, &SetTimeRTD, col );
        
        }
        
        DrawStringCentered( String_LongFire, 83 );
	DrawStringCentered( String_Save, 94 );
        
}


//=========================================================================
/*
__myevic__ void ShowSetDate()
{
	const uint8_t cols[4][3] =
	{
		{ 0x1E, 0x1B, 0x0F },
		{ 0x1E, 0x0F, 0x1B },
		{ 0x1E, 0x1B, 0x0F },
		{ 0x0F, 0x1B, 0x1E }
	};

	int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
	int col = cols[f][EditItemIndex];

	S_RTC_TIME_DATA_T rtd;

	GetRTC( &rtd );

	DrawString( String_Date, 4, 5 );
	DrawHLine( 0, 16, 63, 1 );

	DrawTime( 3, 46, &rtd, 0x1F );
	DrawDate( 4, 64, &SetTimeRTD, col );
        
        DrawStringCentered( String_LongFire, 103 );
	DrawStringCentered( String_Save, 114 );
}
*/


//=========================================================================
__myevic__ int IsMenuScreen()
{
	return (
                ( Screen >= 101 && Screen <= 107 ) 
                || Screen == EVENT_SET_JOULES //vaped scr
               );
}


//=========================================================================
__myevic__ void ShowCheckBattery()
{
  DrawStringCentered( String_Check, 88 );
  DrawStringCentered( String_Battery, 102 );
}


//=========================================================================
__myevic__ void ShowCheckUSB()
{
	DrawStringCentered( String_Check, 80 );
	DrawStringCentered( String_USB, 92 );
	//DrawStringCentered( String_Adapter, 102 );
}


//=========================================================================
__myevic__ void ShowChargeError()
{
	DrawStringCentered( String_Charge, 88 );
	DrawStringCentered( String_Error, 102 );
}


//=========================================================================
__myevic__ void ShowImbBatts()
{
    //scr 55
    
	DrawStringCentered( String_Imbalanced, 88 );
	DrawStringCentered( String_Batteries, 102 );
}


//=========================================================================
__myevic__ void ShowPowerCurve()
{
    	//DrawHLine( 6,  19,  58, 1 );
	//DrawHLine( 6, 119,  58, 1 );

	//DrawVLine( 6,  19, 119, 1 ); //100 h
	//DrawVLine( 58, 19, 119, 1 );
        
        DrawFillRect(6, 19, 58, 120, 1);
        DrawFillRect(7, 20, 57, 119, 0);
        
        DrawVLineDots( 32, 20, 120 ); //100% line

	int t = EditItemIndex; // * 5;
	int j = -1;

	for ( int i = 0; i < PWR_CURVE_PTS; ++i )
	{
		int t1, t2;

		t1 = dfPwrCurve[i].time; // 50 h

		if ( ( i > 0 ) && ( t1 == 0 ) )
			break;

		if ( i < PWR_CURVE_PTS - 1 )
		{
			t2 = dfPwrCurve[i+1].time;

			if ( t2 == 0 ) t2 = 50;
                        
                        //connecting vert lines for pretty view
                        DrawHLine( 7 + dfPwrCurve[i+1].power / 4,
                                20 + 2 * t2,
                                7 + dfPwrCurve[i].power / 4,
                                1 );		
		}
		else
		{
			t2 = 50;
                        }
  
                //DrawVLine( const int x, const int y1, const int y2, const int color )
                
		DrawVLine( 7 + dfPwrCurve[i].power / 4,
                            20 + 2 * t1,
                            19 + 2 * t2,
                            1 );
                               
		if (( t2 > t ) && ( j < 0 ))
		{
			j = i;

			if ( t == t1 )
			{
/*
				DrawFillRect(	7,
								21 + 2 * t1, // / 5,
								7 + dfPwrCurve[i].power / 4,
								22 + 2 * t1, // / 5,
								1 );
*/
                            //DrawHLine( const int x1, const int y, const int x2, const int color )
                            // pointer line
                            DrawHLine( 7,  20 + 2 * t1,  7 + dfPwrCurve[i].power / 4, 1 );
			}
		}
                //DrawLine( int x1, int y1, int x2, int y2, int color, int thick )
                //DrawLine( 7 + dfPwrCurve[i].power / 4,
                //                    20 + 2 * t1, 
                //                    7 + dfPwrCurve[i+1].power / 4, 
                //                    19 + 2 * t2, 
                //                    1, 1 );                                           
	}
        
	if ( !gFlags.edit_value || gFlags.draw_edited_item )
	{
		DrawImage( 2, 16 + EditItemIndex *2, 0xD4 ); // ">"
	}

	//DrawImage( 12, 3, 0xAF ); // T
	//DrawValueRight( 44, 3, t, 1, 0x0B, 0 );
        DrawValue( 6, 5, t, 1, 0x0B, 0 );
	DrawImage( 22, 5, 0x94 );

	//DrawImage( 12, 13, 0xAB ); //P
	//DrawValueRight( 44, 13, dfPwrCurve[j].power, 0, 0x0B, 0 );
        DrawValueRight( 50, 5, dfPwrCurve[j].power, 0, 0x0B, 0 );
	DrawImage( 52, 5, 0xC2 );
}


//=========================================================================
/*
__myevic__ int SplashExists()
{
	int i, h, l;
	const image_t *img = Images[0xFF-1];

	h = img->height;
	l = img->width * h / 8;
	
	if ( img->width != 64 ) return 0;

	for ( i = 0 ; i < l ; ++i )
		if ( img->bitmap[i] ) break;

	return ( ( l && i < l ) ? 1 : 0 );
}
*/


__myevic__ void ShowSplash()
{
//         		  off	on	box
//dfStatus2.splash0         0	1	0
//dfStatus2.splash1         0	0	1

	//if ( gFlags.splash )
	//{
    
	DrawImage( 0, 0, 0xFF );
                
        if ( !dfStatus2.splash0 && dfStatus2.splash1 )
        {
            DrawBoxNameCenter( 115 );
            //    uint8_t strbuf[20];
            //    convert_string1( strbuf, BoxName );
            //    DrawStringCentered( strbuf, 115 );
        }
        
	//}
	//else
	//{
	//	MainView();
	//}
}
