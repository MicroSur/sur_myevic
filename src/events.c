#include "myevic.h"
#include "display.h"
#include "screens.h"
#include "menus.h"
#include "miscs.h"
#include "myrtc.h"
#include "dataflash.h"
#include "atomizer.h"
#include "battery.h"
#include "flappy.h"
#include "tetris.h"
#include "timers.h"
#include "meusbd.h"

#include "events.h"

//=========================================================================

uint8_t		FireClickCount;
uint8_t		FireClickTimer;
int8_t		UserInputs;
int8_t		LastInputs;
uint8_t		FireClicksEvent;

uint8_t		KeyUpTimer;
uint16_t	KeyTicks;
uint16_t	KeyPressTime;

uint8_t		ContrastNum;

//-------------------------------------------------------------------------

//=========================================================================
// Power Curve Editing
//-------------------------------------------------------------------------

__myevic__ int PCGetPoint()
{
	int t = EditItemIndex; //* 5;
	int i;

	for ( i = 0 ; i < PWR_CURVE_PTS ; ++i )
		if ( dfPwrCurve[i].time == t )
			return i;

	return -1;
}

__myevic__ void PCRemovePoint( int i )
{
	if ( i > 0 )
	{
		for ( ; i < PWR_CURVE_PTS ; ++i )
		{
			dfPwrCurve[i].time  = dfPwrCurve[i+1].time;
			dfPwrCurve[i].power = dfPwrCurve[i+1].power;
		}
		--i;
		dfPwrCurve[i].time = 0;
		dfPwrCurve[i].power = 100;
	}
}

__myevic__ int PCAddPoint()
{
	int i;

	int t = EditItemIndex; // * 5;

	if ( t == 0 )
		return 0;

	for ( i = 1 ; i < PWR_CURVE_PTS ; ++i )
		if ( ( dfPwrCurve[i].time == 0 ) || ( dfPwrCurve[i].time > t ) )
			break;

	if ( dfPwrCurve[i-1].time == t )
		return ( i - 1 );

	if ( dfPwrCurve[PWR_CURVE_PTS - 1].time > 0 )
		return -1;

	for ( int j = PWR_CURVE_PTS - 1 ; j > i ; --j )
	{
		dfPwrCurve[j].time  = dfPwrCurve[j-1].time;
		dfPwrCurve[j].power = dfPwrCurve[j-1].power;
	}

	dfPwrCurve[i].time  = t;
	dfPwrCurve[i].power = dfPwrCurve[i-1].power;

	return i;
}

__myevic__ void PCCompact()
{
	for ( int i = PWR_CURVE_PTS - 1 ; i > 0 ; --i )
		if ( dfPwrCurve[i].time )
			if ( dfPwrCurve[i].power == dfPwrCurve[i-1].power )
				PCRemovePoint( i );
}


//=========================================================================
//----- (00003738) --------------------------------------------------------
// Called at 10Hz untill KeyTicks >= 5 (1.4s), then at 100Hz.

__myevic__ void KeyRepeat()
{
	static uint8_t KRDelay = 0;


	if ( !PE0 )
		return;

	if ( ( dfStatus.keylock && !EditModeTimer ) 
                || dfStatus.off )
	{
		if ( !IsMenuScreen() )
			return;
	}

	//if ( dfStatus.off )
	//{
	//	if ( !IsMenuScreen() )
	//		return;
	//}
	//else if ( EditModeTimer && ( Screen == 1 ) )
        if ( !dfStatus.off && EditModeTimer && ( Screen == 1 ) )
	{
		if ( EditItemIndex != 2 )
			return;
	}

	if ( PD2 == PD3 )
	{
		if ( KeyUpTimer )
		{
			if ( !--KeyUpTimer && Screen == 1 && ( dfTCPower >= 1000 || dfMode == 6 ) )
				MainView();
		}
		KeyTicks = 0;
		KRDelay = 6;
	}
	else if ( !KRDelay || !--KRDelay )
	{
		if ( KeyTicks < 205 )
		{
			++KeyTicks;
		}

		// First event has been emitted by GetUserInput()

		// +0.60s
		// Polled every 100ms...
		if ( KeyTicks < 5 )
		{
			KRDelay = 2;
		}
		// then every 10ms.
		// +0.40s (1.00s)
		else if ( KeyTicks < 105 )
		{
			// Quadratic function having its minimum (1) at 104
			KRDelay = 104 - KeyTicks;
			KRDelay = ( KRDelay * KRDelay ) / 1090 + 1; // 545 + 1; 
		}
		// +3.60s (4.60s)
		else if ( KeyTicks < 205 )
		{
			// Step jumped from 1 to 10, ie. speed x10
			// Recover this jump then smooth with same function as above
			KRDelay = 204 - KeyTicks;
			KRDelay = ( KRDelay * KRDelay ) / 1090 + 1; // 545 + 1;
		}

		if ( !PD2 )
		{
			Event = dfStatus2.swap_mp ? 3: 2;
		}
		else if ( !PD3 )
		{
			Event = dfStatus2.swap_mp ? 2: 3;
		}
	}
}

