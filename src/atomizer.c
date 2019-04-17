#include "myevic.h"
#include "screens.h"
#include "events.h"
#include "dataflash.h"
#include "battery.h"
#include "timers.h"
#include "meadc.h"
#include "megpio.h"
#include "myrtc.h"
#include "miscs.h"
#include "atomizer.h"
#include "display.h"

//=========================================================================

//=========================================================================

uint32_t	AtoVoltsADC;
uint32_t	AtoVolts;
uint32_t	AtoVoltsMax;
uint32_t	TargetVolts;
uint32_t        ReplayRez;
uint32_t	AtoRezMilli;
uint32_t	AtoRezMilliMin; //min while puff
uint32_t	AtoMinVolts;
uint32_t	AtoMaxVolts;
uint32_t	AtoMinPower;
uint32_t	AtoMaxPower;
uint32_t	MaxTCPower;
uint32_t	MaxVolts;
uint32_t	MaxPower;
uint32_t	MaxCurrent;
uint16_t	FireDuration;
uint8_t         CurveRepeatTimerDuration;
uint16_t	AtoTemp;
uint16_t        VWVolts;
uint16_t	AtoCurrent;
uint16_t	AtoCurrentMax;
uint16_t	AtoRez;
uint8_t		AtoMillis;
uint16_t	TCR;
uint8_t		AtoProbeCount;
uint8_t		AtoShuntRez;
uint8_t		AtoError;		// 0,1,2,3 = Ok,Open/Large,Short,Low
uint8_t		AtoStatus;		// 0,1,2,3,4,5 = Open,Short,Low,Large,Ok,overCurrent
uint8_t		BoardTemp;
uint8_t		AkkuTemp;
uint8_t		ConfigIndex;
uint8_t		PreheatTimer;
uint16_t	PreheatPower;
uint16_t	PreheatDelay;
uint16_t	CurveDelay;
uint16_t        NextPreheatTimer;
uint16_t	NextPreheatPower;
uint32_t	MilliJoules; // for vaped
uint32_t	MilliJoulesDay; // for vaped
uint32_t	MilliJoulesEnergy;
uint32_t	MilliJoulesVapedOn; //temp var for last session vaped
uint8_t         SessionPuffs;
        
uint8_t		RezMillis;

uint8_t		Set_NewRez_dfRez;
uint16_t	word_200000B8;
uint16_t	word_200000BA;
uint16_t	word_200000BC;
uint16_t	word_200000BE;
uint8_t		NewMillis;

uint8_t		LastAtoError;
uint16_t	LastAtoRez;
uint8_t		LastAtoMillis;

uint8_t         PuffsOffCount;
        
//-------------------------------------------------------------------------

uint8_t		BBCNextMode;
uint8_t		BBCMode;
uint16_t	BuckDuty;
uint16_t	BoostDuty;
uint16_t	PWMCycles;
uint16_t	MaxDuty;
uint16_t	MinBuck;
uint16_t	MaxBoost;
uint16_t	BoostWindow;
uint16_t	ProbeDuty;
uint16_t	SavedBoost;


//=========================================================================
// Temperature coefficients tables
//-------------------------------------------------------------------------

//const uint8_t TempCoefsNI[] =
//		{	10, 20, 35, 60, 80, 110, 120, 130, 140, 150, 160,
//			170, 170, 170, 170, 170, 170, 170, 170, 170, 170	};

//new table and Ni_0 = 68 '20C
const uint8_t TempCoefsNI[] =
		{	10, 16, 33, 48, 66, 82, 98, 120, 133, 150, 160, 180,
			190, 210, 220, 240, 250, 255, 255, 255, 255             };

//const uint8_t TempCoefsTI[] =
//		{	53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 54,
//			55, 55, 55, 55, 56, 56, 56, 56, 56, 56		};

const uint8_t TempCoefsTI[] =
		{	10, 10, 20, 30, 41, 51, 61, 71, 81, 91, 103,
			114, 124, 134, 144, 155, 165, 175, 185, 195, 205	};

//=========================================================================
__myevic__ void SetPWMClock()
{
	uint32_t clk;

	if ( gFlags.pwm_pll )
	{
		clk = CLK_CLKSEL2_PWM0SEL_PLL;
		PWMCycles = CLK_GetPLLClockFreq() / BBC_PWM_FREQ;
	}
	else
	{
		clk = CLK_CLKSEL2_PWM0SEL_PCLK0;
		PWMCycles = CLK_GetPCLK0Freq() / BBC_PWM_FREQ;
	}

	MaxDuty		= PWMCycles - 1;
	MinBuck		= PWMCycles / 48;
	MaxBoost	= PWMCycles / 12;
	ProbeDuty	= PWMCycles / 8;
	BoostWindow	= PWMCycles / 19;

#define MaxBuck  MaxDuty
#define MinBoost MaxDuty

	if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 || ISINVOKE || ISIKU200 )
	{
		MaxDuty = 95 * PWMCycles / 100; //for RegulateDualBuck
	}

	CLK_EnableModuleClock( PWM0_MODULE );
	CLK_SetModuleClock( PWM0_MODULE, clk, 0 );
	SYS_ResetModule( PWM0_RST );
}


//=========================================================================
//----- (00005C4C) --------------------------------------------------------
__myevic__ void InitPWM()
{
        //#define PWM0                 ((PWM_T *)   PWM0_BASE)
        //#define PWM0_BASE            (APBPERIPH_BASE + 0x18000)
        //#define APBPERIPH_BASE       (PERIPH_BASE + 0x00040000)
        //#define PERIPH_BASE          (0x40000000UL)  /*!< (Peripheral) Base Address */
        //PWM0 0x40058000
        //PWM1 0x40059000 
        //#define BBC_PWMCH_BUCK		0
        //#define BBC_PWMCH_BOOST		2
        //#define BBC_PWM_FREQ                  150000
    
	PWM_ConfigOutputChannel( PWM0, BBC_PWMCH_BUCK, BBC_PWM_FREQ, 0 );
        PWM_EnableOutput( PWM0, 1 << BBC_PWMCH_BUCK );
	PWM_EnablePeriodInt( PWM0, BBC_PWMCH_BUCK, 0 );
	PWM_Start( PWM0, 1 << BBC_PWMCH_BUCK );
        
	BuckDuty = 0;
        BoostDuty = 0;
	PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, 0 );
        
    if ( !ISINVOKE && !ISIKU200 )
    {
	PWM_ConfigOutputChannel( PWM0, BBC_PWMCH_BOOST, BBC_PWM_FREQ, 0 );
        PWM_EnableOutput( PWM0, 1 << BBC_PWMCH_BOOST );
	PWM_EnablePeriodInt( PWM0, BBC_PWMCH_BOOST, 0 );
        PWM_Start( PWM0, 1 << BBC_PWMCH_BOOST );
        
        PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, 0 );
    }

/**
 * @brief This macro set the comparator of the selected channel
 * @param[in] pwm The pointer of the specified PWM module
 * @param[in] u32ChannelNum PWM channel number. Valid values are between 0~5
 * @param[in] u32CMR Comparator of specified channel. Valid values are between 0~0xFFFF
 * @return None
 * @details This macro is used to set the comparator of specified channel.
 * @note This new setting will take effect on next PWM period.
 * \hideinitializer

#define PWM_SET_CMR(pwm, u32ChannelNum, u32CMR) ((pwm)->CMPDAT[(u32ChannelNum)]= (u32CMR))
**/
        

	if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISPRIMO1 
                || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 || ISINVOKE 
                || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
	{
		PWM_ConfigOutputChannel( PWM0, BBC_PWMCH_CHARGER, BBC_PWM_FREQ, 0 );
		PWM_EnableOutput( PWM0, 1 << BBC_PWMCH_CHARGER );
		PWM_Start( PWM0, 1 << BBC_PWMCH_CHARGER );

		ChargerDuty = 0;
		PWM_SET_CMR( PWM0, BBC_PWMCH_CHARGER, 0 );

		if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			MaxChargerDuty = 512;
		}
                else if ( ISPRIMO1 )
                {
                        MaxChargerDuty = 288;
                }
                else if ( ISPRIMO2 || ISPREDATOR )
                {
                        MaxChargerDuty = 320;
                }
                else if ( ISGEN3 || ISRX2 || ISINVOKE || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
                {
                        MaxChargerDuty = 360;
                }
		else
		{
			MaxChargerDuty = 256;
		}
	}
        
/*
        if ( ISSINFJ200 )
        {
                PWM_ConfigOutputChannel( PWM1, 4, 4800, 0 );
                PWM_EnableOutput( PWM1, 16 );
                PWM_Start( PWM1, 16 );
                // xduty = 0;
                PWM_SET_CMR( PWM1, 4, 0 );
                
        }
*/
        
        
}

