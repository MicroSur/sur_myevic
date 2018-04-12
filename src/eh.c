#include "myevic.h"
#include "screens.h"
#include "atomizer.h"
#include "display.h"
#include "battery.h"
#include "miscs.h"
#include "events.h"
#include "dataflash.h"
#include "timers.h"
#include "meadc.h"
#include "atomizer.h"
#include "flappy.h"
#include "tetris.h"

//=========================================================================
// Globals

volatile uint8_t Event;
uint8_t	LastEvent;

uint8_t	WattsInc;

//-------------------------------------------------------------------------

uint16_t	NewRez;
uint8_t		NewMillis;

//=========================================================================

__myevic__ void PowerPlus( uint16_t *pwr, uint16_t min, uint16_t max )
{
	max = max - ( max % WattsInc );

	if ( *pwr >= max )
	{
		if ( KeyTicks < 5 )
		{
			*pwr = min;
		}
	}
	else if ( KeyTicks < 105 )
	{
		if ( *pwr < 1000 )
		{
			*pwr += WattsInc;
		}
		else
		{
			*pwr += 10;
		}

		if ( *pwr > max )
		{
			*pwr = max;
		}
	}
	else
	{
		if ( *pwr % 10 )
		{
			*pwr -= *pwr % 10;
		}
		*pwr += 10;
	}
}


//-------------------------------------------------------------------------

__myevic__ void PowerMinus( uint16_t *pwr, uint16_t min, uint16_t max )
{
	if ( *pwr <= min )
	{
		if ( KeyTicks < 5 )
		{
			*pwr = max - ( max % WattsInc );
		}
	}
	else if ( KeyTicks < 105 )
	{
		if ( *pwr <= 1000 )
		{
			*pwr -= WattsInc;
		}
		else
		{
			*pwr -= 10;
		}

		if ( *pwr < min )
		{
			*pwr = min;
		}
	}
	else
	{
		if ( *pwr % 10 )
		{
			*pwr -= *pwr % 10;
		}
		else
		{
			*pwr -= 10;
		}
	}
}

//-------------------------------------------------------------------------

__myevic__ void SmartPowerPM( int plus )
{
        unsigned int spwr;
    	spwr = dfSavedCfgPwr[ConfigIndex];
	spwr -= spwr % WattsInc; //10;
        
        if ( plus )
        {
            spwr += WattsInc;
            if ( spwr > AtoMaxPower ) spwr = AtoMaxPower;
	
        }
        else
        {
            spwr -= WattsInc; //10;
            if ( spwr < AtoMinPower ) spwr = AtoMinPower;
        }
        
	dfSavedCfgPwr[ConfigIndex] = spwr;
	VWVolts = GetAtoVWVolts( spwr, AtoRez );
        
}

//-------------------------------------------------------------------------

__myevic__ void TempPlus()
{
	if ( dfStatus.IsCelsius )
	{
		dfTemp += dfStatus.onedegree ? 1 : 5;
                        //if ( !dfStatus.onedegree )
                        // {
			//dfTemp -= dfTemp % 5;
                        //}
                                
		if ( dfTemp > 995 ) //315
		{
			//dfTemp = ( KeyTicks < 5 ) ? 100 : 315;
                        dfTemp = ( KeyTicks < 5 ) ? 5 : 995;
		}
	}
	else
	{
		dfTemp += dfStatus.onedegree ? 5 : 10;
		if ( dfTemp > 995 ) //600
		{
			//dfTemp = ( KeyTicks < 5 ) ? 200 : 600;
                        dfTemp = ( KeyTicks < 5 ) ? 35 : 995;
		}
	}
}


//-------------------------------------------------------------------------

__myevic__ void TempMinus()
{
	if ( dfStatus.IsCelsius )
	{
		dfTemp -= dfStatus.onedegree ? 1 : 5;
		if ( dfTemp < 5 ) //100
		{
			//dfTemp = ( KeyTicks < 5 ) ? 315 : 100;
                        dfTemp = ( KeyTicks < 5 ) ? 995 : 5;
		}
	}
	else
	{
		dfTemp -= dfStatus.onedegree ? 5 : 10;
		if ( dfTemp < 35 ) //200
		{
			//dfTemp = ( KeyTicks < 5 ) ? 600 : 200;
                        dfTemp = ( KeyTicks < 5 ) ? 995 : 35;
		}
	}
}