__myevic__ void SetClicksAction( uint8_t num )
{
    //multi fire clicks
    switch ( num )
    //switch ( gFlags.asleep ? dfClick[FireClickCount-3] : dfClick[FireClickCount-2] )
    {
        default:
        case CLICK_ACTION_NONE:                                                    
            break;

	case CLICK_ACTION_EDIT:
            FireClicksEvent = 16;	// edit mode
            break;

	case CLICK_ACTION_TDOM:
            FireClicksEvent = EVENT_TOGGLE_TDOM;	// priority power
            break;

        case CLICK_ACTION_CLOCK:
            FireClicksEvent = EVENT_TOGGLE_CLOCK;	// toggle clock display
            break;

        case CLICK_ACTION_NEXT_MODE:
            FireClicksEvent = EVENT_NEXT_MODE;	// change mode
            break;

        case CLICK_ACTION_ON_OFF:
            if ( !dfStatus.off ) SwitchOffCase = 0; 
            FireClicksEvent = 17;	// Switch On/Off
            break;

        case CLICK_ACTION_PROFILE:
            //sur do not like EVENT_NEXT_PROFILE;
            FireClicksEvent = EVENT_PROFILE_MENU;
            break;
                                                        
        case CLICK_ACTION_TETRIS:
            FireClicksEvent = EVENT_TETRIS; // tetris
            break;
                                                        
        case CLICK_ACTION_GAME:
            FireClicksEvent = 41;	// flappy
            break;
                                                        
        case CLICK_ACTION_SAVER:
            FireClicksEvent = EVENT_SAVER;
            break;       

        case CLICK_ACTION_MENU:
            FireClicksEvent = EVENT_ENTER_MENUS;
            break;
                                                        
        case CLICK_ACTION_BATTERIES:
            FireClicksEvent = 34; //all volts
            break;

        case CLICK_ACTION_REZRESET:
            FireClicksEvent = EVENT_RESET_RES; //CustomEvents()
            break;
                                                        
    }
}