//=========================================================================
//----- (00005CC0) --------------------------------------------------------
__myevic__ void BBC_Configure( uint32_t chan, uint32_t mode )
{
	if ( chan == BBC_PWMCH_BUCK ) //0
	{
            //R2, #0x40000000
            //LDR     R0, [R2,#0x40]
            //GPC_MFPL Offset: 0x40
            //
            //SYS_GPC_MFPL_PC0MFP_Msk          (0xF << SYS_GPC_MFPL_PC0MFP_Pos)
            //GPC_MFPL PC0 setting for PWM0_CH0
            //SYS_GPC_MFPL_PC0MFP_Pos = 0
            //BIC.W   R0, R0, #0xF
            
		SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC0MFP_Msk;

		if ( mode )
		{
                    //SYS_GPC_MFPL_PC0MFP_PWM0_CH0           (6ul << SYS_GPC_MFPL_PC0MFP_Pos)
                    //ORR.W   R0, R0, #6
                    
			SYS->GPC_MFPL |= SYS_GPC_MFPL_PC0MFP_PWM0_CH0;
		}
		else
		{
                    //PC 0x40004080
                    
			GPIO_SetMode( PC, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );
		}
	}
	else if ( chan == BBC_PWMCH_BOOST ) //2
	{
		SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC2MFP_Msk;

		if ( mode )
		{
			SYS->GPC_MFPL |= SYS_GPC_MFPL_PC2MFP_PWM0_CH2;
		}
		else
		{
			GPIO_SetMode( PC, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );
		}
	}
	else if ( chan == BBC_PWMCH_CHARGER ) //5
	{
		SYS->GPD_MFPL &= ~SYS_GPD_MFPL_PD7MFP_Msk;

		if ( mode )
		{
			SYS->GPD_MFPL |= SYS_GPD_MFPL_PD7MFP_PWM0_CH5;
		}
		else
		{
			GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
		}
	}
	else if ( chan == 3 ) //need for charge iKuu i200
	{
		SYS->GPA_MFPL &= ~SYS_GPA_MFPL_PA2MFP_Msk;

		if ( mode )
		{
			SYS->GPA_MFPL |= SYS_GPA_MFPL_PA2MFP_PWM1_CH3;
		}
		else
		{
			GPIO_SetMode( PA, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );
		}
	}
/*
 else if ( chan == 4 ) // used in FJ200 stock FW
	{
		SYS->GPA_MFPL &= ~SYS_GPA_MFPL_PA1MFP_Msk;

		if ( mode )
		{
			SYS->GPA_MFPL |= SYS_GPA_MFPL_PA1MFP_PWM1_CH4;
		}
		else
		{
			GPIO_SetMode( PA, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
		}
	}        
*/
}


//=========================================================================
//----- (00008F00) --------------------------------------------------------
__myevic__ void ClampAtoPowers()
{
	if ( AtoMinPower < 10 ) AtoMinPower = 10;
	if ( AtoMaxPower > MaxPower ) AtoMaxPower = MaxPower;
}


//=========================================================================
//----- (0000121C) --------------------------------------------------------
__myevic__ uint16_t ClampPower( uint16_t volts, int clampmax )
{
	uint16_t pwr;

	if ( AtoError || !AtoRez )
	{
		pwr = MaxPower;
	}
	else
	{
		pwr = volts * volts / ( 10 * AtoRez );

		ClampAtoPowers();
                        // ClampAtoPowers =>
                	//if ( AtoMinPower < 10 ) AtoMinPower = 10;
                        //if ( AtoMaxPower > MaxPower ) AtoMaxPower = MaxPower;

		if ( pwr < AtoMinPower )
			pwr = AtoMinPower;

		if ( clampmax && pwr > AtoMaxPower )
			pwr = AtoMaxPower;
	}
	return pwr;
}


//=========================================================================
//----- (00008F28) --------------------------------------------------------
__myevic__ void ClampAtoVolts()
{
	if ( AtoMinVolts < 50 ) AtoMinVolts = 50;
	if ( AtoMaxVolts > MaxVolts ) AtoMaxVolts = MaxVolts;
}


//=========================================================================
//----- (00001274) --------------------------------------------------------
__myevic__ uint16_t GetAtoVWVolts( uint16_t pwr, uint16_t r  )
{
	uint16_t volts; // r0@9

	if ( AtoError || !r ) //AtoRez
	{
		volts = 330;
	}
	else
	{
		volts = sqrtul( 10 * pwr * r ); //AtoRez

		ClampAtoVolts();

		if ( volts < AtoMinVolts ) volts = AtoMinVolts;
		if ( volts > AtoMaxVolts ) volts = AtoMaxVolts;
	}

	return volts;
}


//=========================================================================
//----- (00001334) --------------------------------------------------------
__myevic__ uint16_t CelsiusToF( uint16_t tc )
{
	return ( tc * 9 ) / 5 + 32;
}


//=========================================================================
//----- (000022A8) --------------------------------------------------------
__myevic__ uint16_t FarenheitToC( uint16_t tf )
{
	return ( 5 * ( tf - 32 )) / 9;
}

/*
//=========================================================================
__myevic__ uint16_t GetRegionalTemp( uint16_t atemp )
{
	return dfStatus.IsCelsius ? FarenheitToC( atemp ) : atemp;
}
*/

//=========================================================================
//----- (0000344C) --------------------------------------------------------
__myevic__ void StopFire()
{

//	register const uint32_t lr __asm__("lr");
//	uint32_t caller;
//	caller = lr;

	if ( ISVTCDUAL )
	{
		GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_INPUT );
	}
	else if (   !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && 
                    !ISRX300 && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR && 
                    !ISGEN3 && !ISINVOKE && !ISRX2 && !ISSINFJ200 && !ISRX217 && !ISGEN2 && !ISIKU200 )
	{
		GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_INPUT );  // 0x400040C0 0x80 0
                // ISEGRIPII ISEVICAIO ISSINP80 
	}

	PC1 = 0;                                    // 40004884
        
        if ( !ISVTCDUAL && !ISINVOKE && !ISIKU200 )
        {
            PC3 = 0;                                // 4000488C
        }
        
	BuckDuty = 0;
	PC0 = 0;                                    // 40004880
	BBC_Configure( BBC_PWMCH_BUCK, 0 );         // 0 0

        if ( !ISINVOKE && !ISIKU200 )
        {
            BoostDuty = 0;
            PC2 = 0;                                    // 40004888
            BBC_Configure( BBC_PWMCH_BOOST, 0 );        // 2 0
        }
        
	SetADCState( 1, 0 );
	SetADCState( 2, 0 );
        
	if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
	{
		SetADCState( 15, 0 );
	}

	if ( ISEGRIPII || ISEVICAIO || ISSINFJ200 || ISSINP80 )
	{
		LEDTimer = 20; //dfFireScrDuration * 10
	}

	if ( gFlags.firing )
	{
		gFlags.firing = 0;

		//RTCWriteRegister( RTCSPARE_VV_MJOULES, MilliJoules );
                //RTCWriteRegister( RTCSPARE_VV_MJOULESDAY, MilliJoulesDay );
                dfJoules = MilliJoules;
                dfJoulesDay = MilliJoulesDay;
                dfJoulesEnergy = MilliJoulesEnergy;
                
/*
		if (( FireDuration * 10 >= dfPreheatTime )
		||  ( dfStatus.pcurve && FireDuration > 10 ))
		{
			PreheatDelay = dfPHDelay * 100;
                        CurveDelay = dfCUDelay * 100;
		}       
*/
                //if ( dfStatus.preheat && FireDuration * 10 >= dfPreheatTime )
                //    PreheatDelay = dfPHDelay * 100;

                //if ( dfStatus.pcurve && FireDuration > 10 )
                //    CurveDelay = dfCUDelay * 100;
                
                if ( FireDuration > 5 ) // > 0.5 sec
		{
                        if ( dfStatus.preheat )
                            PreheatDelay = dfPHDelay * 100;
                    
                        if ( dfStatus.pcurve )
                            CurveDelay = dfCUDelay * 100;
                    
			dfTimeCount += FireDuration;
			if ( dfTimeCount > 999999 ) dfTimeCount = 0;
			if ( ++dfPuffCount > 99999 ) dfPuffCount = 0;
                        ++SessionPuffs;
                        
			UpdatePTTimer = 80;                        
                        
                        if ( dfPuffsOff && ++PuffsOffCount >= dfPuffsOff )
                        {   
                            // puffs counter limit - mod off
                            
                            //dfVapeDelayTimer = 70; //6000 = 60s (*100Hz)
                            
                            VapeDelayTimer = dfVapeDelayTimer;
                            dfStatus2.vapedelay = 1;
                            SwitchOffCase = 1;       
                            Event = 17;
                                                          
                        }      
		}             
	}

//	myprintf( "StopFire from 0x%08x\n", caller );

	AutoPuffTimer = 0;
        gFlags.apuff = 0;
	PreheatTimer = 0;

	LowBatVolts = 0;
	BatReadTimer = 200;
}


//=========================================================================
//----- (00003508) --------------------------------------------------------
__myevic__ uint16_t LowestRezMeasure()
{
	uint16_t rez;

	rez = dfResistance;
	if ( dfResistance > AtoRez && AtoRez )
		rez = AtoRez;
	if ( AtoRezMilli / 10 < rez && AtoRezMilli >= 10 )
		rez = ( AtoRezMilli / 10 );
        
        if ( AtoRezMilliMin > AtoRezMilli ) //|| !AtoRezMilliMin ) // not store when AtoRezMilliMin was 0
                AtoRezMilliMin = AtoRezMilli;
        
	return rez;
}


//=========================================================================
//----- (00003540) --------------------------------------------------------
__myevic__ uint16_t AtoPowerLimit( uint16_t pwr )
{
	uint32_t p, i2;

	i2 = MaxCurrent * MaxCurrent;

	if ( AtoRezMilli < MaxPower * 100 / i2 )
	{
		p = i2 * AtoRezMilli / 100;
		if ( pwr > p ) pwr = p;
	}
	return pwr;
}