//----- (000039E0) --------------------------------------------------------
__myevic__ void EventHandler()
{
	unsigned int pwr;
	//unsigned int spwr;

	unsigned int v21;
	int v22;
	int v23;
	int tempf;
        
	if ( Event == 0 )
		return;

//	myprintf( "Event = %d\n", Event );

	NoEventTimer = 200;
	LastEvent = Event;
	Event = 0;

	if ( CustomEvents() )
		return;
        
	switch ( LastEvent )
	{

		case 1:		// Fire
		{
			if ( dfStatus.off )
			{
                            if ( Screen == 60 || Screen == 54 ) // set clock off, battery
                            {
                                Screen = 0;
				SleepTimer = 0;
                                gFlags.refresh_display = 1;
                            }
                            else if( !gFlags.battery_charging && !dfStatus.offmodclock && Screen == 0 )
                            {
                                SetScreen( 60, 10 );
                            }                           
                        return;                       
			}

			if ( Screen == 1 )
			{
				if ( EditModeTimer )
				{
					EditModeTimer = 0;
					gFlags.edit_capture_evt = 0;
					gFlags.draw_edited_item = 1;
					UpdateDFTimer = 50;
				}
			}

			if ( gFlags.firing )
			{                   
				KeyPressTime |= 0x8000;
				return;
			}
                                               
			if ( BatteryStatus == 2 )
			{
                                SetScreen( 56, 2 );
				//Screen = 56; // Check Battery
				//ScreenDuration = 2;
				//gFlags.refresh_display = 1;
				return;
			}

			if ( BatteryStatus == 3 )
			{
                                SetScreen( 57, 2 );
				//Screen = 57; // Check USB Adapter
				//ScreenDuration = 2;
				//gFlags.refresh_display = 1;
				return;
			}

			if ( BatteryStatus == 4 )
			{
                                SetScreen( 58, 2 );
				//Screen = 58; // Charge Error
				//ScreenDuration = 2;
				//gFlags.refresh_display = 1;
				return;
			}

			if ( gFlags.low_battery )
			{
				if ( BatteryVoltage < BatteryCutOff + 20) //+ 50 )
				{
                                        SetScreen( 25, 2 );
					//gFlags.refresh_display = 1;
					//Screen = 25; //Battery Low Lock
					//ScreenDuration = 2;
					KeyPressTime |= 0x8000;
					return;
				}
				else
				{
					gFlags.low_battery = 0;
				}
			}

			if ( BatteryVoltage <= BatteryCutOff ) //+ 30 )
			{
				Event = 28;
				gFlags.low_battery = 1;
				return;
			}

			if ( ( BoardTemp > dfMaxBoardTemp ) 
                        || ( ISSINFJ200 && ( AkkuTemp > 70 ) ) )
			{
				Overtemp();
				return;
			}

			while ( AtoStatus == 4 && AtoProbeCount < 12 )
			{
				ProbeAtomizer();
                                WaitOnTMR2( 10 );
			}

			switch ( AtoStatus )
			{
				case 1:
					Event = 25; //short
					break;
				case 2:
					Event = 27;
					break;
				case 0:
				case 3:
					Event = 26;
					break;
                                case 5:
                                	Event = 70; //overcurrent
					break;
				case 4:
				default:
					break;
			}

			if ( AtoError )
                        {
                            return;
                        }
				

			if ( Set_NewRez_dfRez == 1 ) //from: init, Ato... <> lastAto...
			{
				Set_NewRez_dfRez = 0;
				NewRez = AtoRez;
				NewMillis = AtoMillis;

                                int lock = GetLockState();
				//uint8_t lock = 0;
				//if 	( dfMode == 0 ) lock = dfRezLockedNI;
				//else if ( dfMode == 1 ) lock = dfRezLockedTI;
				//else if ( dfMode == 2 ) lock = dfRezLockedSS;
				//else if ( dfMode == 3 ) lock = dfRezLockedTCR;

                                //#define ISMODEVW(m) (((m)==4)||((m)==6))
                                //#define ISMODEBY(m) ((m)==5)
                                
                                //if ( !lock ) //|| dfMode == 4 || dfMode == 5 || dfMode == 6 )      
                                if ( !lock ) //&& !ISMODETC(dfMode) )    
				{
					dfResistance = AtoRez;
					RezMillis = AtoMillis;
					UpdateDFTimer = 50;
				}
			}

//------------------------------------------------------------------------------

			if ( gFlags.new_rez_ni && dfMode == 0 )
			{
				gFlags.new_rez_ni = 0;

				if ( !dfRezNI )
				{
					dfRezNI = dfResistance;
					dfMillis = ( dfMillis & ~0xf ) | RezMillis;
				}
				else if ( !dfRezLockedNI )
				{
					word_200000BA = dfRezNI * 10 + ( dfMillis & 0xf );

					if (  3 * dfRezNI >= NewRez )
					{
						if (	dfRezNI + dfRezNI * dfNewRezPerc / 100 < NewRez // dfRezNI / 20
							&&	dfRezNI + 1 < NewRez
							//&&	!dfRezLockedNI 
                                                        )
						{
							gFlags.new_rez_ni = 1;
							Event = 32;
							return;
						}
                                                    
						if
						(
							(	(dfRezNI - dfRezNI * dfNewRezPerc / 100 <= NewRez || dfRezNI - 1 <= NewRez) // dfRezNI / 20
							&&	(dfRezNI + dfRezNI * dfNewRezPerc / 100 >= NewRez || dfRezNI + 1 >= NewRez)) // dfRezNI / 20
						//||
						//	(	dfRezLockedNI
						//	&&	(dfRezNI - dfRezNI / 10 <= NewRez || dfRezNI - 4 <= NewRez))
						)
						{
							dfResistance = dfRezNI;
							RezMillis = dfMillis & 0xf;
						}
						else
						{
							if ( dfRezNI - dfRezNI * dfNewRezPerc / 100 > NewRez && dfRezNI - 1 > NewRez ) // dfRezNI / 20
							{
								dfResistance = NewRez;
								RezMillis = NewMillis;

								dfRezNI = NewRez;
								dfMillis = ( dfMillis & ~0xf ) | NewMillis;
							}
						}
					}
					else
					{
						dfResistance = NewRez;
						RezMillis = NewMillis;

						dfRezNI = NewRez;
						dfMillis = ( dfMillis & ~0xf ) | NewMillis;
					}
				}
			}

			if ( gFlags.new_rez_ti && dfMode == 1 )
			{
				gFlags.new_rez_ti = 0;

				if ( !dfRezTI )
				{
					dfRezTI = dfResistance;
					dfMillis = ( dfMillis & ~0xf0 ) | ( RezMillis << 4 );
				}
				else if ( !dfRezLockedTI )
				{
					word_200000B8 = dfRezTI * 10 + ( ( dfMillis >> 4 ) & 0xf );

					if (  2 * dfRezTI >= NewRez )
					{
						if (	dfRezTI + dfRezTI * dfNewRezPerc / 100 < NewRez
							&&	dfRezTI + 1 < NewRez
							//&&	!dfRezLockedTI 
                                                        )
						{
							gFlags.new_rez_ti = 1;
							Event = 32;
							return;
						}

						if
						(
							(	(dfRezTI - dfRezTI * dfNewRezPerc / 100 <= NewRez || dfRezTI - 1 <= NewRez)
							&&	(dfRezTI + dfRezTI * dfNewRezPerc / 100 >= NewRez || dfRezTI + 1 >= NewRez))
						//||
						//	(	dfRezLockedTI
						//	&&	(dfRezTI - dfRezTI / 10 <= NewRez || dfRezTI - 4 <= NewRez))
						)
						{
							dfResistance = dfRezTI;
							RezMillis = ( dfMillis & 0xf0 ) >> 4;
						}
						else
						{
							if ( dfRezTI - dfRezTI * dfNewRezPerc / 100 > NewRez && dfRezTI - 1 > NewRez )
							{
								dfResistance = NewRez;
								RezMillis = NewMillis;

								dfRezTI = NewRez;
								dfMillis = ( dfMillis & ~0xf0 ) | ( NewMillis << 4 );
							}
						}
					}
					else
					{
						dfResistance = NewRez;
						RezMillis = NewMillis;

						dfRezTI = NewRez;
						dfMillis = ( dfMillis & ~0xf0 ) | ( NewMillis << 4 );
					}
				}
			}

			if ( gFlags.new_rez_ss && dfMode == 2 )
			{
				gFlags.new_rez_ss = 0;

				if ( !dfRezSS )
				{
					dfRezSS = dfResistance;
					dfMillis = ( dfMillis & ~0xf00 ) | ( RezMillis << 8 );
				}
				else if ( !dfRezLockedSS )
				{
					word_200000BC = dfRezSS * 10 + ( ( dfMillis >> 8 ) & 0xf );

					if ( 3 * dfRezSS >= 2 * NewRez )
					{
						if (	dfRezSS + dfRezSS * dfNewRezPerc / 100 < NewRez
							&&	dfRezSS + 1 < NewRez
							//&&	!dfRezLockedSS 
                                                        )
						{
							gFlags.new_rez_ss = 1;
							Event = 32;
							return;
						}
						else
						{
							if
							(
								(	( dfRezSS - dfRezSS * dfNewRezPerc / 100 <= NewRez || dfRezSS - 1 <= NewRez )
								&&	( dfRezSS + dfRezSS * dfNewRezPerc / 100 >= NewRez || dfRezSS + 1 >= NewRez ) )
							//||
							//	(	  dfRezLockedSS
							//	&&	( dfRezSS - dfRezSS / 10 <= NewRez || dfRezSS - 4 <= NewRez ) )
							)
							{
								dfResistance = dfRezSS;
								RezMillis = ( dfMillis & 0xf00 ) >> 8;
							}
							else if ( dfRezSS - dfRezSS * dfNewRezPerc / 100 > NewRez && dfRezSS - 1 > NewRez )
							{
								dfResistance = NewRez;
								RezMillis = NewMillis;

								dfRezSS = NewRez;
								dfMillis = ( dfMillis & ~0xf00 ) | ( NewMillis << 8 );
							}
						}
					}
					else
					{
						dfResistance = NewRez;
						RezMillis = NewMillis;

						dfRezSS = NewRez;
						dfMillis = ( dfMillis & ~0xf00 ) | ( NewMillis << 8 );
					}
				}
			}

			if ( gFlags.new_rez_tcr && dfMode == 3 )
			{
				gFlags.new_rez_tcr = 0;

				if ( !dfRezTCR )
				{
					dfRezTCR = dfResistance;
					dfMillis = ( dfMillis & ~0xf000 ) | ( RezMillis << 12 );
				}
				else if ( !dfRezLockedTCR )
				{
					word_200000BE = dfRezTCR * 10 + ( dfMillis >> 12 );

					if ( 3 * dfRezTCR >= 2 * NewRez )
					{
						if (	dfRezTCR + dfRezTCR * dfNewRezPerc / 100 < NewRez
							&&	dfRezTCR + 1 < NewRez
							//&&	!dfRezLockedTCR 
                                                        )
						{
							gFlags.new_rez_tcr = 1;
							Event = 32;
							return;
						}
						else
						{
							if
							(
								(	( dfRezTCR - dfRezTCR * dfNewRezPerc / 100 <= NewRez || dfRezTCR - 1 <= NewRez )
								&&	( dfRezTCR + dfRezTCR * dfNewRezPerc / 100 >= NewRez || dfRezTCR + 1 >= NewRez ) )
							//||
							//	(	  dfRezLockedTCR
							//	&&	( dfRezTCR - dfRezTCR / 10 <= NewRez || dfRezTCR - 4 <= NewRez ) )
							)
							{
								dfResistance = dfRezTCR;
								RezMillis = ( dfMillis & 0xf000 ) >> 12;
							}
							else if ( dfRezTCR - dfRezTCR * dfNewRezPerc / 100 > NewRez && dfRezTCR - 1 > NewRez )
							{
								dfResistance = NewRez;
								RezMillis = NewMillis;
								
								dfRezTCR = NewRez;
								dfMillis = ( dfMillis & ~0xf000 ) | ( NewMillis << 12 );
							}
						}
					}
					else
					{
						dfResistance = NewRez;
						RezMillis = NewMillis;

						dfRezTCR = NewRez;
						dfMillis = ( dfMillis & ~0xf000 ) | ( NewMillis << 12 );
					}
				}
			}

			UpdateDFTimer = 50;

//------------------------------------------------------------------------------

			if ( ISVTCDUAL )
			{
				GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
				PD1 = 0;
			}
			else if ( !ISCUBOID && ! ISCUBO200 && !ISRX200S && !ISRX23 
                                && !ISRX300 && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR 
                                && !ISGEN3 && !ISRX2 && !ISINVOKE && !ISSINFJ200 && !ISRX217 && !ISGEN2)
			{
				GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
				PD7 = 0;
			}

			if ( ISEGRIPII || ISEVICAIO || ISSINFJ200 || ISSINP80 ) 
			{
				if ( !dfStealthOn ) // != 0 for stealth contrast too
				{
					LEDTimer = 0;
					gFlags.led_on = 1;
				}
			}

		//	myprintf( "StartFire\n" );

                        if ( !VapeDelayTimer )
                        {
                            gFlags.firing = 1;
                        }

			FireDuration = 0;
                        CurveRepeatTimerDuration=0;
                        AtoRezMilliMin = AtoRezMilli;
                        
                        if ( Screen == 1 ) gFlags.FireNotFlipped = 1;
        
			if ( BattProbeCount == 1 ) BattProbeCount = 2;

			switch ( dfTempAlgo )
			{
				case 1:
					GetTempCoef( TempCoefsNI );
					break;

				case 2:
					GetTempCoef( TempCoefsTI );
					break;

				case 3:
					TCR = 120;
					break;

				case 4:
					if ( dfMode == 3 || dfMode == 4 || dfMode == 5 ) //for power & bypass too
					{
						TCR = dfTCRM[dfTCRIndex];
					}
					else if ( dfMode < 3 )
					{
						TCR = dfTCRP[dfMode];
					}
					break;

				default:
					break;
			}

                //if ( gFlags.pbank ) 
                //        TargetVolts = 500;
                //else           
			TargetVolts = 100;
			////PowerScale = 100;
                
                        
			if ( ISMODETC(dfMode) )
			{
                                InitTCAlgo(); //put here for vtwo mini bug
                            
				if ( dfResistance <= 150 )
				{
					if ( !gFlags.check_mode && !dfStatus.chkmodeoff )
					{
						if ( dfRezType != 1 )
						{
							tempf = dfTemp;
							if ( dfStatus.IsCelsius ) tempf = CelsiusToF( dfTemp );

							// 10W - 40W on full temp range
							int p  = 100 + ( 3 * ( tempf - 200 ) / 4 );

							TargetVolts = GetAtoVWVolts( p, AtoRez );
						}
						else
						{
							dfMode = 4;
						}
					}
					else
					{
						v21 = dfTCPower;
						if ( v21 > 2 * MaxPower / 3 ) v21 = 2 * MaxPower / 3;
						if ( v21 < 300 ) v21 = 300;

						v22 = AtoPowerLimit( v21 );
						v23 = GetVoltsForPower( v22 );

						TargetVolts = v23;
					}
				}
				else
				{
					dfMode = 4;
				}
			}

			if ( !ISMODETC(dfMode) )
			{
				if ( AtoRez < 5 ) //0.05
				{
					StopFire();
					Event = 27;
					return;
				}
			}
			//else
			//{
			//	InitTCAlgo(); // went upstairs
			//}

			if ( dfMode == 6 )
			{
				pwr = dfSavedCfgPwr[ConfigIndex];
			}
			else
			{
				pwr = dfPower;
			}

                        uint8_t pc = 0;
			if ( ISMODEVW(dfMode) )
			{
				if ( dfStatus.pcurve )
				{
					pwr = dfPwrCurve[0].power * pwr / 100;

					if ( pwr > AtoMaxPower )
					{
						pwr = AtoMaxPower;
					}
                                        if ( !CurveDelay ) pc = 1;
				}
				//else if ( !PreheatDelay && dfStatus.preheat )
                                else if ( dfStatus.preheat )
				{
                                    if (dfStatus2.smart_ph && dfPHDelay && NextPreheatTimer )
                                    {
                                        PreheatTimer = NextPreheatTimer;
                                    } 
                                    else if ( !PreheatDelay )
                                    {
					PreheatTimer = dfPreheatTime;
                                    }
                                        

					if ( dfStatus.phpct )
					{
						PreheatPower = dfPower * dfPreheatPwr / 100;
					}
					else
					{
						PreheatPower = dfPreheatPwr;
					}

					if ( PreheatPower > AtoMaxPower )
					{
						PreheatPower = AtoMaxPower;
					}

					pwr = PreheatPower;
                                        pc = 1;
				}


				gFlags.limit_power = 0;
//                              if ( pwr > 300 ) pwr = 300;
				if ( pwr > BatteryMaxPwr )
				{
					gFlags.limit_power = 1;
				//	PowerScale = 100 * BatteryMaxPwr / pwr;
					pwr = BatteryMaxPwr;
                                        pc = 1;
				}
                                
                    //if ( !gFlags.pbank ) 
                    //{
                                if ( dfStatus.vvlite && !pc )
                                {
                                    if ( !dfVVLockedVolt ) dfVVLockedVolt = VWVolts;
                                    TargetVolts = dfVVLockedVolt;
                                }
                                else
                                {
                                    TargetVolts = GetVoltsForPower( pwr );//* PowerScale / 100 );
                                }
                    //}
                    //else
                    //{
                    //    TargetVolts = 500;
                    //}
                                
				LowBatVolts = ( BatteryVoltage > BatteryCutOff + 100 ) ? 0 : BatteryVoltage;
			}

			if ( ISMODEBY(dfMode) )
			{
				TargetVolts = BypassVolts; //AtoMaxVolts;
			}

			SetADCState( 1, 1 );
			SetADCState( 2, 1 );
			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 1 );
			}
                        
			AtoWarmUp();
                        
			if ( !gFlags.firing || LastInputs != 1 )
				StopFire();
                        
                        SetScreen( 2, 1 );
			//gFlags.refresh_display = 1;
			//Screen = 2; //on fire screen
			//ScreenDuration = 1;
			return;
		}