//=========================================================================
//----- (00004B34) --------------------------------------------------------
// Called at 100Hz
__myevic__ void GetUserInput()
{
        //gFlags.apuff    static uint8_t	apuff = 0;
        
	UserInputs = 14;

	if ( ( !PE0 || AutoPuffTimer ) && PD2 && PD3 )
	{
            
            	if (( LastInputs == 5 ) || ( LastInputs == 6 ))
                    return; // f+rb / f+lb
                        
		if ( !PE0 ) //AutoPuffTimer = 0; //fb pressed
                { 
                    if ( dfStatus.autopuff && dfAutoPuffTimer )
                    {
                        if ( FireClickCount > 1 || KeyPressTime > 80 )
                        {
                            AutoPuffTimer = 0;
                            gFlags.apuff = 0;
                        }
                        else if ( KeyPressTime >= 20 && !AutoPuffTimer && Screen == 2 ) //on fire screen...
                        {
                            //AutoPuffTimer = (uint16_t)dfAutoPuffTimer * 10 - FireDuration * 10;
                            AutoPuffTimer = ( (uint16_t)dfAutoPuffTimer - FireDuration ) * 10;
                            gFlags.apuff = 1;
                        }
                    }
                }
                
		UserInputs = 1;  
	}
	else
	{
		if ( gFlags.firing )
		{
			if ( LastInputs == 1 )
			{
                                if ( gFlags.apuff ) // 1 //for correct last FireDuration in TC
                                {
                                    //gFlags.apuff = 0; //in StopFire(); 
                                    FireDuration = (uint16_t)dfAutoPuffTimer;
                                    if ( dfStealthOn != 1 )
                                    {
                                        ShowFireDuration( 0 );
                                        DisplayRefresh();   
                                    }
                                }
                                                            
				StopFire();          //2                  
                                //gFlags.refresh_display = 1; //bad idea // for correct last FireDuration in TC

			}
			gFlags.user_idle = 1;
			LastInputs = -1;
			KeyPressTime = 0;
			return;
		}

		if ( !FireClickTimer && FireClicksEvent )
		{
			Event = FireClicksEvent;
			FireClicksEvent = 0;
		}

		//if ( !dfStatus.off || IsMenuScreen() )
                //! means pressed
		if ( !PD2 && PE0 && PD3 ) 
                {
                    UserInputs = 2;
                }
                else if ( !PD3 && PE0 && PD2 ) 
                {
                    UserInputs = 3;
                }
		else if ( !PD2 && !PD3 && PE0 ) 
                {
                    UserInputs = 4;
                }
		else if ( !PE0 && !PD2 && PD3 ) 
                {
                    UserInputs = 5;
                }
		else if ( !PE0 && !PD3 && PD2 ) 
                {
                    UserInputs = 6;
                }
                else if ( !PE0 && !PD3 && !PD2 )
                {
                    UserInputs = 7;
                }

		if ( USBD_IS_ATTACHED() )
		{
			if ( !gFlags.usb_attached )
			{
				UserInputs = 10; // USB cable attach
				BattProbeCount = 0;

				if ( dfStatus.off && FireClickCount == 1 )
				{
					FireClickCount = 0;
				}
			}
		}
		else
		{
			if ( gFlags.usb_attached )
			{
				UserInputs = 11; // USB cable detach
			}
		}

		if ( gFlags.usb_attached )
		{
			if ( ISVTCDUAL )
			{
				if ( NumBatteries == 1 )
				{
					if ( !PD1 && !gFlags.battery_charging )
					{
						if ( !BattProbeCount || BattProbeCount >= 50 )
						{
							UserInputs = 12;
							BattProbeCount = 0;
						}
					}
					else if ( PD1 && gFlags.battery_charging && BatteryVoltage >= 414 )
					{
						UserInputs = 13;
						BattProbeCount = 1;
					}
				}
			}
			else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 && !ISPRIMO1 
                                && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISRX2 
                                && !ISINVOKE && !ISSINFJ200 && !ISRX217 && !ISGEN2 && !ISIKU200 )
			{
				if ( !PD7 && !gFlags.battery_charging )
				{
					if ( !BattProbeCount || BattProbeCount >= 50 )
					{
						UserInputs = 12; //start hard charge 
						BattProbeCount = 0;
					}
				}
				else if ( PD7 && gFlags.battery_charging )
				{
					UserInputs = 13; //stop hard charge
					BattProbeCount = 1;
				}
			}
		}
	}

	if ( UserInputs >= 14 )
	{
		if ( LastInputs == 1 )
			StopFire();
                
		gFlags.user_idle = 1;
		LastInputs = -1;
		KeyPressTime = 0;
		return;
	}

	if ( UserInputs != LastInputs )
	{
		LastInputs = UserInputs;
		KeyPressTime = 0;
		gFlags.user_idle = 0;

		if ( UserInputs < 10 )
		{
			SplashTimer = 0;
		}

		return;
	}

	++KeyPressTime;

	// A keypress must be stable during at least
	// 60 milliseconds before an event is emitted.
	// (Taking into account the 50ms debounce time)

        uint8_t kpt = IsMenuScreen() ? 5 : 1;
                
        if ( KeyPressTime == kpt ) //6
	{
		gFlags.user_idle = 0;

		if (   UserInputs == 4
			|| UserInputs == 5
			|| UserInputs == 6
			|| UserInputs == 7
                        || UserInputs == 2
                        || UserInputs == 3
		)
			return;

		Event = UserInputs;

		if ( UserInputs == 1 )
		{
			FireClickTimer = 40;
			++FireClickCount;
			FireClicksEvent = 0;
			Event = 0;
                        HideLogo = dfHideLogo;
                        
			// Disable multi-click features in menus
			if ( IsMenuScreen() )
			{
				FireClickCount = 1;
			}

			switch ( FireClickCount )
			{
				case 1:
					FireClicksEvent = 15;	// single click
                                       
					if ( Screen != 1 || !EditModeTimer || ( EditItemIndex != 4 && EditItemIndex != 5 ) )
					{
						Event = 1;	// fire
					}           
                                        
                                       // if (AutoPuffTimer ) Event = EVENT_AUTO_PUFF;
                                                                
					break;

				case 2:
				case 3:
				case 4:
                                case 5:
                                        SetClicksAction( dfClick[FireClickCount-2] );

/*
					if ( dfStatus.off )
					{
                                            if ( FireClickCount == 2 )
                                            {
                                                //SetScreen( 54, 10 );
                                                FireClicksEvent = 34;
                                                //SetClicksAction( CLICK_ACTION_BATTERIES );
                                            }
                                            else if ( FireClicksEvent != 17 )
						{
							FireClicksEvent = 0;
						}
                                        }    
*/
                                        
                                        if ( dfStatus.off )
					{
                                            if ( FireClicksEvent != 17 )
                                            {
                                                if ( FireClickCount == 2 )
                                                {
                                                    //SetScreen( 54, 10 );
                                                    FireClicksEvent = 34;
                                                    //SetClicksAction( CLICK_ACTION_BATTERIES );
                                                }
                                                else
						{
                                                    FireClicksEvent = 0;
						}
                                            }
                                            
                                        }
                                        
					break;

                                default:
                                        if ( !dfStatus.off ) SwitchOffCase = 0; 
                                        FireClicksEvent = 17; //not Event
                                        break;
                                            
				//case 7:
				//	FireClicksEvent = 31;	// board temp screen from on state
				//	break;

				//case 6:                               
				//	FireClicksEvent = 29;	// firmware version screen from on state
				//	break;
                                        
                                //case 7:                                    
				//	FireClicksEvent = 20;	// Info screen
				//	break;
			}
		}
        }
        else if ( KeyPressTime == 6 )
        {
                gFlags.user_idle = 0;
                
		if ( UserInputs == 2 ) //right button
		{
			if ( dfStatus.wakeonpm && Screen == 60 && !dfStatus.off )
                            //!dfStatus.off = pass key while clock on screen in off state
			{
                                ClearScreenBuffer();
                                Event = 0;
                                
				//if ( dfScreenSaver == SSAVER_3D )
				//{
                                //    Next3DObject();
                                //    //Event = 0;
				//}
                                //else 
                                //{
                                    NextSSaver();
                                //}
			} //else if for rotate savers
			else if ( dfStatus.keylock && !EditModeTimer
				&& Screen != 51 && !dfStatus.off //new coil
				&& !IsMenuScreen() 
                                //&& Screen != 5 //charge
                                )
			{
				Event = 30;	// key lock violation
			}
			else
			{
				Event = dfStatus2.swap_mp ? 3: 2;
			}                    
		}
		else if ( UserInputs == 3 ) //left button
		{
			if ( dfStatus.wakeonpm && Screen == 60 )
			{
                                ClearScreenBuffer();
                                Event = 0;
                                
				//if ( dfScreenSaver == SSAVER_3D )
				//{
                                //    Previous3DObject();
                                //    //Event = 0;
				//}
                                //else 
                                //{
                                    PreviousSSaver();
                                //}
			}
			else if ( dfStatus.keylock && !EditModeTimer
				&& Screen != 51 //&& !dfStatus.Off
				&& !IsMenuScreen()
                                && Screen != 5 //charge
                                )
			{
				Event = 30;	// key lock violation
			}
			else
			{
				Event = dfStatus2.swap_mp ? 2: 3;
			}                                              
		}
	}
	else if ( KeyPressTime == 20 )
	{
		if ( UserInputs == 1 )
		{
			FireClicksEvent = 0;
		}
		else if ( UserInputs == 4 )
		{
			// Left + Right button
			if ( IsMenuScreen() )
			{
				Event = EVENT_PARENT_MENU;
			}
		}
		else if ( UserInputs == 5 )
		{
			// Fire + Right button
			if ( IsMenuScreen() )
			{
				Event = EVENT_EXIT_MENUS;
			}
			else if ( !dfStatus.off )
			{
				if ( !gFlags.playing_fb && !gFlags.playing_tt)
				{
					Event = EVENT_ENTER_MENUS;
				}
				else
				{
                                    Event = 0;
                                    if ( gFlags.playing_fb )
                                    {
					gFlags.playing_fb = 0;					
					fbInitTimeouts();
                                    }
                                    else if ( gFlags.playing_tt )
                                    {
					gFlags.playing_tt = 0;
					ttInitTimeouts();
                                    }
                                    MainView();

				}                                
			}
                }
                else if ( UserInputs == 7 ) //all 3 buttons
                {
                    if ( !dfStatus.off && ( Screen == 1 || Screen == 0 ) )  //( !dfStatus.off && !IsMenuScreen() )
                    {
                        SetClicksAction( dfThreeButtonsAct );
                        ScreenDuration = ScrMainTimes[dfScrMainTime]; //GetMainScreenDuration();
                    }
                    else if ( dfThreeButtonsAct == CLICK_ACTION_ON_OFF )
                    {
                        //if ( !dfStatus.off ) SwitchOffCase = 0; 
                        Event = 17; //on
                    }
                }
		
	}
        else if ( KeyPressTime == 100 )
        {
		if ( UserInputs == 5 )
		{
			// Fire + Right button LONG
			if ( dfStatus.off )
			{
				Event = 39;	// tcr set menu
			}
			else
			{
                                gFlags.user_idle = 0;
				Event = EVENT_PROFILE_MENU;	// profile selection
			}
		}
		else if ( UserInputs == 6 )
		{
			if ( dfStatus.off )
			{
				Event = 34;	// battery voltage screen
			}
			else
			{
				Event = 6;	// stealth on/off
			}
		}   
		else if ( UserInputs == 4 ) //left + right
		{
			if ( !EditModeTimer && !IsMenuScreen() ) //!IsMenuScreen() for no key lock in menu
			{
				if ( dfStatus.off )
				{
					Event = 18;	// flip display
				}
				else
				{
					Event = 4;	// key (un)lock
				}
			}
		}                
        }
	else if ( KeyPressTime == 200 )
	{
		if ( UserInputs == 1 ){
                    if (( Screen == 1 ) && ( EditModeTimer > 0 ))
                    {
                        uint8_t a;
                        if ( EditItemIndex == 4 || EditItemIndex == 5 ) //4 = 3-d info line
                        {
                            if ( EditItemIndex == 4 ) a = dfAPT3;
                            else a = dfAPT;
                        
                            EditModeTimer = 1000;
                            switch ( a )
                            {
                                    case 1:
                                        Event = 22;	// puff reset
                                        break;
                                    case 2:
                                        Event = 23;	// time reset
                                        break;                                        
                                    case 3:
                                        Event = EVENT_RESET_VAPED;
                                        break;
                                    case 5:
                                        Event = EVENT_RESET_JOULES;
                                        break;                                        
                                    case 4: //vv day
                                        MilliJoulesDay = 0;
                                        break;
                                    case 15: //stopwatch
                                        RTCGetEpoch( &startwatch );
                                        break;      
                            }
                        }
                    }
                    else
                    {
				Event = EVENT_LONG_FIRE;
                    }
                    
		}
                else if ( UserInputs == 5 ) // Fire + Right button LONG
		{
			Event = EVENT_POWER_CURVE;
		}
/*
		else if ( UserInputs == 4 )
		{
			if ( !EditModeTimer )
			{
				if ( dfStatus.off )
				{
					Event = 18;	// flip display
				}
				else
				{
					Event = 4;	// key (un)lock
				}
			}
		}
*/
/*
		else if ( UserInputs == 5 )
		{
			// Fire + Right button LONG
			if ( dfStatus.off )
			{
				Event = 39;	// tcr set menu
			}
			else
			{
				Event = EVENT_PROFILE_MENU;	// profile selection
			}
		}
		else if ( UserInputs == 6 )
		{
			if ( dfStatus.off )
			{
				Event = 34;	// battery voltage screen
			}
			else
			{
				Event = 6;	// stealth on/off
			}
		}
*/
	}