//=========================================================================
//----- (00003564) --------------------------------------------------------
__myevic__ void ReadAtoCurrent()
{
	unsigned int adcShunt1, adcShunt2;
	unsigned int adcAtoVolts;
	unsigned int arez;
	unsigned int current1, current2;
        int c;
	//int s;

        if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISPRIMOSE || ISFIT || ISGEN3 || ISRX2 
                || ISINVOKE || ISSINP80 || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 
                || ( ISPRIMOMINI && dfHWVersion > 100 ) )
            c = 11; //lsrs B
        else
            c = 12;
        
	if ( gFlags.firing || gFlags.probing_ato )
	{
		if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			CLK_SysTickDelay( 10 );
			adcShunt2 = ADC_Read( 15 );
			if ( gFlags.firing && BuckDuty <= 25 && adcShunt2 > 200 )
				adcShunt2 = 0;
			CLK_SysTickDelay( 10 );
		}
		else
		{
			adcShunt2 = 0;
		}

		adcShunt1 = ADC_Read( 2 );
		CLK_SysTickDelay( 10 );
		adcAtoVolts = ADC_Read( 1 );

		if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			current1 = ( ( 10 * 2560 * adcShunt2 ) >> 12 ) / AtoShuntRez;
			if ( gFlags.firing )
			{
				current2 = ( ( 10 * 2560 * adcShunt1 ) >> 12 ) / AtoShuntRez;
				AtoCurrent = current1 + current2;
			}
			else
			{
				current2 = 0;
				AtoCurrent = current1;
			}
		}
		else
		{
			// Shunt current, in 10th of an Amp
			current1 = ( ( 10 * 2560 * adcShunt1 ) >> c ) / AtoShuntRez;
			current2 = 0;

			AtoCurrent = current1;
		}

		arez = LowestRezMeasure();
                        
		if	(  gFlags.firing
			   // ( shunt current ) > ( 1.6 * theorical ato current )
			&& 160 * 13 * adcAtoVolts / 100 * AtoShuntRez < 30 * ( adcShunt1 + adcShunt2 ) * arez
			&& AtoCurrent > 50		// 5.0A
			&& TargetVolts >= 100	// 1.00V
		)
		{
			//s = 2;
		}
	//	Definitely useless.
	//	else if ( ( current1 >= 255 || current2 >= 255 ) && !ISMODEBY(dfMode) )
	//	{
	//		// Original comparisons where "> 256".
	//		// This case can only occur if shunt resistance value is below 1 mOhm,
	//		// due to the resolution of the EADC. On all hardware versions, shunts
	//		// are between 1.00 and 1.25 mOhm so we could never get there.
	//		// This test seems useless anyway since AtoPowerLimit() is called
	//		// systematicaly before each power regulation, but eh.
	//		s = 3;
	//	}
		else
		{
                    if ( AtoStatus != 0 && TargetVolts > 100 )
                    {
                        if ( AtoCurrentMax < AtoCurrent ) AtoCurrentMax = AtoCurrent;
                        if ( AtoVoltsMax < AtoVolts ) AtoVoltsMax = AtoVolts;
                    }
			return;
		}

		AtoStatus = 5; // overcurrent //AtoStatus = 1

		if ( gFlags.firing ) Event = 70; //String_Error //25; 
                
                StopFire();
                
/*
		myprintf(	"\n"
					" Short %d! u32ADValue_Res_temp(%d) u32ADValue_CurVol_temp(%d)"
					" g_u16DetRes_I(%d.%d) u16Res(%d) g_u32Set_OutVol(%d).\n",
					s,
					adcShunt1,
					adcAtoVolts,
					AtoCurrent / 10,
					AtoCurrent % 10,
					arez,
					TargetVolts	);
*/
	}
}


//=========================================================================
//----- (00002F44) --------------------------------------------------------
__myevic__ void ReadAtoTemp()
{
        //result AtoTemp = 
	long t;
	long base_rez = dfResistance * 10 + RezMillis;

	if (	AtoRezMilli
		&&	AtoRezMilli <= 3000
		&&	dfResistance <= 150
		&&	AtoRezMilli <= 5 * base_rez )
	{
		if ( AtoRezMilli <= base_rez )
		{
			AtoTemp = CelsiusToF( dfColdLockTemp ); //70 F; when cold & no_fire
		}
		else if ( dfTempAlgo == 1 || dfTempAlgo == 2 ) //ni ti
		{
                        //new tables need for TCR calc
			AtoTemp = 100 * ( AtoRezMilli - base_rez ) / TCR + CelsiusToF( dfColdLockTemp ); // 68 Ni +140
		}
		//else if ( dfTempAlgo == 2 )
		//{
                //        //new table need for TCR calc
                //        AtoTemp = 100 * ( AtoRezMilli - base_rez ) / TCR + CelsiusToF( dfColdLockTemp );
		//	//AtoTemp = 10 * AtoRezMilli * TCR / base_rez - 460; // Ti
		//}
		else if ( dfTempAlgo > 2 ) //( dfTempAlgo == 3 || dfTempAlgo == 4 ) //SS, mTCR, (simple Ni Ti SS)
		{
			t = base_rez * TCR;
			t = 100000 * ( AtoRezMilli - base_rez ) / t + dfColdLockTemp; //20;
			AtoTemp = CelsiusToF( t );
		}
	}
}


//=========================================================================
//----- (00002FF0) --------------------------------------------------------
__myevic__ void GetTempCoef( const uint8_t tc[] )
{
	int r; // r2@4
	int v; // r4@6

	if ( dfResistance > 100 )
	{
		TCR = tc[20];
		return;
	}

	r = dfResistance / 5;
	TCR = tc[r];

	if ( dfResistance % 5 || r < 20 )
	{
		v = tc[r + 1];
		if ( v <= TCR )
		{
			TCR = ( TCR - ( TCR - v ) / 5 * ( dfResistance % 5 ) );
			if ( v > TCR ) TCR = v;
		}
		else
		{
			TCR = ( TCR + ( v - TCR ) / 5 * ( dfResistance % 5 ) );
			if ( v < TCR ) TCR = v;
		}
	}
}


//=========================================================================
//----- (00003078) --------------------------------------------------------
__myevic__ void CheckMode()
{
	unsigned int v0; // r0@2
	unsigned int v1; // r1@4

	static uint8_t CheckModeCounter;
        
	if ( AtoRezMilli / 10 <= AtoRez )
		v0 = 0;
	else
		v0 = ( AtoRezMilli / 10 - AtoRez );

	v1 = 10 * AtoRezMilli / AtoRez;

	if (	AtoRez > 150
		||	( v1 <= 115 && ( ( dfMode != 2 && dfMode != 3 ) || v1 <= 105 || v0 <= 1 ) ) )
	{
		if ( FireDuration < 10 ) //was 20, 2 sec puff for start check coil type
		{
			CheckModeCounter = 0;
			return;
		}

		dfStatus2.reztype = 0; // VW wire //dfRezType = 1;
		dfMode = 4;

		if ( dfPower > 200 )
		{
			dfPower = 200;
			VWVolts = GetAtoVWVolts( 200, AtoRez );
		}

		if ( !gFlags.new_rez_ni  )
		{
			dfRezNI  = word_200000BA / 10;
			dfMillis &= ~0xf;
			dfMillis |= word_200000BA % 10; //word_200000B8
		}
		if ( !gFlags.new_rez_ti  )
		{
			dfRezTI  = word_200000B8 / 10;
			dfMillis &= ~0xf0;
			dfMillis |= ( word_200000B8 % 10 ) << 4;
		}
		if ( !gFlags.new_rez_ss  )
		{
			dfRezSS  = word_200000BC / 10;
			dfMillis &= ~0xf00;
			dfMillis |= ( word_200000BC % 10 ) << 8;  //word_200000B8
		}
		if ( !gFlags.new_rez_tcr )
		{
			dfRezTCR = word_200000BE / 10;
			dfMillis &= ~0xf000;
			dfMillis |= ( word_200000BE % 10 ) << 12;  //word_200000B8
		}

		if ( AtoRez < 5 )
		{
			StopFire();
			Event = 27;
		}
	}
	else
	{
		if ( ++CheckModeCounter <= 3 )
			return;

		dfStatus2.reztype = 1; // TC wire //dfRezType = 2;
	}

	UpdateDFTimer = 50;

	gFlags.check_mode = 0;
	CheckModeCounter = 0;
}


//=========================================================================
//----- (00003250) --------------------------------------------------------
__myevic__ void ReadAtomizer()
{
    // 1000Hz from Main( on Fire ) + from Probe
    
	uint32_t ADCShuntSum;
	uint32_t ADCShuntSum1;
	uint32_t ADCShuntSum2;
	uint32_t ADCAtoSum;

	int NumShuntSamples;

	if ( TargetVolts )
	{
            
		if ( !gFlags.firing )
		{
                        NumShuntSamples = 20;

/*
			if ( AtoProbeCount == 10 )
			{
				NumShuntSamples = 50;
			}
			else
			{
				NumShuntSamples = 1;
			}
*/
		}
		else //firing
		{
			NumShuntSamples = 1;
		}

		ADCAtoSum = 0;
		ADCShuntSum1 = 0;
		ADCShuntSum2 = 0;

		for ( int count = 0 ; count < NumShuntSamples ; ++count )
		{                                            
			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				CLK_SysTickDelay( 10 );
				ADCShuntSum2 += ADC_Read( 15 );
			}
                        
			CLK_SysTickDelay( 10 );
			ADCShuntSum1 += ADC_Read( 2 );
			CLK_SysTickDelay( 10 );
			ADCAtoSum += ADC_Read( 1 );
		}

		ADCShuntSum = ( ADCShuntSum1 + ADCShuntSum2 ) ? : 1;
                
		AtoRezMilli = 13 * AtoShuntRez * ADCAtoSum / ( 3 * ADCShuntSum );
                
                if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISPRIMOSE || ISFIT || ISGEN3
                    || ISRX2 || ISSINP80 || ISINVOKE || ISSINFJ200 || ISRX217 || ISGEN2 
                    || ( ISPRIMOMINI && dfHWVersion > 100 ) || ISIKU200 )  
                {
                    AtoRezMilli >>= 1; //AtoRezMilli /= 2;
                }
                
		if ( gFlags.firing )
		{
			uint32_t pwr = AtoCurrent * AtoCurrent * AtoRezMilli / 10000; // P = I^2 * R
			MilliJoules += pwr; //*10
                        MilliJoulesDay += pwr;
                        MilliJoulesEnergy += pwr;
                        
                        if ( !dfStatus.keylock && dfStatus2.replay )
                        {
                                dfReplayRez = AtoRezMilli / 10;
                                dfReplayMillis = AtoRezMilli % 10;
                                ReplayRez = AtoRezMilli; 
                                dfReplayPower = pwr;
                        }
		}
                