//------------------------------------------------------------------------------

		default:
			return;

//------------------------------------------------------------------------------

		case 34:	// Show battery voltage
			//if ( !dfStatus.off )
			//	return;
                        
                        SetScreen( 54, 30 );
			//gFlags.refresh_display = 1;
			//Screen = 54;
			//ScreenDuration = 5;
			return;

		case 32:	// New coil
			StopFire();
                        SetScreen( 51, 10 );
			//gFlags.refresh_display = 1;
			//Screen = 51;
			//ScreenDuration = 10;
			return;

/*
		case 31:	// Show board temperature
			if ( dfStatus.off )
				return;
			gFlags.refresh_display = 1;
			Screen = 37;
			ScreenDuration = 5;
			return;
*/

		case 30:	// Key lock violation
			if ( dfStatus.off )
				return;
                        
                        SetScreen( 28, 2 );
			//gFlags.refresh_display = 1;
			//Screen = 28;
			//ScreenDuration = 2;
			return;

		case 29:	// FW Version screen
			if ( dfStatus.off )
				return;
                        
                        SetScreen( 50, 10 );
			//gFlags.refresh_display = 1;
			//Screen = 50;
			//ScreenDuration = 10;
			return;

		case 28:	// Battery < 3.1V idle or < 2.8V firing
			StopFire();
			KeyPressTime |= 0x8000;
                        SetScreen( 24, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 24; // Battery Low
			//ScreenDuration = 2;
			return;

		case 27:	// Atomizer Low
			StopFire();
                        SetScreen( 22, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 22;
			//ScreenDuration = 2;
			KeyPressTime |= 0x8000;
			return;

		case 26:	// No Atomizer Found
			StopFire();
                        SetScreen( 20, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 20;
			//ScreenDuration = 2;
			return;

		case 25:	// Atomizer short
                        SetScreen( 21, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 21;
			//ScreenDuration = 2;
			return;
                        
		case 70:	// Atomizer short current
                        SetScreen( 70, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 70;
			//ScreenDuration = 2;
			return;
                        
                case 71:	// Atomizer short Bad contact
                        SetScreen( 71, 3 );
			//gFlags.refresh_display = 1;
			//Screen = 71;
			//ScreenDuration = 2;
			return;
                        
		case 24:	// 10s Fire protection
			StopFire();
			if ( AtoError )
				return;
			if ( FireDuration >= dfProtec ) //FIRE_PROTEC_MAX )
			{
                                SetScreen( 23, 4 );
				//gFlags.refresh_display = 1;
				//Screen = 23;
				//ScreenDuration = 3;
			}
			return;

		case 23:	// Reset Time counter
			dfTimeCount = 0;
			UpdatePTTimer = 80;
			EditModeTimer = 1000;
			gFlags.refresh_display = 1;
			gFlags.draw_edited_item = 1;
			return;

		case 22:	// Reset Puff counter
			dfPuffCount = 0;
			UpdatePTTimer = 80;
			EditModeTimer = 1000;
			gFlags.refresh_display = 1;
			gFlags.draw_edited_item = 1;
			return;
                        
		//case 20:	// Show Info
		//	if ( dfStatus.off )
		//		return;
		//	gFlags.refresh_display = 1;
		//	Screen = 100;
		//	ScreenDuration = 10;
		//	return;
                        
		case 18:	// Flip display
			if ( !dfStatus.off )
				return;
                        
			dfStatus.flipped ^= 1;
                        dfStatus2.swap_mp ^= 1;
			InitDisplay();
                        SetScreen( 1, 2 );
			//gFlags.refresh_display = 1;
			//Screen = 1;
			//ScreenDuration = 2;
			UpdateDataFlash();
			return;

		case 17:	// Switch On/Off
			if ( gFlags.firing )
				return;
                        
			if ( dfStatus.off ) //switch mod on
			{
				gFlags.sample_vbat = 1;
				ReadBatteryVoltage();                                                                
				if ( ( BatteryVoltage > BatteryCutOff + 20 ) || gFlags.usb_attached )
				{
					dfStatus.off = 0;
                                        if ( dfStatus2.splash0 || dfStatus2.splash1 ) //01 10 on
                                        {
                                                SplashTimer = 2;
                                        }
					MainView();
				}
			}
			else //swith to off
			{                         
                                if ( !gFlags.FireNotFlipped )
                                {
                                    gFlags.FireNotFlipped = 1;
                                    dfStatus.flipped ^= 1;
                                    InitDisplay();
                                }
                                
                                PuffsOffCount = 0;
                                AwakeTimer = 0;
                                
				dfStatus.off = 1;
				gFlags.refresh_display = 1;
				//LEDOff();
                                
				if ( gFlags.battery_charging )
				{
					ChargeView();
					BatAnimLevel = BatteryTenth;
				}
				else
				{
                                        SetScreen( 61, SwitchOffCase ? 4 : 2 ); //goodbye
				//	Screen = 0;
				//	SleepTimer = 0;
				}
                                
			}
			return;

		case 16:	// Edit mode
			if ( dfStatus.off )
				return;
			gFlags.draw_edited_item = 1;
			EditItemIndex = 0;
			EditModeTimer = 1000;
			MainView();
			return;

		case 15:	// Single Fire click
			if ( dfStatus.off || gFlags.firing )
				return;
			if ( gFlags.refresh_battery )
			{
				gFlags.refresh_battery = 0;
				gFlags.sample_vbat = 1;
				ReadBatteryVoltage();
			}
			if ( Screen == 1 )
			{
				if ( EditModeTimer )
				{
					EditModeTimer = 0;
					gFlags.edit_capture_evt = 0;
					gFlags.draw_edited_item = 1;
					UpdateDFTimer = 50;
				}
			}
			MainView();
			return;

		case 13:	// Battery charge stop
			gFlags.battery_charging = 0;
                        if ( Screen == 5 )
			{
				gFlags.refresh_display = 1;
				Screen = 0;
				if ( dfStatus.off )
					SleepTimer = 0;
				else
					SleepTimer = dfDimOffTimeout * 100; //18000;
			}
			return;

		case 12:	// Battery charge start
			gFlags.battery_charging = 1;
			gFlags.refresh_display = 1;
			BatAnimLevel = BatteryTenth;
                        gFlags.nbcr = 0;
                        
			if ( !IsMenuScreen() )
			{
				if ( BatteryStatus == 1 )
				{
					Screen = 55; //Imbalanced Batteries
					ScreenDuration = 2;
				}
				else if ( dfStatus.off )
				{
					ChargeView();
				}
				else
				{
					if ( Screen != 5 )
						MainView();
				}
			}
			return;

		case 11:	// USB cable detach
			if ( ISVTCDUAL )
			{
				PF2 = 0;
				PA2 = 0;
			}
			ChargeMode = 0;
			ChargeStep = 0;
			ChargeStatus = 0;
			if ( BatteryStatus == 3 || BatteryStatus == 4 )
			{
				BatteryStatus = 0;
			}
			gFlags.usb_attached = 0;
			gFlags.battery_charging = 0;
                        dfStatus.usbchghotoff = 0;
			gFlags.monitoring = 0;
                        gFlags.refresh_display = 1;
			if ( Screen == 5 )
			{
				if ( dfStatus.off || dfStealthOn == 1 )
				{
                                    	Screen = 0;
					gFlags.refresh_display = 1;
                                        
					if ( dfStatus.off )
						SleepTimer = 0;
					else
						SleepTimer = dfDimOffTimeout * 100; //18000;
				}
				else
				{
					MainView();
				}
			}
			return;

		case 10:	// USB cable attach
			ChargeMode = 0;
			ChargeStep = 0;
			ChargeStatus = 1;
			if ( NumBatteries > 1 )
			{
                            if ( ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 || ISINVOKE || ISSINFJ200 || ISRX217 || ISGEN2 )
                            {
                                USBMaxLoad = 3;
                            } 
                            else 
                            {
                                USBMaxLoad = 2;
                            }				
			}
			else
			{
				USBMaxLoad = 1;
			}
			gFlags.low_battery = 0;
			gFlags.usb_attached = 1;
			if ( !dfStatus.off )
			{
				if ( Screen == 0 )
				{
					ChargeView();
				}
				else
				{
					MainView();
				}
			}
			return;

		case 6:		// Stealth On/Off
			if ( dfStatus.off )
				return;
                        
			//dfStealthOn = ( dfStealthOn == 0 );
                        if ( ++dfStealthOn > 2 ) 
                            dfStealthOn = 0; // 2 fire contrast
                        
                        SetScreen( 40, 2 );
			//gFlags.refresh_display = 1;
			//Screen = 40;
			//ScreenDuration = 2;
			return;

		case 4:		// Key (Un)Lock
			if ( dfStatus.off )
				return;
			dfStatus.keylock ^= 1;
                        
			if ( dfStatus.keylock )
				Screen = 28;
			else
				Screen = 31;
                        
			ScreenDuration = 2;
			gFlags.refresh_display = 1;
			UpdateDFTimer = 50;
			return;

//------------------------------------------------------------------------------

		case 3:		// - (minus or left) button
		{
                    
                        if ( Screen == 5 ) //&& dfStealthOn != 1 ) //charge scr on off
			{
				//gFlags.screen_on = ( gFlags.screen_on == 0 );                            
                                gFlags.screen_on ^= 1;                            
                                ScreenDuration = ScrChargeTimes[dfScrChargeTime]; //ScreenDuration = 0;                               
                                gFlags.refresh_display = 1;
			}
                                            
			if ( dfStatus.off )
			{
				return;
			}

			if ( Screen == 0 || Screen == 60 )
			{
                                    if ( dfStatus.wakeonpm )
                                    {
					MainView();
                                    }         
			}                        
			else if ( Screen == 2 )
			{
				MainView();
			}
			else if ( Screen == 51 ) //new coil screen [OLD]
			{
				switch ( dfMode )
				{
					case 0:
						dfResistance = dfRezNI;
						RezMillis = dfMillis & 0xf;
						gFlags.new_rez_ni = 0;
						break;
					case 1:
						dfResistance = dfRezTI;
						RezMillis = ( dfMillis & 0xf0 ) >> 4;
						gFlags.new_rez_ti = 0;
						break;
					case 2:
						dfResistance = dfRezSS;
						RezMillis = ( dfMillis & 0xf00 ) >> 8;
						gFlags.new_rez_ss = 0;
						break;
					case 3:
						dfResistance = dfRezTCR;
						RezMillis = ( dfMillis & 0xf000 ) >> 12;
						gFlags.new_rez_tcr = 0;
						break;
					default:
						break;
				}
				MainView();
			}
			else if ( Screen == 1 )
			{
				KeyUpTimer = 10;

				if ( EditModeTimer ) // -
				{
					EditModeTimer = 1000;

					if ( gFlags.edit_capture_evt )
					{
                                            if ( EditItemIndex == 2 )
                                            {
						dfStatus.priopwr ? TempMinus() : PowerMinus( &dfTCPower, 10, MaxTCPower );
                                            }
                                            else if ( EditItemIndex == 4 )
                                            {
                                                if ( --dfAPT3 > 14 ) dfAPT3 = 14;
                                            }
                                            else if ( EditItemIndex == 5 )
                                            {
                                                if ( --dfAPT > 14 ) dfAPT = 14;
                                            }
					}
					else if ( !ISMODETC(dfMode) )
					{
						if ( dfMode == 6 ) //smart
						{                                                        
                                                        if ( ++EditItemIndex > 6 )
                                                        {
								EditItemIndex = 0;
                                                        }
                                                        else if ( EditItemIndex < 4 )
                                                        {
								EditItemIndex = 4;                                  
                                                        }
/*
							if ( EditItemIndex == 0 )
							{
								EditItemIndex = 6;
							}
							else
							{
								EditItemIndex = 0;
							}
*/
						}
						else
// change info lines for edit
						{
							if ( EditItemIndex == 0 )
							{
								EditItemIndex = 3; //re-read rez in VW
							}
							else
							{
                                                                if ( dfUIVersion == 1 && EditItemIndex == 3 )
                                                                {
                                                                    ++EditItemIndex; //skip one info line
                                                                }
								if ( ++EditItemIndex > 6 )
									EditItemIndex = 0;
							}
						}
					}
					else
					{
                                                if ( dfUIVersion == 1 && EditItemIndex == 3 )
                                                {
                                                        ++EditItemIndex; //skip 3-d info line
                                                }
						if ( ++EditItemIndex > 6 )
							EditItemIndex = 0;
					}
				}
				else // no edit, minus button
				{
					if ( ISMODETC(dfMode) )
					{
						dfStatus.priopwr ? PowerMinus( &dfTCPower, 10, MaxTCPower ) : TempMinus();
					}
					else if ( dfMode == 6 ) // Smart
					{
						if ( ConfigIndex < 10 )
						{
                                                        SmartPowerPM( 0 ); 
/*
							spwr = dfSavedCfgPwr[ConfigIndex];
							spwr -= spwr % WattsInc; //10;
							spwr -= WattsInc; //10;
							if ( spwr < AtoMinPower ) spwr = AtoMinPower;

							dfSavedCfgPwr[ConfigIndex] = spwr;
							dfVWVolts = GetAtoVWVolts( spwr, AtoRez );
                                                        //dfPower = spwr;
*/
						}
					}
					else if ( dfMode == 4 )
					{
						PowerMinus( &dfPower, AtoMinPower, AtoMaxPower );
						VWVolts = GetAtoVWVolts( dfPower, AtoRez );
                                                dfVVLockedVolt = GetAtoVWVolts( dfPower, dfResistance );
						//if ( ConfigIndex < 10 && !AtoError && AtoRez )
						//{
						//	dfSavedCfgPwr[ConfigIndex] = dfPower;
						//}
					}
				}

                                MainView();
                                if ( KeyTicks >= 5 )
                                {
					gFlags.draw_edited_item = 1;
                                        DrawScreen();
                                }
                                UpdateDFTimer = 50;
			}

			break;
		}

//------------------------------------------------------------------------------

		case 2:		// + (plus or right) button
		{
			//if ( dfStatus.off )
			//{
			//	return;
			//}

                        if ( dfStatus.off )
			{
                            if ( Screen == 61 ) // hide goodbye scr
                            {
                                Screen = 0;
				SleepTimer = 0;
                                gFlags.refresh_display = 1;
                            }
                            else if( !gFlags.battery_charging && Screen == 0 )
                            {
                                SetScreen( 61, SwitchOffCase ? 4 : 2 ); //goodbye
                            }                           
                        return;                       
			}
                        
			if ( Screen == 0 || Screen == 60 )
			{
				if ( dfStatus.wakeonpm )
				{
					MainView();
				}
			}
			else if ( Screen == 2 )
			{
				MainView();
			}
			else if ( Screen == 51 ) //new coil screen [NEW]
			{
				switch ( dfMode )
				{
					case 0:
						dfRezNI = NewRez;
						dfMillis = ( dfMillis & ~0xf ) | NewMillis;
						gFlags.new_rez_ni = 0;
						break;
					case 1:
						dfRezTI = NewRez;
						dfMillis = ( dfMillis & ~0xf0 ) | ( NewMillis << 4 );
						gFlags.new_rez_ti = 0;
						break;
					case 2:
						dfRezSS = NewRez;
						dfMillis = ( dfMillis & ~0xf00 ) | ( NewMillis << 8 );
						gFlags.new_rez_ss = 0;
						break;
					case 3:
						dfRezTCR = NewRez;
						dfMillis = ( dfMillis & ~0xf000 ) | ( NewMillis << 12 );
						gFlags.new_rez_tcr = 0;
						break;
					default:
						break;
				}
				MainView();
			}
			else if ( Screen == 1 ) //main screen
			{
				KeyUpTimer = 10;

				if ( EditModeTimer )
				{
					EditModeTimer = 1000;
					gFlags.draw_edited_item = 1;

					switch ( EditItemIndex )
					{
						case 0:
							NextMode();
							break;

						case 1:
							if ( dfMode == 3 )
							{
								if ( ++dfTCRIndex >= 3 )
									dfTCRIndex = 0;
							}
							else if ( dfMode == 0 || dfMode == 1 || dfMode == 2 )
							{
								++dfTCMode;
								if ( dfTCMode > 2 )
									dfTCMode = 0;
								dfMode = dfTCMode;
								ModeChange();
							}
							break;

						case 2:
							dfStatus.priopwr ? TempPlus() : PowerPlus( &dfTCPower, 10, MaxTCPower );
							gFlags.edit_capture_evt = 1; // for use minus button here too
							break;

						case 3:
							SwitchRezLock( 1 );
							break;

                                                case 4:
							if ( ++dfAPT3 > 14 ) dfAPT3 = 0;
                                                        gFlags.edit_capture_evt = 1;
							break;
                                                        
                                                case 5:
							if ( ++dfAPT > 14 ) dfAPT = 0;
                                                        gFlags.edit_capture_evt = 1;
							break;

                                                case 6:       
                                                        if ( NumBatteries > 1 )
                                                        {
                                                            if ( ++dfBattLine > 3 ) dfBattLine = 0;
                                                        }
                                                        else
                                                        {
                                                            if ( ++dfBattLine > 2 ) dfBattLine = 0;
                                                        }
/*
							if ( !dfStatus.battpc )
							{
								dfStatus.battpc = 1;
								dfStatus.battv = 0;
							}
							else if ( !dfStatus.battv )
							{
								dfStatus.battv = 1;
							}
							else
							{
								dfStatus.battv = 0;
								dfStatus.battpc = 0;
							}
*/
							break;
					}

				}
				else //plus button, not in edit
				{
					switch ( dfMode )
					{
						case 0:
						case 1:
						case 2:
						case 3:
							dfStatus.priopwr ? PowerPlus( &dfTCPower, 10, MaxTCPower ) : TempPlus();
							break;

						case 4:
							PowerPlus( &dfPower, AtoMinPower, AtoMaxPower );
							VWVolts = GetAtoVWVolts( dfPower, AtoRez );
                                                        dfVVLockedVolt = GetAtoVWVolts( dfPower, dfResistance );
							//if ( ConfigIndex < 10 && !AtoError && AtoRez )
							//{
							//	dfSavedCfgPwr[ConfigIndex] = dfPower;
							//}
							break;

						case 6: // +
							if ( ConfigIndex < 10 )
                                                            //&& !AtoError && AtoRez
							{
                                                            SmartPowerPM( 1 ); 
/*
								spwr = dfSavedCfgPwr[ConfigIndex];
								spwr -= spwr % WattsInc; //10;
								spwr += WattsInc; //10;
								if ( spwr > AtoMaxPower ) spwr = AtoMaxPower;

								dfSavedCfgPwr[ConfigIndex] = spwr;
								dfVWVolts = GetAtoVWVolts( spwr, AtoRez );
                                                                //dfPower = spwr;
*/
							}
							break;

						case 5:
						default:
							break;
					}

				}

                                MainView();
                                if ( KeyTicks >= 5 )
                                {
					gFlags.draw_edited_item = 1;
                                        DrawScreen();
                                }
                                UpdateDFTimer = 50;
			}

			break;
		}
	}
}