/*
	else if ( KeyPressTime == 300 )
	{
		if ( UserInputs == 5 ) // Fire + Right button very LONG
		{
			Event = EVENT_POWER_CURVE;
		}
	}
*/
	else if ( ( KeyPressTime & 0x8000 ) || ( KeyPressTime & 0x7fff ) > 100 ) //200
	{
		if ( UserInputs == 1 )
		{
                        uint16_t pt;
                        pt = dfProtec * 10; // in real +3 sec then before mod off
                        
			if ( KeyPressTime >= pt ) //FIRE_PROTEC_MAX * 10 + 100 )
			{
				//KeyPressTime = pt; //dfProtec * 10 + 50; //FIRE_PROTEC_MAX * 10 + 100;
				gFlags.user_idle = 1; //to switch mod Off
			}
			else if ( ( gFlags.firing || Screen == 23 ) && FireDuration >= dfProtec )
			{
                            if ( !( gFlags.apuff && dfStatus.endlessfire ) )
				Event = 24;	// fire protection, show screen 23_1
			}
		}
		else if ( KeyPressTime & 0x8000 )
		{
			KeyPressTime = 1;
		}
	}
}


//=========================================================================
// Event Handling
//-------------------------------------------------------------------------

__myevic__ int EvtFire()
{
	int vret = 0;
                    
	switch ( Screen )
	{
		case 101:
                        ContrastNum ^= 1;
                        //if ( ContrastNum ) DisplaySetContrast( dfContrast2 );
                        //else DisplaySetContrast( dfContrast );
                        gFlags.refresh_display = 1;
                        ScreenDuration = 10;
			vret = 1;
                        break;

		case 102:    
		{
			vret = MenuEvent( LastEvent );
		}
		break;

		case 103:
		{
			MainView();
			vret = 1;
		}
		break;

		case 104:
		{
			RTCAdjustClock( 0 );
			MainView();
			vret = 1;
		}
		break;

		case 105:
		//case 106:
		{
			EditModeTimer = 6000;
			if ( --EditItemIndex > 5 ) EditItemIndex = 5; //2
			gFlags.draw_edited_item = 1;
			vret = 1;
		}
		break;
		
		case 107:
		{
			EditModeTimer = 3000;
			if ( gFlags.edit_value )
			{
				gFlags.edit_value = 0;
				PCCompact();
			}
			else
			{
				if (( PCGetPoint() >= 0 ) || ( PCAddPoint() >= 0 ))
				{
					gFlags.edit_value = 1;
				}
			}
			vret = 1;
		}
                break;
                
                case EVENT_SET_JOULES: //scr = event
                {    
                	EditModeTimer = 0;
			//MainView();
                        //CurrentMenuItem = 6;
                        Event = EVENT_PARENT_MENU;
                        vret = 1;
                }    
	}

	return vret;
}