//myprintf( "%d(%d,%d,%d)\n", AtoRezMilli, ADCAtoSum, ADCShuntSum1, ADCShuntSum2 );
                        
		if ( AtoRezMilli >= 5 )
		{
			if ( AtoRezMilli <= 20 )
			{
				AtoStatus = 1;
				//myprintf( "RL_GND %d(%d,%d,%d)\n",
				//		  AtoRezMilli, ADCAtoSum, ADCShuntSum1, ADCShuntSum2 );
				if ( gFlags.firing )
				{
					StopFire();
					Event = 25; //short
				}
				return;
			}
			if ( ( AtoProbeCount <= 10 && AtoRezMilli < 50 ) || AtoRezMilli < 40 )
			{
				//myprintf( "RL_LOW %d\n", AtoRezMilli );
				AtoStatus = 2;
				if ( gFlags.firing )
				{
					Event = 27; //low
				}
				return;
			}
                        
                    //if ( !gFlags.pbank ) 
                    //{
			if ( AtoRezMilli > 20000 )
			{
				AtoStatus = 0;
				AtoRezMilli = 0;
				if ( gFlags.firing )
				{
					Event = 26; //no ato
				}
				return;
			}
			if ( AtoProbeCount <= 10 && AtoRezMilli > 5000 ) //more if infin autofire? todo
			{
				AtoStatus = 3;
				//myprintf( "RL_LARGE %d\n", AtoRezMilli );
				return;
			}
			if ( gFlags.firing && AtoRezMilli / 10 <= AtoRez >> 2 )
			{
				StopFire();
				Event = 71; //25;  check contact bad
				//myprintf( "RL_GND2 %d\n", AtoRezMilli );
				return;
			}
			if ( (AtoStatus == 4 || !AtoStatus || AtoStatus == 3)
					&& AtoRezMilli >= 50 // = 0.05
					&& AtoRezMilli <= 5000
					&& (gFlags.firing || AtoProbeCount <= 10) )
			{
				AtoStatus = 4;
                                ReadAtoTemp();
			}
                        
                    //} //pbank
                    //else
                    //{
                    //    AtoStatus = 4;
                    //}
                        
		}
	}
}


//=========================================================================
// Dual Buck converter regulation
//-----------------------------------------------------------------------------
// (RX200S|RX2/3)
//-----------------------------------------------------------------------------
__myevic__ void RegulateDualBuck()
{
    
    //ISINVOKE && ISIKU200 do not have dual buck, but pwm(2) not enabled in InitPWM, proba
    
	if ( BBCMode != BBCNextMode )
	{
		if ( gFlags.firing )
		{
                    if ( !ISINVOKE && !ISIKU200 )
                    {
			BBC_Configure( BBC_PWMCH_BUCK2, 1 );
			BuckDuty = MinBuck;
			PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK2, BuckDuty );
			PC3 = 1;
                    }
		}

		BBC_Configure( BBC_PWMCH_BUCK1, 1 );
		BuckDuty = MinBuck;
		PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK1, BuckDuty );
		PC1 = 1;

		BBCMode = BBCNextMode;
	}

	if ( AtoVolts > TargetVolts )
	{
		if ( BuckDuty <= MinBuck )
			BuckDuty = 0;
		else
			--BuckDuty;
	}
	else if ( AtoVolts < TargetVolts )
	{
		if ( BuckDuty < MaxDuty )
			++BuckDuty;
	}

	if ( ( AtoStatus == 0 || AtoStatus == 1 || AtoStatus == 5 
                || ( !gFlags.firing && AtoProbeCount >= 12 ) )
		&& BuckDuty >= ProbeDuty )
	{
		BuckDuty = ProbeDuty;
	}

    if ( !ISINVOKE && !ISIKU200 )
    {
	PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK2, BuckDuty ); //2
    }
        
	PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK1, BuckDuty ); //0
}


//=========================================================================
//----- (00002CD8) --------------------------------------------------------
__myevic__ void RegulateBuckBoost()
{
	static uint8_t BBCNumCmps;
	static uint16_t bd = 0;
	static uint16_t av = 0;

	if ( gFlags.firing )
	{
		if ( CheckBattery() )
			return;
	}
	else
	{
		if ( !gFlags.probing_ato )
			return;
	}

	AtoVoltsADC = ADC_Read( 1 );
	AtoVolts = ( 1109 * AtoVoltsADC ) >> 12;

	if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 
                || ISIKU200 || ISINVOKE )
	{
		RegulateDualBuck();
		return;
	}

	switch ( BBCNextMode )
	{
		case 1:	// Transition mode Buck <-> Boost, or bypass mode
			{
				if ( BBCMode != 1 )
				{
					BBC_Configure( BBC_PWMCH_BOOST, 1 );
					BoostDuty = MinBoost;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, BoostDuty );
					if ( !ISVTCDUAL ) PC3 = 1;

					BBC_Configure( BBC_PWMCH_BUCK, 1 );
					BuckDuty = MaxBuck;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, BuckDuty );
					PC1 = 1;
				}

				if ( AtoVolts == TargetVolts )
				{
					BBCNumCmps = 0;
				}
				else if ( ++BBCNumCmps >= 5 )
				{
					if ( !ISMODEBY(dfMode) )
					{
						if ( BBCMode == 2 ) BBCNextMode = 3;
						else BBCNextMode = 2;
					}
					BBCMode = 1;
				}
			}
			break;

		case 2:	// Buck mode
			{
				if ( BBCMode != 2 )
				{
					BBC_Configure( BBC_PWMCH_BOOST, 1 );
					BoostDuty = MinBoost;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, BoostDuty );
					if ( !ISVTCDUAL ) PC3 = 1;

					BBC_Configure( BBC_PWMCH_BUCK, 1 );
					BuckDuty = ( BBCMode == 0 ) ? MinBuck : MaxBuck;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, BuckDuty );
					PC1 = 1;

					BBCMode = 2;
				}

				if ( AtoVolts < TargetVolts )
				{
					if ( BuckDuty < MaxBuck )
					{
						++BuckDuty;
					}
					else
					{
						if ( 12 * TargetVolts > 10 * BatteryVoltage )
						{
							BBCNextMode = 1;
						}
					}
				}
				else if ( AtoVolts > TargetVolts )
				{
					if ( BuckDuty > MinBuck ) --BuckDuty;
					else BuckDuty = 0;
				}

				if (	( AtoStatus == 0 || AtoStatus == 1 || AtoStatus == 5 )
					||	( !gFlags.firing && AtoProbeCount >= 12 ) )
				{
					if ( BuckDuty >= ProbeDuty ) BuckDuty = ProbeDuty;
				}

				PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, BuckDuty );
			}
			break;

		case 3:	// Boost mode
			{
				if ( BBCMode != 3 )
				{
					BBC_Configure( BBC_PWMCH_BOOST, 1 );
					BoostDuty = MinBoost;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, BoostDuty );
					if ( !ISVTCDUAL ) PC3 = 1;
                                
					BBC_Configure( BBC_PWMCH_BUCK, 1 );
					BuckDuty = MaxBuck;
					PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, BuckDuty );
					PC1 = 1;

					BBCMode = 3;

					bd = 0;
				}

				if ( AtoVolts < TargetVolts && BoostDuty > MaxBoost )
				{
					if ( gFlags.firing && ISMODEVW(dfMode) )
					{
						if ( !bd )
						{
							bd = BoostDuty;
							av = AtoVolts;
						}

						if (( BoostDuty + BoostWindow >= bd ) || ( AtoVolts > av + 2 ))
						{
							--BoostDuty;

							if ( AtoVolts > av + 2 )
							{
								bd = BoostDuty;
								av = AtoVolts;
							}
						}
					}
					else
					{
						--BoostDuty;
					}
				}
				else if ( AtoVolts > TargetVolts )
				{
					if ( BoostDuty < MinBoost )
					{
						++BoostDuty;
					}
					else
					{
						BBCNextMode = 1;
					}
				}

                                    PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, BoostDuty );
                                
			}
			break;

		default:
			break;
	}
}


//=========================================================================
//----- (00002EBC) --------------------------------------------------------
__myevic__ void AtoWarmUp()
{
	//uint32_t WUC;

	BBCNextMode = 2;
	BBCMode = 0;

	// Theorical:
	// ProbeDuty > 3124 * PWMCycles / AtoShuntRez / NumBatteries / 250
	// to read rez > 500000Ω if no atomizer (simplified).
	ProbeDuty = PWMCycles / NumBatteries / 8;
//	// For real-time atomizer measure accuracy (10 bits).
//	if ( AtoProbeCount >= 12 ) ProbeDuty <<= 1;

	//WUC = 0;
	//WarmUpCounter = ( !gFlags.pwm_pll || NumBatteries > 1 ) ? 2000 : 3000;
        WarmUpCounter = 1<<12; //5000;

	// Loop time around 19us on atomizer probing
	//  and around 26.4us (37.86kHz) on firing.
	do
	{
		if ( !gFlags.probing_ato && !gFlags.firing )
			break;

		//if ( gFlags.firing || !gFlags.pwm_pll || !WUC || ( WUC - WarmUpCounter >= 20 ) )
		//{
		//	WUC = WarmUpCounter;
			RegulateBuckBoost();
		//}

		ReadAtoCurrent();

		if ( AtoVolts == TargetVolts )
			break;

		if (( AtoStatus == 0 || AtoStatus == 1 || AtoStatus == 5 
                        || ( !gFlags.firing && AtoProbeCount >= 12 ))
			&& BuckDuty >= ProbeDuty )
		{
			break;
		}

		if ( ISMODEBY(dfMode) && BBCMode == 1 ) break;

	}
	while ( WarmUpCounter );
}


//=========================================================================
//----- (000031DC) --------------------------------------------------------
__myevic__ uint16_t AtoPower( uint16_t volts )
{
	uint32_t pwr;

	pwr = volts * volts / AtoRezMilli;
	if ( !pwr ) pwr = 1;
	return (uint16_t)pwr;
}


//=========================================================================
//----- (000031F4) --------------------------------------------------------
__myevic__ uint16_t GetVoltsForPower( uint16_t pwr )
{
	uint16_t v;

	v = sqrtul( (uint32_t)pwr * AtoRezMilli );

	if ( v > MaxVolts ) v = MaxVolts;

	return v;
}


//=========================================================================
//----- (00003198) --------------------------------------------------------
__myevic__ void TweakTargetVoltsVW()
{ // 1000 Hz
	unsigned int pwr;
        int pc = 0;
        //int poverscale_flag = 0;
        
	if ( dfMode == 6 )
	{
		pwr = dfSavedCfgPwr[ConfigIndex];
	}
	else
	{
		pwr = dfPower;
	}

	if ( dfStatus.pcurve && !CurveDelay )
	{
		int t;
                
                if ( dfCurveRepeatTimer && CurveRepeatTimerDuration > dfCurveRepeatTimer ) //10 = 1sec
                    CurveRepeatTimerDuration = 0;
                        
		for ( t = 0 ; t < PWR_CURVE_PTS-1 ; ++t )
			//if (( dfPwrCurve[t+1].time > FireDuration ) || ( dfPwrCurve[t+1].time == 0 ))
                        if (( dfPwrCurve[t+1].time > CurveRepeatTimerDuration ) || ( dfPwrCurve[t+1].time == 0 ))
				break;

		int p = dfPwrCurve[t].power;
		//if ( p > 100 && CurveDelay ) p = 100;

                    pwr = p * pwr / 100;
                
                //if ( !CurveDelay ) 
                    pc = 1;
	}

        if ( dfStatus.preheat && PreheatTimer )
	{
		pwr = PreheatPower;
                pc = 1;
	}
        
	pwr = AtoPowerLimit( pwr );

	//if ( dfStatus2.pwrlow && gFlags.limit_power )
        //    poverscale_flag = 1;
                                            
        if ( dfStatus.vvlite && !pc && !gFlags.decrease_voltage ) 
            //&& !( dfStatus.keylock && dfStatus2.replay ) )
        {
            if ( !dfVVLockedVolt ) dfVVLockedVolt = VWVolts;
            TargetVolts = dfVVLockedVolt;
        }
        else
        {
            TargetVolts = GetVoltsForPower( pwr * PowerScale / 100 );
        }
}


//=========================================================================
//----- (00003658) --------------------------------------------------------

__myevic__ void TweakTargetVoltsJT()
{
	unsigned int pwr;
	unsigned int volts;
	unsigned int temp;

	if ( TargetVolts )
	{
		pwr = dfTCPower;

		//if ( pwr < 10 ) pwr = 10;

		pwr = AtoPowerLimit( pwr );

		volts = GetVoltsForPower( pwr );

		temp = ( dfStatus.IsCelsius ) ? CelsiusToF( dfTemp ) : dfTemp;

		if ( gFlags.decrease_voltage )
		{
			if ( TargetVolts ) --TargetVolts;
		}
		else if ( AtoTemp < temp )
		{
			++TargetVolts;
			gFlags.limit_ato_temp = 0;
		}
		else if ( AtoTemp > temp )
		{
			if ( gFlags.limit_ato_temp )
			{
				gFlags.limit_ato_temp = 0;
				TargetVolts = 0;
				if ( !ISVTCDUAL && !ISINVOKE && !ISIKU200 ) PC3 = 0;
				PC1 = 0;
				BuckDuty = 0;
				PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, 0 );
                                
				BoostDuty = 0;
                                
                            if ( !ISINVOKE && !ISIKU200 )
                            {                                
				PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, 0 );
                            }
                                
			}
			else
			{
				if ( TargetVolts ) --TargetVolts;
			}
		}

		if ( TargetVolts > volts ) TargetVolts = volts;
	}
}

//=========================================================================

__myevic__ void TweakTargetVoltsReplay()
{
	unsigned int pwr;
	unsigned int volts;

                if ( AtoRezMilli < 5 ) 
                {                    
                    Event = 4; //keylock state change event
                    AtoRezMilliMin = 0; //AtoRezMilli;
                    StopFire();
                    return;
                }
            
		pwr = AtoPowerLimit( dfReplayPower ); //dfPower , dfMaxPower - preheat 
		volts = GetVoltsForPower( pwr );
                //volts = GetAtoVWVolts( pwr, ReplayRez / 10 );
                //ReplayRez = dfReplayRez * 10 + dfReplayMillis;
                        
                if ( AtoRezMilli > ReplayRez )
		{
                    if ( TargetVolts ) --TargetVolts;
		}
                else if ( AtoRezMilli < ReplayRez )
                {
                    ++TargetVolts;
                }

                if ( !gFlags.decrease_voltage )
                {
                    if ( TargetVolts > volts ) TargetVolts = volts;
                }
                else
                {
                    TargetVolts = GetVoltsForPower( pwr * PowerScale / 100 );
                }
}


//=========================================================================
//----- (00008F90) --------------------------------------------------------
__myevic__ void SetMinMaxPower()
{
	unsigned int u;

	if ( AtoError || !AtoRez )
	{
		AtoMinPower = 10;
		AtoMaxPower = MaxPower;
	}
	else
	{
		u = MaxCurrent * AtoRez;
		if ( u > MaxVolts ) u = MaxVolts;
		AtoMaxPower = (( u * u ) / AtoRez + 5 ) / 10;
		AtoMinPower = 2500 / AtoRez / 10;
		ClampAtoPowers();
	}
}


//=========================================================================
//----- (00009000) --------------------------------------------------------
__myevic__ void SetMinMaxVolts()
{
	if ( AtoError || !AtoRez )
	{
		AtoMinVolts = 50;
		AtoMaxVolts = MaxVolts;
	}
	else
	{
		AtoMinVolts = sqrtul( 10ul * 10 * AtoRez );
		AtoMaxVolts = sqrtul( 10ul * MaxPower * AtoRez );
		ClampAtoVolts();
	}
}


//=========================================================================
void RoundPowers()
{
	dfPower -= dfPower % WattsInc;
	dfTCPower -= dfTCPower % WattsInc;
	dfPreheatPwr -= dfPreheatPwr % WattsInc;
}


//=========================================================================
// SMART mode resistances values
//JoyeTech BF coils res 0.25(TC NI),  0.5, 0.6, 1, 1.5 Ohm
/*
const uint16_t SMARTRezValues[] =
		{	50, 60, 100, 150	};

// SMART mode power ranges
const uint16_t SMARTPowers[] =
		{	250, 300,
			200, 280,
			180, 250,
			160, 200,
			200, 750	};
*/


//=========================================================================
//----- (00008F5C) --------------------------------------------------------
/*
__myevic__ int SearchSMARTRez( uint16_t rez )
{
	int i;
	uint16_t r;

	i = 0;
	while ( 1 )
	{
		r = SMARTRezValues[i];
		if ( ( r + r / 10 >= rez ) && ( r - r / 10 <= rez ) ) break;
		if ( ++i == 4 ) break;
	}
	return i;
}
*/


//=========================================================================
//----- (000038FC) --------------------------------------------------------
__myevic__ void SetAtoSMARTParams()
{
	int i, j, k;

	uint16_t r;
	//uint16_t p;

	for ( i = 9 ; i > 0 ; --i )
	{
		r = dfSavedCfgRez[i];

		if ( r )
		{
			if ( 	(( r - r / 10 ) <= AtoRez || r - 1 <= AtoRez )
				&& 	(( r + r / 10 ) >= AtoRez || r + 1 >= AtoRez ) )
			{
				break; // new
			}
		}
	}

	if ( i > 0 ) //choose exist
	{
		ConfigIndex = i;
                
/*
		p = dfSavedCfgPwr[i];
		if ( p > MaxPower || p < 10 )
		{
                                //search BF coils
				j = SearchSMARTRez( r );
				dfSavedCfgPwr[i] = SMARTPowers[ j << 1 ];
                                
                                if ( !dfSavedCfgPwr[i] )
                                {
                                    dfSavedCfgPwr[i] = dfPower;
                                }
*/
                
/*
		if ( dfSavedCfgRez[9] )
		{
			// Most recently used at end of list
			r = dfSavedCfgRez[9];
			p = dfSavedCfgPwr[9];
			dfSavedCfgRez[9] = dfSavedCfgRez[i];
			dfSavedCfgPwr[9] = dfSavedCfgPwr[i];
			dfSavedCfgRez[i] = r;
			dfSavedCfgPwr[i] = p;
			ConfigIndex = 9;
		}
*/
	}
	else //set new
	{
		for ( j = 0 ; j < 10 ; ++j )
		{
			if ( !dfSavedCfgRez[j] ) break;
		}

		if ( j == 10 )
		{
			ConfigIndex = 9;

			for ( k = 0 ; k < 9 ; ++k )
			{
				dfSavedCfgRez[k] = dfSavedCfgRez[k+1];
				dfSavedCfgPwr[k] = dfSavedCfgPwr[k+1];
			}
		}
		else
		{
			ConfigIndex = j;
		}

		dfSavedCfgRez[ConfigIndex] = AtoRez;
                dfSavedCfgPwr[ConfigIndex] = dfPower;
		//dfSavedCfgPwr[ConfigIndex] = SMARTPowers[ SearchSMARTRez( AtoRez ) << 1 ];
	}
        
	UpdateDFTimer = 50;
}