//-------------------------------------------------------------------------

__myevic__ int EvtSingleFire()
{
	int vret = 0;

	switch ( Screen )
	{
		case 100:
		{
			MainView();
			vret = 1;
		}
		break;

		case 101:
		case 103:
		case 104:
		case 105:
		//case 106:
		case 107:
                case 50:     // FW Version
                case EVENT_SET_JOULES:    
		{
			vret = 1; // need prevents immediate call of mainview
		}
		break;

		case 102:
		{
			vret = MenuEvent( LastEvent );
		}
		break;
	}

	return vret;
}

//-------------------------------------------------------------------------

__myevic__ int EvtPlusButton()
{
	int vret = 0;

	switch ( Screen )
	{
		case 1:
		{
			if ( EditModeTimer )
			{
				if ( EditItemIndex == 1 )
				{
					if ( dfMode < 3 )
					{
						KeyUpTimer = 10;
						EditModeTimer = 1000;

						do
						{
							if ( ++dfMode > 2 ) dfMode = 0;
						}
						while ( dfModesSel & ( 1 << dfMode ) );
						dfTCMode = dfMode;
						ModeChange();

						UpdateDFTimer = 50;
						//gFlags.refresh_display = 1;
						vret = 1;
					}
				}
			}
		}
		break;

		case 101:
		{
                    if ( ContrastNum )
                    {
			if ( dfContrast2 <= 250 ) dfContrast2 += 5;
			else dfContrast2 = 255;
                        //DisplaySetContrast( dfContrast2 );                       
                    }
                    else
                    {
			if ( dfContrast <= 250 ) dfContrast += 5;
			else dfContrast = 255;
                        DisplaySetContrast( dfContrast );
                        //gFlags.MainContrast = 1;
                    }
			UpdateDFTimer = 50;
			//gFlags.refresh_display = 1;
			ScreenDuration = 10;
			vret = 1;
		}
		break;

		case 102:
		{
			vret = MenuEvent( LastEvent );
		}
		break;

		case 103:
		{
			if ( !gFlags.has_x32 )
			{
				unsigned int cs = RTCGetClockSpeed();
				if ( KeyTicks < 105 )
				{
					++cs;
				}
				else
				{
					cs -= cs % 10;
					cs += 10;
				}
				if ( cs > RTC_MAX_CLOCK_RATIO )
				{
					if ( KeyTicks < 5 )
					{
						cs = RTC_MIN_CLOCK_RATIO;
					}
					else
					{
						cs = RTC_MAX_CLOCK_RATIO;
					}
				}
				RTCSetClockSpeed( cs );
			}
			//gFlags.refresh_display = 1;
			ScreenDuration = 120;
			vret = 1;
		}
		break;
		
		case 104:
		{
			RTCAdjustClock( 1 );
			//gFlags.refresh_display = 1;
			ScreenDuration = 120;
			vret = 1;
		}
		break;

		case 105:
		{
                    if ( EditItemIndex > 2 )
                    {
			switch ( EditItemIndex - 3 )
			{
				case 0:
					++SetTimeRTD.u32Second;
					SetTimeRTD.u32Second %= 60;
					break;
				case 1:
					++SetTimeRTD.u32Minute;
					SetTimeRTD.u32Minute %= 60;
					break;
				case 2:
					++SetTimeRTD.u32Hour;
					SetTimeRTD.u32Hour %= 24;
					break;
			}
                    }
                    else
                    {
                        int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			switch ( ( f << 2 | EditItemIndex ) )
			{
				case  0:
				case  4:
				case  8:
				case 14:
					if ( SetTimeRTD.u32Year < RTC_YEAR2000 + 1000 ) ++SetTimeRTD.u32Year;
					break;
				case  1:
				case  6:
				case  9:
				case 13:
					SetTimeRTD.u32Month = SetTimeRTD.u32Month %12 + 1;
					break;
				case  2:
				case  5:
				case 10:
				case 12:
					SetTimeRTD.u32Day = SetTimeRTD.u32Day %31 + 1;
					break;
			}
                    }
                    
			gFlags.draw_edited_item = 1;
			//gFlags.refresh_display = 1;
			ScreenDuration = 60;
			EditModeTimer = 6000;
			vret = 1;
		}
		break;

/*
		case 106:
		{
			int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			switch ( ( f << 2 | EditItemIndex ) )
			{
				case  0:
				case  4:
				case  8:
				case 14:
					if ( SetTimeRTD.u32Year < RTC_YEAR2000 + 1000 ) ++SetTimeRTD.u32Year;
					break;
				case  1:
				case  6:
				case  9:
				case 13:
					SetTimeRTD.u32Month = SetTimeRTD.u32Month %12 + 1;
					break;
				case  2:
				case  5:
				case 10:
				case 12:
					SetTimeRTD.u32Day = SetTimeRTD.u32Day %31 + 1;
					break;
			}
			gFlags.draw_edited_item = 1;
			gFlags.refresh_display = 1;
			ScreenDuration = 60;
			EditModeTimer = 6000;
			vret = 1;
		}
		break;
*/

		case 107:
		{
			if ( gFlags.edit_value )
			{
				int i = PCGetPoint();

				if ( ++dfPwrCurve[i].power > 200 )
				{
					if ( KeyTicks < 5 ) dfPwrCurve[i].power = 0;
					else dfPwrCurve[i].power = 200;
				}
                                UpdateDFTimer = 50;
			}
			else
			{                           
				++EditItemIndex;
				EditItemIndex %= 50;
			}
			EditModeTimer = 3000;
			//gFlags.refresh_display = 1;
			vret = 1;	
		}
                break;
                
                case EVENT_SET_JOULES: // scr = event
		{                    
                    	if ( ++dfVVRatio > VVEL_MAX_RATIO )
			{
				if ( KeyTicks < 5 ) dfVVRatio = VVEL_MIN_RATIO;
				else dfVVRatio = VVEL_MAX_RATIO;
			}
                        UpdateDFTimer = 50;
                        //EditModeTimer = 3000;
                        ScreenDuration = 60;
			//gFlags.refresh_display = 1;
			vret = 1;
		}
                break;
	}
        
        gFlags.refresh_display = 1;
	return vret;
}

//-------------------------------------------------------------------------

__myevic__ int EvtMinusButton()
{
	int vret = 0;
        
	switch ( Screen )
	{
		case 101:
		{
                    if ( ContrastNum )
                    {
			if ( dfContrast2 >= 5 ) dfContrast2 -= 5;
			else dfContrast2 = 0;
                        //DisplaySetContrast( dfContrast2 );                       
                    }
                    else
                    {
			if ( dfContrast >= 5 ) dfContrast -= 5;
			else dfContrast = 0;                       
                        DisplaySetContrast( dfContrast );
                        //gFlags.MainContrast = 1;
                    }
			UpdateDFTimer = 50;			
			//gFlags.refresh_display = 1;
			ScreenDuration = 10;
			vret = 1;
		}
		break;

		case 102:
		{
			vret = MenuEvent( LastEvent );
		}
		break;

		case 103:
		{
			if ( !gFlags.has_x32 )
			{
				unsigned int cs = RTCGetClockSpeed();
				if ( KeyTicks < 105 )
				{
					--cs;
				}
				else
				{
					if ( cs % 10 ) cs -= cs % 10;
					else cs -= 10;
				}
				if ( cs < RTC_MIN_CLOCK_RATIO )
				{
					if ( KeyTicks < 5 )
					{
						cs = RTC_MAX_CLOCK_RATIO;
					}
					else
					{
						cs = RTC_MIN_CLOCK_RATIO;
					}
				}
				RTCSetClockSpeed( cs );
			}
			//gFlags.refresh_display = 1;
			ScreenDuration = 120;
			vret = 1;
		}
		break;

		case 104:
		{
			RTCAdjustClock( -1 );
			//gFlags.refresh_display = 1;
			ScreenDuration = 120;
			vret = 1;
		}
		break;

		case 105:
		{
                    if ( EditItemIndex > 2 )
                    {
			switch ( EditItemIndex - 3 )
			{
				case 0:
					SetTimeRTD.u32Second = ( SetTimeRTD.u32Second + 59 ) % 60;
					break;
				case 1:
					SetTimeRTD.u32Minute = ( SetTimeRTD.u32Minute + 59 ) % 60;
					break;
				case 2:
					SetTimeRTD.u32Hour = ( SetTimeRTD.u32Hour + 23 ) % 24;
					break;
			}
                    }
                    else //if ( EditItemIndex <= 2 )
                    {
                        int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			switch ( ( f << 2 | EditItemIndex ) )
			{
				case  0:
				case  4:
				case  8:
				case 14:
					if ( SetTimeRTD.u32Year > RTC_YEAR2000 ) --SetTimeRTD.u32Year;
					break;
				case  1:
				case  6:
				case  9:
				case 13:
					SetTimeRTD.u32Month = ( SetTimeRTD.u32Month + 10 ) % 12 + 1;
					break;
				case  2:
				case  5:
				case 10:
				case 12:
					SetTimeRTD.u32Day = ( SetTimeRTD.u32Day + 29 ) % 31 + 1;
					break;
			}
                    }
                        
			gFlags.draw_edited_item = 1;
			//gFlags.refresh_display = 1;
			ScreenDuration = 60;
			EditModeTimer = 6000;
			vret = 1;
		}
		break;

/*
		case 106:
		{
			int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			switch ( ( f << 2 | EditItemIndex ) )
			{
				case  0:
				case  4:
				case  8:
				case 14:
					if ( SetTimeRTD.u32Year > RTC_YEAR2000 ) --SetTimeRTD.u32Year;
					break;
				case  1:
				case  6:
				case  9:
				case 13:
					SetTimeRTD.u32Month = ( SetTimeRTD.u32Month + 10 ) % 12 + 1;
					break;
				case  2:
				case  5:
				case 10:
				case 12:
					SetTimeRTD.u32Day = ( SetTimeRTD.u32Day + 29 ) % 31 + 1;
					break;
			}
			gFlags.draw_edited_item = 1;
			gFlags.refresh_display = 1;
			ScreenDuration = 60;
			EditModeTimer = 6000;
			vret = 1;
		}
		break;
*/

		case 107:
		{
			if ( gFlags.edit_value )
			{
				int i = PCGetPoint();

				if ( !dfPwrCurve[i].power-- )
				{
					if ( KeyTicks < 5 ) dfPwrCurve[i].power = 200;
					else dfPwrCurve[i].power = 0;
				}
                                UpdateDFTimer = 50;
			}
			else
			{
				if ( !EditItemIndex-- ) EditItemIndex = 49;
			}
			EditModeTimer = 3000;
			//gFlags.refresh_display = 1;
			vret = 1;
		}
                break;
                
                case EVENT_SET_JOULES: //scr = event
		{                    
                        if ( --dfVVRatio < VVEL_MIN_RATIO  )
                        {
				if ( KeyTicks < 5 ) dfVVRatio = VVEL_MAX_RATIO;
				else dfVVRatio = VVEL_MIN_RATIO;
                        }
                        UpdateDFTimer = 50;
                        //EditModeTimer = 3000;
                        ScreenDuration = 60;
			//gFlags.refresh_display = 1;
			vret = 1;
		}
                break;                
	}

        gFlags.refresh_display = 1;
	return vret;
}