//=========================================================================
//----- (000090DC) --------------------------------------------------------
__myevic__ void SetAtoLimits()
{
	uint16_t pwr;

	SetMinMaxVolts();
	SetMinMaxPower();

	if ( dfPower > MaxPower ) dfPower = MaxPower;
	if ( dfTCPower > MaxTCPower ) dfTCPower = MaxTCPower;

	if ( !AtoError && AtoRez )
	{
                if ( Set_NewRez_dfRez ) //no change ConfigIndex while vaping
                {
                        SetAtoSMARTParams();
                }
            
		if ( dfMode == 6 )
                {
                        //if ( Set_NewRez_dfRez ) //no change ConfigIndex while vaping
                        //{
                        //    SetAtoSMARTParams();
                        //}
                
			pwr = dfSavedCfgPwr[ConfigIndex];
                }
		else
                {
			pwr = dfPower;
                }
                
		if ( pwr < AtoMinPower ) pwr = AtoMinPower;
		if ( pwr > AtoMaxPower ) pwr = AtoMaxPower;

                    VWVolts = GetAtoVWVolts( pwr, AtoRez );
                    dfVVLockedVolt = GetAtoVWVolts( pwr, dfResistance );

		if ( dfMode == 6 ) //SMART
			//dfPower = ClampPower( dfVWVolts, 1 );
                        //dfSavedCfgPwr[ConfigIndex] = ClampPower( dfVWVolts, 1 );
                        dfSavedCfgPwr[ConfigIndex] = pwr;
		else
			dfPower = pwr;
                        //dfPower = ClampPower( dfVWVolts, 1 );
                        
	}
}


//=========================================================================
//----- (00006038) --------------------------------------------------------
__myevic__ void ProbeAtomizer()
{
    // from ResetResistance(), eh() on fire_button, Main() 3 times, Menus() 2
    
	if ( 
               ( ISVTCDUAL && ( BatteryStatus == 2 || !PA3 ) )
            || ( ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 ) && ( BatteryStatus == 2 || !PF0 ) )
            || ( ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISINVOKE ) && ( BatteryStatus == 2 || !PD1 ) )
            || ( ( ISRX2 || ISRX217 || ISGEN2 ) && ( BatteryStatus == 2 || !PF2 ) ) 
            || ( ( ISSINFJ200 || ISIKU200 ) && BatteryStatus == 2 )
            )
        {
		AtoStatus = 0;   //no ato (no batt || !pin)
	}
	else //ISSINP80
	{
		SetADCState( 1, 1 );
		SetADCState( 2, 1 );
		if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			SetADCState( 15, 1 );
		}
		WaitOnTMR2( 2 );

/*
 		if (( AtoProbeCount == 8 ) || ( gFlags.firing ))
		{
			if ( gFlags.firing )
			{
				gFlags.limit_ato_temp = 1;
			}
			TargetVolts = GetVoltsForPower( 50 );
			if ( !TargetVolts ) TargetVolts = 100;
		}

		else if ( AtoProbeCount == 9 )
		{
			TargetVolts = GetVoltsForPower( 100 );
		}
		else if ( AtoProbeCount == 10 )
		{
			TargetVolts = GetVoltsForPower( 150 );
		}
		else
		{
			TargetVolts = 100;
		}

		if ( TargetVolts > 600 ) TargetVolts = 600;
*/

/*
			if ( gFlags.firing )
			{
				gFlags.limit_ato_temp = 1;
			
                                //TargetVolts = GetVoltsForPower( 50 );                               
                                if ( !TargetVolts )  TargetVolts = 100;
                        }
                        else 
                        {
                                TargetVolts = 100;     
                        }
*/
                
                if ( gFlags.firing )
			gFlags.limit_ato_temp = 1;
                
                
                if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 || AtoProbeCount < 8 )
                    TargetVolts = 100;
                else
                    TargetVolts = GetVoltsForPower( 100 ); //not for rx23
                
                if ( Screen == 0 || Screen == 60 || Screen == 5 )
                {
                    TargetVolts = 50;
                }

                if ( !TargetVolts ) TargetVolts = 100;
                if ( TargetVolts > 600 ) TargetVolts = 600;
                
                //if ( gFlags.pbank ) TargetVolts = 500;  

		gFlags.probing_ato = 1;
		AtoWarmUp();
		WaitOnTMR2( 2 );
		ReadAtomizer();
		gFlags.probing_ato = 0;

		if ( !gFlags.firing )
		{
			PC1 = 0;
			if ( !ISVTCDUAL && !ISINVOKE && !ISIKU200 ) PC3 = 0;
                        
			SetADCState( 1, 0 );
			SetADCState( 2, 0 );
			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 0 );
			}
                        
			BoostDuty = 0;
                        
                    if ( !ISINVOKE && !ISIKU200 )
                    {                        
			PWM_SET_CMR( PWM0, BBC_PWMCH_BOOST, 0 );
                    }
                        
			BuckDuty = 0;
			PWM_SET_CMR( PWM0, BBC_PWMCH_BUCK, 0 );
		}
	}

//myprintf("AtoStatus=%d\n", AtoStatus);
        
        //AtoStatus;	0,1,2,3,4,5 = Open,Short,Low,Large,Ok,overCurrent
        //AtoError;     0,1,2,3 = Ok,Open/Large,Short,Low
	switch ( AtoStatus )
	{
		case 0:
		case 3:
			AtoError = 1; //no ato
			break;
		case 1:
                case 5:    
			AtoError = 2; //short
			break;
		case 2:
			AtoError = 3; //low
			break;
		case 4:
		default:
			AtoError = 0; //Ok
			break;
	}

//	myprintf( "Probe: %2d. ARM=%d AS=%d\n", AtoProbeCount, AtoRezMilli, AtoStatus );

	if ( AtoProbeCount < 12 )
	{
		++AtoProbeCount;
	}
       
	if ( AtoStatus == 4 )
	{
                //if ( AtoProbeCount < 11 )
         
		AtoRez = AtoRezMilli / 10;
		AtoMillis = AtoRezMilli % 10;
                   
                if ( AtoProbeCount != 11 )
                    return;
                
                if ( !ISMODETC(dfMode) )
                {
                    if ( !dfStatus2.vwrezlock && dfResistance > AtoRez ) dfResistance = AtoRez; // real goes down to cold (may be in TC too?)
                }
	}
	else
	{
		AtoRez = 0;
		AtoMillis = 0;
		if ( AtoStatus == 0 ) AtoProbeCount = 0;
	}
        
        //if ( gFlags.pbank ) return; //Set_NewRez_dfRez = 0;
/*
	if ( AtoError == LastAtoError
			&& ( AtoRez + AtoRez / 20 ) >= LastAtoRez // /20
			&& ( AtoRez - AtoRez / 20 ) <= LastAtoRez )
	{
		AtoRez = LastAtoRez;
		AtoMillis = LastAtoMillis;
	}
*/
	if ( AtoError == LastAtoError
			&& ( AtoRez > LastAtoRez )
			&& ( AtoRez <= ( LastAtoRez + 3 ) ) )
	{
		AtoRez = LastAtoRez;
		AtoMillis = LastAtoMillis;
	}
        
	if ( AtoRez != LastAtoRez || AtoError != LastAtoError )
	{
		if ( !LastAtoRez )
		{
			Set_NewRez_dfRez = 1;
		}               
		LastAtoRez = AtoRez;
		LastAtoMillis = AtoMillis;
		LastAtoError = AtoError;
                
                SetAtoLimits();
                
		if ( Screen != 2 || gFlags.firing ) gFlags.refresh_display = 1; //no refresh while scr2 delay
		//ScreenDuration = GetMainScreenDuration();
        }


	if ( Set_NewRez_dfRez )
            
            // ato mount
            
            //1 if LastAtoRez=0
            //2 from ResetResistance()
            //1 InitVariables()
            //2 SleepIfIdle()
            //2 shunt change
	{          
		if ( Set_NewRez_dfRez == 2 ) //was: reset rez, sleep, shunt changed
		{
			Set_NewRez_dfRez = 1;
		}
                
		if ( !dfResistance )
		{                               
			if ( AtoRez )
			{                             
                                dfResistance = AtoRez;
				RezMillis = AtoMillis;
                                UpdateDFTimer = 50;                               
			}
		}
                            
                if ( dfStatus2.autoprofile ) SetProfile(); //auto set Profile for new atom
                
		gFlags.new_rez_ni = 1;
		gFlags.new_rez_ti = 1;
		gFlags.new_rez_ss = 1;
		gFlags.new_rez_tcr = 1;

		if ( !dfStatus.chkmodeoff ) gFlags.check_mode = 1;
                
                SetAtoLimits(); //for Volt setting fix on smart profile changes
	}
}

//=========================================================================
__myevic__ uint16_t RereadRez()
{   
    // Reread dfResistance
    // for SwitchRezLock
    
    //ISMODETC(m) ((m)<=3)
    //ISMODEVW(m) (((m)==4)||((m)==6))
    //ISMODEBY(m) ((m)==5)

        //if ( AtoRez )
        //{
		dfResistance = AtoRezMilli / 10; //=real //AtoRez;
		RezMillis = AtoMillis;
		//*prez = dfResistance;
                
                if ( ISMODETC(dfMode) )
                {
                    dfMillis &= ~( 0xf << ( dfMode << 2 ) );
                    dfMillis |= RezMillis << ( dfMode << 2 );
                }
                else
                {
                    VWVolts = GetAtoVWVolts( dfPower, dfResistance ); //AtoRez
                    dfVVLockedVolt = VWVolts;
                }
        //}
        return dfResistance;
}

//=========================================================================
//----- (000088B4) --------------------------------------------------------

/*
__myevic__ void _SwitchRezLock( uint8_t plock, uint16_t *prez )
{

    if ( AtoRez )
    {
        *plock ^= 1;
        *prez = RereadRez();
    }
*/
        
/*
        if ( AtoRez )
        {
		dfResistance = AtoRezMilli / 10; //AtoRez;
		RezMillis = AtoMillis;

		*prez = dfResistance;
		dfMillis &= ~( 0xf << ( dfMode << 2 ) );
		dfMillis |= RezMillis << ( dfMode << 2 );   
        }
*/
//}


/*
__myevic__ void SetRez()
{
    	switch ( dfMode )
	{
		case 0:
			dfRezNI = RereadRez();
			break;

		case 1:
			dfRezTI = RereadRez();
			break;

		case 2:
			dfRezSS = RereadRez();
			break;

		case 3:
			dfRezTCR = RereadRez();
			break;
                        
                case 4: //pwr re-read rez in VW too
                case 5: //bypass
                        
                    if ( AtoRez )
                    {    
                        AtoRez = RereadRez();
                    }
                        break;

		default:
			break;
	}
}
*/


__myevic__ void SwitchRezLock( int lock )
{      
	switch ( dfMode )
	{
		case 0:
                    dfRezNI = RereadRez();
                    if ( lock ) dfStatus2.dfRezLockedNI ^= 1;
                    //_SwitchRezLock( dfStatus2.dfRezLockedNI, &dfRezNI );
                    break;

		case 1:
                    dfRezTI = RereadRez();
                    if ( lock ) dfStatus2.dfRezLockedTI ^= 1;
                    break;

		case 2:
                    dfRezSS = RereadRez();
                    if ( lock ) dfStatus2.dfRezLockedSS ^= 1;
                    break;

		case 3:
                    dfRezTCR = RereadRez();
                    if ( lock ) dfStatus2.dfRezLockedTCR ^= 1;
                    break;
                        
                case 4: //pwr re-read rez in VW too
                case 5: //bypass
                        
                    if ( AtoRez )
                    {    
                        if ( lock ) 
                        {
                            dfStatus2.vwrezlock ^= 1;
                        }
                        
                        AtoRez = RereadRez();
                        dfRezVW = AtoRez;
                    }
                        
/*                       
                        //AtoRez = AtoRezMilli / 10;
                        //dfResistance = AtoRez;
                        dfResistance = AtoRezMilli / 10;
                        
                        dfVWVolts = GetAtoVWVolts( dfPower, dfResistance ); //AtoRez
                        dfVVLockedVolt = dfVWVolts;
			RezMillis = AtoMillis;
*/
                        UpdateDFTimer = 50;

                        break;

		default:
			break;
	}
}

//=========================================================================

const uint32_t BoardTempTable[] =
	{	34800, 26670, 20620, 16070, 12630, 10000,
		7976, 6407, 5182, 4218, 3455, 2847, 2360,
		1967, 1648, 1388, 1175, 999, 853, 732, 630	}; //21


//=========================================================================
//----- (00007BE0) --------------------------------------------------------
__myevic__ void ReadBoardTemp()
{
	unsigned int sample;
	unsigned int tdr;
	int v3;
	int v4;
	int v6;
	int v7;

	static uint32_t BTempSample;
	static uint8_t	BTempSampleCnt;


	while ( BTempSampleCnt++ < 16 )
	{
		if ( ISRX300 )
		{
			BTempSample += ADC_Read( 17 );
		}
		else
		{
			BTempSample += ADC_Read( 14 );
		}

		if ( !gFlags.sample_btemp )
			return;
	}

	gFlags.sample_btemp = 0;

	sample = BTempSample >> 4;

        uint16_t tConst;
        
        if ( ISRX23 ) 
            tConst = 5024;
        else 
            tConst = 5280;
        
	if ( ISRX300 )
	{
		sample = 2560 * 1000 * sample >> 12;

		if ( sample >= 748000 )
		{
			BoardTemp = 0;
		}
		else
		{
			BoardTemp = ( 748000 - sample ) / 1672;

			if ( BoardTemp > 10 )
			{
				BoardTemp -= 5;
			}
		}
	}
	else
	{
		BoardTemp = 0;

		if ( sample != 4096 && sample > 100 ) //4095
		{
			if ( NumBatteries == 1 )
			{
				uint16_t v;

				if ( BatteryVoltage <= 360 )
				{
					v = BatteryVoltage - 33;
				}
				else
				{
					v = 320;
				}
				tdr = ( 20000 * sample / ( 16 * v - sample )); // why?
			}
			else
			{
				tdr = ( 20000 * sample / ( tConst - sample ));
			}

			if ( tdr <= 630 )
			{
				BoardTemp = 99;
			}
			else if ( tdr < 34800 )
			{
				for ( v3 = 1 ; v3 < 20 ; ++v3 )
				{
					if ( BoardTempTable[v3] <= tdr )
						break;
				}

				v4 = BoardTempTable[v3];
				v6 = (( 10 * BoardTempTable[v3 - 1] - 10 * v4 ) / 5 );
				v7 = 0;
				do
				{
					if ( v6 * v7 + 10 * v4 <= 10 * tdr && (v7 + 1) * v6 + 10 * v4 > 10 * tdr )
						break;
					++v7;
				}
				while ( v7 < 5 );

				BoardTemp = 5 * v3 - v7;
			}
		}
	}

        if ( BoardTemp) BoardTemp += dfBoardTempCorr;
	BTempSampleCnt = 0;
	BTempSample = 0;
}

const uint32_t AkkuTempTable[] =
	{	641800, 494500, 383000, 299400, 236000, 187500,
		150000, 120500, 97460, 79250, 64800, 53270, 44070,
		36640, 30610, 25700, 21650 }; //17

//=========================================================================
__myevic__ void ReadAkkuTemp()
{
	unsigned int sample;
	unsigned int tdr;
	int v3;
	int v4;
	int v6;
	int v7;

	static uint32_t ATempSample;
	static uint8_t	ATempSampleCnt;


	while ( ATempSampleCnt++ < 16 )
	{
		ATempSample += ADC_Read( 4 );

		if ( !gFlags.sample_atemp )
			return;
	}

	gFlags.sample_atemp = 0;

	sample = ATempSample >> 4;

        uint16_t tConst;
        
        tConst = 5280;
        
        AkkuTemp = 0;

	if ( sample < 4095 && sample > 100 )
	{
			tdr = ( 200000 * sample / ( tConst - sample ));

			if ( tdr <= 21650 )
			{
				AkkuTemp = 70; //99;
			}
			else if ( tdr < 641800 )
			{
				for ( v3 = 1 ; v3 < 16 ; ++v3 )
				{
					if ( AkkuTempTable[v3] <= tdr )
						break;
				}

				v4 = AkkuTempTable[v3];
				v6 = (( 10 * AkkuTempTable[v3 - 1] - 10 * v4 ) / 5 );
				v7 = 0;
                                
				do
				{
					if ( v6 * v7 + 10 * v4 <= 10 * tdr && (v7 + 1) * v6 + 10 * v4 > 10 * tdr )
						break;
					++v7;
				}
				while ( v7 < 5 );

				AkkuTemp = 5 * v3 - v7;
			}
	}

        if ( AkkuTemp) AkkuTemp += dfAkkuTempCorr;
	ATempSampleCnt = 0;
	ATempSample = 0;
}

//=========================================================================
//----- (00006004) --------------------------------------------------------
__myevic__ void Overtemp()
{
	StopFire();
        
        if ( !gFlags.FireNotFlipped )
        {
                    gFlags.FireNotFlipped = 1;
                    dfStatus.flipped ^= 1;
                    InitDisplay();
        }
        
        SetScreen( 29, 5 );
	//gFlags.refresh_display = 1;
	//Screen = 29;
	//ScreenDuration = 5;
	KeyPressTime |= 0x8000;
}

__myevic__ void ProbeAtoSeries()
{
	while ( AtoStatus == 4 && AtoProbeCount < 11 ) //12
	{
		ProbeAtomizer();
		WaitOnTMR2( 10 );
	}
}

//=========================================================================
// Force re-read of the resistance
//-------------------------------------------------------------------------
__myevic__ void ResetResistance()
{
        //full reread
	dfResistance = 0;
	AtoRezMilli = 0;
	AtoRez = 0;
	Set_NewRez_dfRez = 2; //reset flag
	AtoProbeCount = 0;
	AtoStatus = 4;

        ProbeAtoSeries();
/*
	while ( AtoStatus == 4 && AtoProbeCount < 11 ) //12
	{
		ProbeAtomizer();
		WaitOnTMR2( 10 );
	}
*/
                                
        AtoRez = AtoRezMilli / 10;
        AtoMillis = AtoRezMilli % 10;
        dfResistance = AtoRez;
                
	RezMillis = AtoMillis;
				
        LastAtoRez = AtoRez;
        LastAtoMillis = AtoMillis;     
                                
        UpdateDFTimer = 50;
}