//-------------------------------------------------------------------------

/*
__myevic__ int EvtToggleClock()
{

        HideLogo = 0;
	dfStatus.clock ^= 1;
        dfStatus2.anim3d = 0;
        ShowLogo( 0 );
	UpdateDFTimer = 50;

        
        gFlags.toggleclock ^= 1;
        //gFlags.refresh_display;
                
	return 1;
}
*/

__myevic__ int EvtLongFire()
{
	int vret = 0;

	switch ( Screen )
	{
            	case 101: //contrast
		{
			//UpdateDataFlash();
			Event = EVENT_PARENT_MENU;
			vret = 1;
		}
		break;
                
		case 102: //menus
			vret = MenuEvent( LastEvent );
			break;

                        //bug on mod wis no quartz 2020 01 01 saved as 2020 01 02 (yy mm dd)
                case 105: //SetTimeDate
		//case 106: //SetDate
		{
			//S_RTC_TIME_DATA_T rtd;

			//GetRTC( &rtd );
			//SetTimeRTD.u32Hour = rtd.u32Hour;
			//SetTimeRTD.u32Minute = rtd.u32Minute;
			//SetTimeRTD.u32Second = rtd.u32Second;
			// NOBREAK
                    
      			SetRTC( &SetTimeRTD );
			EditModeTimer = 0;
                        Event = EVENT_PARENT_MENU;
			vret = 1;
			break;
		}
/*
		case 105: //SetTime
			SetRTC( &SetTimeRTD );
			EditModeTimer = 0;
			//MainView();
                        Event = EVENT_PARENT_MENU;
			vret = 1;
			break;
*/

		case 107:
		{
			int i = PCGetPoint();
			if ( i > 0 )
				PCRemovePoint( i );
			gFlags.edit_value = 0;
			gFlags.refresh_display = 1;
			vret = 1;
                        break;
		}
                
/*
		case EVENT_SET_JOULES:
                        dfVVRatio = VVEL_DEF_RATIO;  
                        //        gFlags.edit_value = 0;
			//EditModeTimer = 0;
			//MainView();
                        //Event = EVENT_PARENT_MENU;
			vret = 1;
			break;                
*/
	}

	return vret;
}

//-------------------------------------------------------------------------

__myevic__ int EvtContrastMenu()
{
        ContrastNum = 0;
       // DisplaySetContrast( dfContrast );
       // gFlags.refresh_display = 1;
	SetScreen( 101, 10 );
	return 1;
}

//-------------------------------------------------------------------------

__myevic__ int EvtEnterMenus()
{
        CurrentMenu = 0;
	CurrentMenuItem = 0;
	SetScreen( 102, 15 );
	return 1;
}

//-------------------------------------------------------------------------

S_RTC_TIME_DATA_T SetTimeRTD;

/*
__myevic__ int EvtSet( int what )
{
    	switch ( what )
	{
            	case 0: //EvtSetTime
                    case 1: //EvtSetDate
                        case 2: //EvtSetJoules
		{
	GetRTC( &SetTimeRTD );
	SetScreen( 105, 60 );
	EditItemIndex = 2;
	EditModeTimer = 6000;
	return 1;
}
*/

__myevic__ int EvtSetTimeDate()
{
	GetRTC( &SetTimeRTD );
	SetScreen( 105, 60 );
	EditItemIndex = 5; //2
	EditModeTimer = 6000;
	return 1;
}

/*
__myevic__ int EvtSetDate()
{
	GetRTC( &SetTimeRTD );
	SetScreen( 106, 60 );
	EditItemIndex = 2;
	EditModeTimer = 6000;
	return 1;
}
*/

__myevic__ int EvtSetJoules()
{
	SetScreen( EVENT_SET_JOULES, 60 ); //scr 123
	//EditItemIndex = 0;
	//EditModeTimer = 6000;
	return 1;
}

__myevic__ void ResetVapedCounter()
{
    MilliJoules = 0;
    dfJoules = 0;
    MilliJoulesVapedOn = 0;
    UpdateDFTimer = 50;
}