//=========================================================================
// Algo
//-------------------------------------------------------------------------

static filter_t	TempFilter;
//static uint8_t TCBoost;

typedef struct algostage
{
	int dec;
	int inc;
	int (*cond)();
	int counter;
	int tests;
}
algostage_t;

typedef struct algoctl
{
	algostage_t *stages;
	algostage_t *stage;
	uint8_t		nstages;
	uint8_t		nstage;
	uint8_t		nbtests;
	uint8_t		boost;
	uint16_t	ttemp;
	uint16_t	atemp;
	uint16_t	mvolts;
	uint16_t	power;
	uint32_t	counter;
	int32_t		error;
	int32_t		integ;
	uint8_t		start;
}
algoctl_t;

static algoctl_t AlgoCtl;


int condTrue()
{
	return 1;
}

int condFalse()
{
	return 0;
}

// Simulate Joyetech with segments

int condCross1()
{
	return ( AlgoCtl.atemp > AlgoCtl.ttemp );
}

int condCross2()
{
	return (( AlgoCtl.atemp < AlgoCtl.ttemp ) && ( AlgoCtl.stage->counter > 100 ));
}

int condFinal()
{
	return 0;
}

algostage_t tabStagesSim[]=
{
	{ 10, 10, condCross1 }, // climb
	{ 10, 10, condCross2 }, // drop
	{ 10, 10, condFinal  }, // regulate
};

algostage_t tabStagesSweet[]=
{
	{ 10, 10, condCross1 }, // climb
	{ 10, 10, condCross2 }, // drop
	{  2,  2, condFinal  }, // regulate
};

// Auto algo

int condBoostEnd()
{
        return (( AlgoCtl.atemp > AlgoCtl.ttemp * AlgoCtl.boost / 100 ) || gFlags.decrease_voltage );
}

int condNearCross1()
{
	return ( AlgoCtl.atemp > AlgoCtl.ttemp - 60 );
}

int condNearCross2()
{
	return (( AlgoCtl.atemp < AlgoCtl.ttemp + 5 )
	//	&&	( AlgoCtl.atemp > AlgoCtl.ttemp - 5 )
		&&	( AlgoCtl.stage->counter > 100 ));
}

algostage_t tabStagesSegments[]=
{
	/* 0 */ {  3, 30, condTrue,      }, // calibration (not yet used)
	/* 1 */ {  3, 30, condBoostEnd,  }, // boost
	/* 2 */ {  3,  5, condNearCross1 }, // climb
	/* 3 */ {  10, 3, condCross1     }, // nearCross1
	/* 4 */ {  5,  3, condNearCross2 }, // drop
	/* 5 */ {  2,  3, condCross2     }, // nearCross2
	/* 6 */ {  2,  2, condFinal      }, // regulate
};

//-------------------------------------------------------------------------

__myevic__ void InitTCAlgo()
{
	//AlgoCtl.start = 0; //not used
	AlgoCtl.boost = dfTCBoost;
	AlgoCtl.ttemp = dfStatus.IsCelsius ? dfTemp : FarenheitToC( dfTemp );

	AlgoCtl.counter = 0;

	switch ( dfTCAlgo )
	{
		case TCALGO_BOOST:
			//TCBoost = dfTCBoost;
			AlgoCtl.nstages = sizeof( tabStagesSegments ) / sizeof( algostage_t );
			AlgoCtl.stages = tabStagesSegments;
			AlgoCtl.power = dfTCPower;
			AlgoCtl.nbtests = 3;
			break;

		case TCALGO_SWEET:
			AlgoCtl.nstages = sizeof( tabStagesSweet ) / sizeof( algostage_t );
			AlgoCtl.stages = tabStagesSweet;
			AlgoCtl.power = dfTCPower;
			AlgoCtl.nbtests = 3;
			break;

		case TCALGO_PID:
			ReadAtoTemp();
			AlgoCtl.atemp = FarenheitToC( AtoTemp );
			AlgoCtl.error = AlgoCtl.ttemp - AlgoCtl.atemp;
			AlgoCtl.integ = 0;
			break;

		case TCALGO_JOY:
		default:
                        //AlgoCtl not used
			//AlgoCtl.nstages = sizeof( tabStagesSegments ) / sizeof( algostage_t );
			//AlgoCtl.stages = tabStagesSim;
			//AlgoCtl.power = dfTCPower;
			//AlgoCtl.nbtests = 1;
			break;
	}

	AlgoCtl.nstage = 0;

	if ( AlgoCtl.nstages )
	{
		AlgoCtl.stage = &AlgoCtl.stages[0];

		for ( int i = 0 ; i < AlgoCtl.nstages ; ++i )
		{
			AlgoCtl.stages[i].counter = 0;
			AlgoCtl.stages[i].tests = 0;
		}
	}

	AlgoCtl.mvolts = TargetVolts * 10;

	InitFilter( &TempFilter );
}


/*
__myevic__ void CheckModeSeg()
{
    //for TC
	unsigned int pwr;
	unsigned int volts;

	pwr = dfTCPower;

	if ( pwr < 10 ) pwr = 10;

	if ( pwr > 400 ) pwr = 400;
	if ( pwr < 300 ) pwr = 300;

	pwr = AtoPowerLimit( pwr );

	if ( FireDuration <= 2 )
	{
		if ( pwr > 300 ) pwr = 300;
	}

	volts = GetVoltsForPower( pwr );

	TargetVolts = volts;

	return;
}
*/


__myevic__ void TweakTargetVoltsSegments()
{
	unsigned int pwr;
	unsigned int volts;

	pwr = AlgoCtl.power;

	if ( pwr < 10 ) pwr = 10;

	pwr   = AtoPowerLimit( pwr );
	volts = GetVoltsForPower( pwr );

	AlgoCtl.atemp = FarenheitToC( AtoTemp );

	if ( AlgoCtl.nstage < AlgoCtl.nstages - 1 )
	{
		AlgoCtl.atemp = FilterWMean( &TempFilter, AlgoCtl.atemp );
	}

	if ( AlgoCtl.stage->cond() )
	{
		if ( ++AlgoCtl.stage->tests >= AlgoCtl.nbtests )
		{
			AlgoCtl.stage = &AlgoCtl.stages[++AlgoCtl.nstage];
		}
	}
	else
	{
		AlgoCtl.stage->tests = 0;
	}

	if ( gFlags.decrease_voltage )
	{
		if ( TargetVolts )
		{
			AlgoCtl.mvolts -= 10;
			TargetVolts = AlgoCtl.mvolts / 10;
		}
	}
	else if ( AlgoCtl.atemp < AlgoCtl.ttemp )
	{
		AlgoCtl.mvolts += AlgoCtl.stage->inc;
		TargetVolts = AlgoCtl.mvolts / 10;
	}
	else if ( AlgoCtl.atemp > AlgoCtl.ttemp )
	{
		AlgoCtl.mvolts -= AlgoCtl.stage->dec;
		TargetVolts = AlgoCtl.mvolts / 10;
	}

	if ( TargetVolts > volts )
	{
		TargetVolts = volts;
		AlgoCtl.mvolts = TargetVolts * 10;
	}

	++AlgoCtl.stage->counter;
	++AlgoCtl.counter;
}


__myevic__ void TweakTargetVoltsPID()
{
	int32_t pwr;
	int32_t error;
	int32_t ediff;
	uint32_t volts;

	++AlgoCtl.counter;

	AtoTemp = FilterWMean( &TempFilter, AtoTemp );

	// 50Hz refresh
	if ( AlgoCtl.counter % 20 )
		return;

	AlgoCtl.atemp = FarenheitToC( AtoTemp );

	error = AlgoCtl.ttemp - AlgoCtl.atemp;
	ediff = error - AlgoCtl.error;

	AlgoCtl.error = error;
	AlgoCtl.integ += error;

	pwr = AlgoCtl.error * dfPID.P
		+ AlgoCtl.integ * dfPID.I / 50
		+ ediff * dfPID.D * 50;

	pwr /= 100;

	if ( pwr < 10 ) pwr = 10;
	pwr = AtoPowerLimit( pwr );
	if ( pwr > dfTCPower ) pwr = dfTCPower;
	if ( pwr > BatteryMaxPwr ) pwr = BatteryMaxPwr;

	volts = GetVoltsForPower( pwr );

	if ( volts < 50 ) volts = 50;

	if ( gFlags.decrease_voltage )
	{
		if ( volts < TargetVolts ) TargetVolts = volts;
		else if ( TargetVolts ) --TargetVolts;
	}
	else
	{
		TargetVolts = volts;
	}
}


__myevic__ void TweakTargetVoltsTC()
{
	if ( !TargetVolts )
		return;

/*
	if ( gFlags.check_mode )
	{
		CheckModeSeg();
		return;
	}
*/

	switch ( dfTCAlgo )
	{
		case TCALGO_SWEET:
		case TCALGO_BOOST:
			TweakTargetVoltsSegments();
			break;

		case TCALGO_PID:
			TweakTargetVoltsPID();
			break;

		default:
		case TCALGO_JOY:
			TweakTargetVoltsJT();
			break;
	}
}

__myevic__ int GetLockState()
{
        
        int lock = 0;
	if 	( dfMode == 0 ) lock = dfStatus2.dfRezLockedNI; // 1 : 0'
	else if ( dfMode == 1 ) lock = dfStatus2.dfRezLockedTI;
	else if ( dfMode == 2 ) lock = dfStatus2.dfRezLockedSS;
	else if ( dfMode == 3 ) lock = dfStatus2.dfRezLockedTCR;
        else if ( dfStatus2.vwrezlock ) lock = 1; //dfMode == 4 && 
        return lock;
        
}