__myevic__ void ResetJoulesCounter()
{
    MilliJoulesEnergy = 0;
    dfJoulesEnergy = 0;
    UpdateDFTimer = 50;
}

__myevic__ void ResetAllCounters()
{
    ResetVapedCounter();
    ResetPuffCounters();
    ResetJoulesCounter();
}
__myevic__ void ResetPuffCounters()
{
    dfTimeCount = 0;
    dfPuffCount = 0;
    SessionPuffs = 0;
    UpdatePTTimer = 80;
    UpdateDFTimer = 50;
}

//-------------------------------------------------------------------------

__myevic__ int CustomEvents()
{
	int vret;

	vret = 1;

	switch ( LastEvent )
	{
		case   1:	// Fire button                           
			vret = EvtFire();
			break;

		case   2:	// + button
			vret = EvtPlusButton();
			break;

		case   3:	// - button
			vret = EvtMinusButton();
			break;

		case  15:	// Single Fire
			vret = EvtSingleFire();
			break;

		case  39:	// TCR Set Menu select
			vret = MenuEvent( LastEvent );
			break;

		case EVENT_TOGGLE_CLOCK:	// Double Fire
			//vret = EvtToggleClock();
                        gFlags.toggleclock ^= 1;
                        MainView(); //fast change
			break;

		case EVENT_EDIT_CONTRAST:	// Contrast screen
			vret = EvtContrastMenu();
			break;
                        
		case EVENT_RESET_RES:
                        if ( AtoStatus == 4 ) //ok res
                        {
                            ResetResistance();
                            SwitchRezLock( 1 ); //reread TC rez
                        }
			break;
                        
		case EVENT_ENTER_MENUS:	// Menus screen
                        EditModeTimer = 0; //prevent exit menu by this timeout
                        gFlags.MainContrast = 1;
                        DisplaySetContrast( dfContrast );
			vret = EvtEnterMenus();
			break;

		//case EVENT_DEBUG_MODE:
		//	vret = EvtDebugMode();
		//	break;

		case EVENT_LONG_FIRE:
			vret = EvtLongFire();
			break;

		case EVENT_EXIT_MENUS:
			vret = MenuEvent( LastEvent );
			break;

		case EVENT_PARENT_MENU:
			vret = MenuEvent( LastEvent );
			break;

		case EVENT_SET_TIME_DATE:
			vret = EvtSetTimeDate();
			break;
                        
		case EVENT_SET_JOULES:
			vret = EvtSetJoules();
			break;
                        
		//case EVENT_SET_DATE:
		//	vret = EvtSetDate();
		//	break;

		case EVENT_NEXT_MODE:
			NextMode();
			gFlags.refresh_display = 1;
			break;

		case EVENT_TOGGLE_TDOM:
			if ( ISMODETC(dfMode) )
			{
				dfStatus.priopwr ^= 1;
				UpdateDFTimer = 50;
				gFlags.refresh_display = 1;
			}
			else if ( ISMODEVW(dfMode) )
			{
				MenuEvent( LastEvent );
			}
			break;

		case EVENT_RESET_VAPED:
		        ResetVapedCounter();
                        //EditModeTimer = 1000;
			//gFlags.refresh_display = 1;
			//gFlags.draw_edited_item = 1; 
                        SetEditTimer();
                        break;
                        
                case EVENT_RESET_JOULES:
                        ResetJoulesCounter();
                        //EditModeTimer = 1000;
			//gFlags.refresh_display = 1;
			//gFlags.draw_edited_item = 1;
                        SetEditTimer();
			break;

/*
		case EVENT_FORCE_VCOM:
			//dfStatus.storage = 0;
			dfStatus.vcom = 1;
			InitUSB();
			break;
*/

		case EVENT_AUTO_PUFF:
			//if ( AutoPuffTimer > 0 )
                        //{
			////	MainView();
                        //}
			//else
			if ( !AutoPuffTimer )
                            StopFire();
			break;

		case EVENT_CLK_ADJUST:
			SetScreen( 104, 120 );
			break;

		case EVENT_CLK_SPEED:
			SetScreen( 103, 120 );
			break;

		case EVENT_INVERT_SCREEN:
			DisplaySetInverse( dfStatus.invert );
			break;

		case EVENT_MODE_CHANGE:
			ModeChange();
			break;

		case EVENT_PROFILE_MENU:
			vret = MenuEvent( LastEvent );
			break;

//		case EVENT_NEXT_PROFILE:
//			LoadProfile( ( dfProfile + 1 ) % DATAFLASH_PROFILES_MAX );
//			ShowProfNum = 30;
//			break;

		case EVENT_POWER_CURVE:
			SetScreen( 107, 30 );
			EditModeTimer = 3000;
			EditItemIndex = 0;
			break;
                        
/*
		case 40: //was logo menu select
			SetScreen( 107, 15 );
			EditModeTimer = 3000;
			EditItemIndex = 0;
			break;
*/
                        
                case 41:
			//fbStartGame();
                        CurrentMenu = &GameMenu; 
                        CurrentMenuItem = FBSpeed;
                        SetScreen( 102, 15 );                        
			break;
                        
                case EVENT_TETRIS:
 			//ttStartGame();
                        CurrentMenu = &GameTtMenu; 
                        CurrentMenuItem = dfTTSpeed;
                        SetScreen( 102, 15 );
			break;
                        
                case EVENT_SAVER:
                        gFlags.animready = 0;
                        SetScreen( 60, GetScreenProtection() );
			//Screen = 60;
			//ScreenDuration = GetScreenProtection();
                        //gFlags.refresh_display = 1;
			break;   
                                               
		default:
			vret = 0;
			break;
	}

	return vret;
}

//==========================================================================

