#include "myevic.h"
#include "atomizer.h"
#include "display.h"
#include "battery.h"
#include "screens.h"
#include "events.h"
#include "myrtc.h"
#include "timers.h"
#include "dataflash.h"
#include "meadc.h"
#include "megpio.h"
#include "meusbd.h"
#include "miscs.h"
#include "flappy.h"
#include "tetris.h"


//=========================================================================
// Globals

volatile gFlags_t gFlags;
uint8_t BoxModel;

time_t startwatch;

//const uint8_t  MaxBoardTemp = 70;

//=========================================================================
// Additional initialisations
//-------------------------------------------------------------------------
/*
__myevic__ void CustomStartup()
{

	if ( 0 ) // EADC test
	{
		uint32_t s1, s2, s3;

		SetADCState( 0, 1 );
		SetADCState( 4, 1 );

		do
		{
			ClearScreenBuffer();

			CLK_SysTickDelay( 10 );
			s3 = ADC_Read( 4 );

			CLK_SysTickDelay( 10 );
			s1 = ADC_Read( 18 );

			CLK_SysTickDelay( 10 );
			s2 = ADC_Read( 0 );

			DrawValue( 8,  0, s1, 0, 0x29, 4 );
			DrawValue( 8, 20, s2, 0, 0x29, 4 );
			DrawValue( 8, 40, s3, 0, 0x29, 4 );

			DisplayRefresh();

			WaitOnTMR2( 1000 );
		}
		while ( PD3 );
	}


	if ( 0 ) //Timer test 1
	{
		TIMER_Stop( TIMER3 );
		TIMER_Close( TIMER3 );

		MemClear( gPlayfield.uc, 256 );

		CLK_SetModuleClock( TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_LIRC, 0 );

		gPlayfield.ul[1] =
		TIMER_Open( TIMER3, TIMER_PERIODIC_MODE, 10 );
		TIMER_EnableInt( TIMER3 );
		TIMER_Start( TIMER3 );
	}

	if ( 0 ) // Timer test 2
	{
		TIMER_Close( TIMER2 );
		TIMER_Close( TIMER3 );

		MemClear( gPlayfield.uc, 256 );

		CLK_SetModuleClock( TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HXT, 0 );
		CLK_SetModuleClock( TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_LIRC, 0 );

		CLK_EnableModuleClock( TMR2_MODULE );
		CLK_EnableModuleClock( TMR3_MODULE );

		__set_PRIMASK(1);

		TIMER3->CTL |= TIMER_CTL_RSTCNT_Msk;
		TIMER2->CTL |= TIMER_CTL_RSTCNT_Msk;

		TIMER3->CMP  = 1000;

		TIMER3->CTL  = TIMER_CTL_CNTEN_Msk | TIMER_ONESHOT_MODE;
		TIMER2->CTL  = TIMER_CTL_CNTEN_Msk | TIMER_CONTINUOUS_MODE;
		while(!(TIMER3->INTSTS & TIMER_INTSTS_TIF_Msk));
		TIMER2->CTL = 0;

		gPlayfield.ul[0] = TIMER2->CNT;

		__set_PRIMASK(0);

		TIMER_Close( TIMER2 );
		TIMER_Close( TIMER3 );

		CLK_SetModuleClock( TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0 );
		CLK_EnableModuleClock( TMR2_MODULE );
		TIMER_Open( TIMER2, TIMER_PERIODIC_MODE, 1000 );
		TIMER_EnableInt( TIMER2 );
		TIMER_Start( TIMER2 );

		CLK_SetModuleClock( TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HXT, 0 );
		CLK_EnableModuleClock( TMR3_MODULE );
		TIMER_Open( TIMER3, TIMER_PERIODIC_MODE, 10 );
		TIMER_EnableInt( TIMER3 );
		TIMER_Start( TIMER3 );
	}

	return;
}
*/


//=========================================================================
//----- (0000652C) --------------------------------------------------------
void InitDevices()
{
	SYS_UnlockReg();

	// Internal 22.1184MHz oscillator
	CLK_EnableXtalRC( CLK_PWRCTL_HIRCEN_Msk );
	CLK_WaitClockReady( CLK_STATUS_HIRCSTB_Msk );
        // HCLK clock source: HIRC, HCLK source divider: 1
	CLK_SetHCLK( CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK( 1 ) );
	
        // LIRC clock (internal RC 10kHz)
	//	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	//	CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
        
	// 12.000MHz external crystal
	CLK_EnableXtalRC( CLK_PWRCTL_HXTEN_Msk );
	CLK_WaitClockReady( CLK_STATUS_HXTSTB_Msk );

	// FMC Frequency Optimisation mode <= 72MHz
	FMC_EnableFreqOptimizeMode( FMC_FTCTL_OPTIMIZE_72MHZ );

	// Setup PLL to 144MHz and HCLK source to PLL/2
	CLK_SetCoreClock( CPU_FREQ );
CLK_WaitClockReady( CLK_STATUS_PLLSTB_Msk );

	// UART0 CLK = HXT/1
	//#if (ENABLE_UART)
	//CLK_EnableModuleClock( UART0_MODULE );
	//CLK_SetModuleClock( UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART( 1 ) );
	//#endif

	// USB CLK = PLL/3 (48MHz)
	CLK_EnableModuleClock( USBD_MODULE );
	CLK_SetModuleClock( USBD_MODULE, 0, CLK_CLKDIV0_USB( 3 ) );
        // Enable USB 3.3V LDO
	SYS->USBPHY = SYS_USBPHY_LDO33EN_Msk;

	// WDT CLK = LIRC/1
	CLK_EnableModuleClock( WDT_MODULE );
	CLK_SetModuleClock( WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0 );

	// SPI0 CLK = PCLK0/1
        //CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK0, 0);
	CLK_EnableModuleClock( SPI0_MODULE );

	// EADC CLK = PCLK1/8 (9MHz)
	CLK_EnableModuleClock( EADC_MODULE );
	CLK_SetModuleClock( EADC_MODULE, 0, CLK_CLKDIV0_EADC( 8 ) );

	// CRC CLK = HCLK/1
	CLK_EnableModuleClock( CRC_MODULE );

	// TIMERS CLOCKS
	CLK_EnableModuleClock( TMR0_MODULE );
	CLK_EnableModuleClock( TMR1_MODULE );
	CLK_EnableModuleClock( TMR2_MODULE );
	CLK_EnableModuleClock( TMR3_MODULE );
	CLK_SetModuleClock( TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0 );
	CLK_SetModuleClock( TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_PCLK0, 0 );
	CLK_SetModuleClock( TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0 );
	CLK_SetModuleClock( TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HXT, 0 );

	// Enable battery voltage sampling by ADC
	SYS->IVSCTL |= SYS_IVSCTL_VBATUGEN_Msk;

	// ADC reference voltage
	SYS->VREFCTL = SYS_VREFCTL_VREF_2_56V;

	// Brown-out detector; interrupts under 2.2V
	SYS_EnableBOD( SYS_BODCTL_BOD_RST_EN, SYS_BODCTL_BODVL_2_2V );

	// Update clock data
	SystemCoreClockUpdate();

	WDT_Open( WDT_TIMEOUT_2POW18, WDT_RESET_DELAY_18CLK, TRUE, FALSE );

	SYS_LockReg();
}


//=========================================================================
//----- (00000C48) --------------------------------------------------------
__myevic__ void InitHardware()
{
	SYS_UnlockReg();

	//  32.768kHz external crystal
	if ( dfStatus.x32off )
	{
		CLK_DisableXtalRC( CLK_PWRCTL_LXTEN_Msk );
	}
	else
	{
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF0MFP_Msk|SYS_GPF_MFPL_PF1MFP_Msk);
		SYS->GPF_MFPL |=  (SYS_GPF_MFPL_PF0MFP_X32_OUT|SYS_GPF_MFPL_PF1MFP_X32_IN);

		CLK_EnableXtalRC( CLK_PWRCTL_LXTEN_Msk );
		CLK_WaitClockReady( CLK_STATUS_LXTSTB_Msk );
	}

	SetPWMClock();
	SYS_LockReg();
	InitGPIO();

	//if ( !PD3 )
	//{
	//	gFlags.noclock = 1;
	//}

	InitSPI0();
	InitEADC();
	InitPWM();
	InitTimers();
	InitUSB();
}


//=========================================================================
// Real-Time Clock
//-------------------------------------------------------------------------
__myevic__ void InitRTC()
{
	if ( !gFlags.noclock )
	{
		gFlags.has_x32 = !dfStatus.x32off;
		RTCStart( 0 );
	}

	if ( gFlags.has_x32 )
	{
		// Disable Light Sleep mode.
		dfStatus.lsloff = 1;
		UpdateDFTimer = 1;
	}
	else if ( !dfStatus.x32off )
	{
		// Disable X32
		dfStatus.x32off = 1;
		// Enable Light Sleep mode.
		dfStatus.lsloff = 0;
		UpdateDFTimer = 1;
	}

	gFlags.rtcinit = 1;

//RTCGetEpoch( &startwatch );
        
        
//	time_t vvbase;

//	vvbase = RTCReadRegister( RTCSPARE_VV_BASE );

//	if ( ( vvbase == 0 ) || ( vvbase % 86400 ) )
//	{
//		vvbase = RTCGetEpoch( 0 );
//		vvbase -= vvbase % 86400;
//		RTCWriteRegister( RTCSPARE_VV_BASE, vvbase );
//		RTCWriteRegister( RTCSPARE_VV_MJOULES, 0 );
//                RTCWriteRegister( RTCSPARE_VV_MJOULESDAY, 0 );
//	}
//	else
//	{
		//MilliJoules = RTCReadRegister( RTCSPARE_VV_MJOULES );
                //MilliJoulesDay = RTCReadRegister( RTCSPARE_VV_MJOULESDAY );
//	}
        if ( !MilliJoules ) MilliJoules = dfJoules;
        if ( !MilliJoulesDay ) MilliJoulesDay = dfJoulesDay;
        if ( !MilliJoulesEnergy ) MilliJoulesEnergy = dfJoulesEnergy;
        
        MilliJoulesVapedOn = MilliJoules;
        
        RTCWriteNextMidnight();
        
}


//=========================================================================
//----- (0000895C) --------------------------------------------------------
__myevic__ void InitVariables()
{
    //call after restart and profile changes
    
	//InitDataFlash(); //in main
	LEDGetColor();
	KeyPressTime |= 0x8000;
	LastInputs |= 0x80;
	Set_NewRez_dfRez = 1;
	gFlags.draw_edited_item = 1;
	gFlags.refresh_battery = 1;
	gFlags.read_battery = 1;
	gFlags.read_bir = 1;
	WattsInc = dfStatus.onewatt ? 10 : 1;
	RoundPowers();
	AtoMinVolts = 50;
	AtoMaxVolts = MaxVolts;
	AtoMinPower = 10;
	AtoMaxPower = MaxPower;
	//Object3D = 1;	
        //if ( !dfMaxBoardTemp ) dfMaxBoardTemp = 70;
        
        //dfUIVersion = 1;
        //dfStatus.sme = 1; // DFMagicNumber = 0xDA;
        gFlags.FireNotFlipped = 1;
        gFlags.screen_on = 1;
        //if ( !dfColdLockTemp ) dfColdLockTemp = 20;
        //if ( !dfNewRezPerc ) dfNewRezPerc = 5;
        AtoTemp = CelsiusToF( dfColdLockTemp ); //70;
        
        //if ( dfVapeDelayTimer > 3600 ) dfVapeDelayTimer = 0; //not need if reset df
        if ( dfStatus2.vapedelay && !VapeDelayTimer ) VapeDelayTimer = dfVapeDelayTimer;
        
        VWVolts = 330;
                
        if ( !dfReplayRez ) 
            dfStatus2.replay = 0;
        else
            ReplayRez = dfReplayRez * 10 + dfReplayMillis;
        
        //gFlags.wake_up = 1;
        
        //AwakeTimer = 0;
        
        //test
        //gFlags.pbank = 1;
        
        //dfAwakeTimer = 20; //test 20 sec alive
        
        //dfVWTempAlgo = 3; //SS test
}


//=========================================================================
// BSOD

/*__myevic__ void Plantouille( int xpsr, int* stack )
{

	int i, k;

	k = 0;

	SYS_UnlockReg();
	WDT_Close();
	SYS_LockReg();

	InitDisplay();

	while ( 1 )
	{
		ClearScreenBuffer();

		DrawImage( 0, 0, 'X'+0x27 );
		DrawHexLong( 16, 0, xpsr, 0 );

		DrawHexDigit( 0, 16, k );

		for ( i = 0; i < 14 ; ++i )
		{
			DrawHexLong( 16, 16+i*8, stack[i+k*14], 0 );
		}

		DisplayRefresh();

		while ( !PE0 || !PD2 || !PD3 )
			CLK_SysTickDelay( 10000 );

		while ( PE0 && PD2 && PD3 )
			CLK_SysTickDelay( 10000 );

		if ( !PE0 )
		{
		  SYS_UnlockReg();
		  SYS_ResetChip();
		  while ( 1 )
			;
		}

		if ( !PD2 ) ++k;
		if ( !PD3 ) --k;

		if ( k < 0 ) k = 0;
		else if ( k > 15 ) k = 15;
	}

}
*/

//=========================================================================
//----- (00005D24) --------------------------------------------------------
__myevic__ void DevicesOnOff( int off )
{
	if ( off )
	{
		TIMER_DisableInt( TIMER0 );
		TIMER_DisableInt( TIMER1 );
		TIMER_DisableInt( TIMER2 );

		if ( !gFlags.light_sleep )
		{
			TIMER_DisableInt( TIMER3 );
		}

		EADC_Close( EADC );
                
		SetADCState( 1, 0 );
		SetADCState( 2, 0 );
                
                if ( ISRX300 )
                    SetADCState( 17, 0 );
                else
                    SetADCState( 14, 0 ); //ISSINP80

		if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISPRIMO1 
                        || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISINVOKE || ISRX2 
                        || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
		{

                        SetADCState( 3, 0 );
                        SetADCState( 13, 0 );
                    
			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 0 );
			}

                        if ( ISSINFJ200 || ISIKU200 )
                        {
                                SetADCState( 4, 0 ); //akku temp
                        }
                        
			PD7 = 0;                                                // 0x400048DC
			BBC_Configure( BBC_PWMCH_CHARGER, 0 );                  // 5 0
			PD7 = 0;

			if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 )
			{
				PF2 = 0;                                        // 0x40004948
			}
		}

		PC1 = 0;                                                        // 40004884
		PC0 = 0;                                                        // 40004880
		BBC_Configure( BBC_PWMCH_BUCK, 0 );                             // 0 0
                
		if ( !ISVTCDUAL && !ISINVOKE ) 
                {
                    //ISIKU200
                    PC3 = 0;                                                    // 4000488C
                }
                
                if ( !ISINVOKE )
                {
                    //ISIKU200
                    PC2 = 0;                                                        // 40004888
                }
                
                if ( !ISINVOKE && !ISIKU200 )
                {
                    BBC_Configure( BBC_PWMCH_BOOST, 0 );                        // 2 0
                }
                
                if ( ISIKU200 )
                {
                    PA2 = 0;
                    BBC_Configure( 3, 0 );  //need for charge
                    PA2 = 0;
                }
                
		if ( ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF1 = 0;                                                // 40004944
		}
		else if ( ISRX300 )
		{
			PD1 = 0;                                                // 400048C4
		}
                else if ( ISSINFJ200 )
                {
                    	PA1 = 0;                                                // 0x400048DC
			//? BBC_Configure( 4, 0 );                                  // 4 0
			PA1 = 0;
                        PA2 = 0;
                }                
                else if ( ISRX2 || ISRX217 || ISGEN2 )
		{
                        PA3 = 0;
                }
		else if ( !ISSINP80 )
		{
			PB7 = 0;                                                // 4000485C
		}

                if ( !ISRX2 && !ISSINP80 && !ISINVOKE && !ISRX217 && !ISGEN2 )
		{ 
                    //i200
                    if ( ISIKU200 ) PE13 = 0;
                    GPIO_DisableInt( PD, 0 );
                    PD0 = 0;                                                        // 400048C0
                    GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );        // 400040C0 1 1
                }
                
		if ( ISRX300 || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 
                        || ISINVOKE || ISSINFJ200 || ISRX217 || ISGEN2 )
		{
                    if ( ISRX300 || ISRX2 || ISSINFJ200 || ISRX217 || ISGEN2 )
                    {
			PF5 = 0;                                                // 40004954
			PF6 = 0;                                                // 40004958
                    }   
                    else if ( ISGEN3 )
                    {
                        PF5 = 0;
                    }
                    
                    if ( !ISRX2 && !ISSINFJ200 && !ISRX217 && !ISGEN2 )
                    {
			PA3 = 0;                                                // 4000480C
			PA2 = 0;                                                // 40004808
                    }
                    //ISIKU200  PC2 = 0; PC3 = 0 ^^
		}
                
		if ( ISVTCDUAL )
		{
			GPIO_DisableInt( PD, 1 );
			PD1 = 0;                                                // 400048C4
			GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
		}
		else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 
                        && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISRX2 
                        && !ISINVOKE && !ISSINFJ200 && !ISRX217 && !ISGEN2 && ! ISIKU200 )
		{
			GPIO_DisableInt( PD, 7 ); //ISSINP80
			PD7 = 0;                                                // 400048DC
			GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
		}

		SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE11MFP_Msk|SYS_GPE_MFPH_PE12MFP_Msk|SYS_GPE_MFPH_PE13MFP_Msk);
		SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE11MFP_GPIO|SYS_GPE_MFPH_PE12MFP_GPIO|SYS_GPE_MFPH_PE13MFP_GPIO);
                
		PE11 = 0;                                                       // 4000492C
		GPIO_SetMode( PE, GPIO_PIN_PIN11_Msk, GPIO_MODE_OUTPUT );
		PE12 = 0;                                                       // 40004930
		GPIO_SetMode( PE, GPIO_PIN_PIN12_Msk, GPIO_MODE_OUTPUT );
		PE13 = 0;                                                       // 40004934
		GPIO_SetMode( PE, GPIO_PIN_PIN13_Msk, GPIO_MODE_OUTPUT );
		PE10 = 0;                                                       // 40004928

		GPIO_EnableInt( PE, 0, GPIO_INT_BOTH_EDGE );                    // 4100
		GPIO_EnableInt( PD, 2, GPIO_INT_BOTH_EDGE );                    // 40C0
		GPIO_EnableInt( PD, 3, GPIO_INT_BOTH_EDGE );                    // 40C0

		if ( ISVTCDUAL )
		{
			PA3 = 0;                                                // 480C
			PC3 = 0;                                                // 488C
			PF2 = 0;                                                // 4948
			PA2 = 0;                                                // 4808
		}
		else if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			PF0 = 0;                                                // 4940
		}
                else if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISINVOKE )
                {
                        PD1 = 0;                                                // 48C4
                }
                else if ( ISGEN3 )
                {
                        PF0 = 0;                                                // 4940
                        PD1 = 0;                                                // 48C4
                }
                else if ( ISRX2 || ISRX217 || ISGEN2 )
                {
                        PF2 = 0;                                                // 4948
                }
                
		SYS_UnlockReg();
		SYS->USBPHY &= ~SYS_USBPHY_LDO33EN_Msk;
		SYS->IVSCTL &= ~(SYS_IVSCTL_VBATUGEN_Msk|SYS_IVSCTL_VTEMPEN_Msk);
		SYS_DisableBOD();
		SYS->VREFCTL = 0;
		SYS_LockReg();

		USBD_CLR_INT_FLAG( USBD_INTSTS_WAKEUP|USBD_INTSTS_FLDET|USBD_INTSTS_BUS|USBD_INTSTS_USB );
		USBD_ENABLE_INT( USBD_INT_WAKEUP );
	}
	else //Device On
	{
		USBD_CLR_INT_FLAG( USBD_INTSTS_WAKEUP );

		SYS_UnlockReg();
		SYS->USBPHY |= SYS_USBPHY_LDO33EN_Msk;
		SYS->IVSCTL |= SYS_IVSCTL_VBATUGEN_Msk;
                               
		if ( ISRX300 )
		{
			SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;
		}
                
		SYS->VREFCTL = SYS_VREFCTL_VREF_2_56V;
		SYS_EnableBOD( SYS_BODCTL_BOD_RST_EN, SYS_BODCTL_BODVL_2_2V );
		SYS_LockReg();

		GPIO_DisableInt( PE, 0 );                                       // 4100
		GPIO_DisableInt( PD, 2 );                                       // 40C0
		GPIO_DisableInt( PD, 3 );                                       // 40C0

		if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF2 = 1;                                                // 4948
		}

		SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE11MFP_Msk|SYS_GPE_MFPH_PE12MFP_Msk|SYS_GPE_MFPH_PE13MFP_Msk);
		SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE11MFP_SPI0_MOSI0|SYS_GPE_MFPH_PE12MFP_SPI0_SS|SYS_GPE_MFPH_PE13MFP_SPI0_CLK);

                if ( !ISRX2 && !ISSINP80 && !ISINVOKE && !ISRX217 && !ISGEN2 )
                {
                        GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk, GPIO_MODE_INPUT );         // 40C0 1 0
                        GPIO_EnableInt( PD, 0, GPIO_INT_FALLING );                      // 40C0 0 1
                }
                
		if ( ISVTCDUAL )
		{
			GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_INPUT ); // 40C0
			GPIO_EnableInt( PD, 1, GPIO_INT_RISING );
			GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN1_Msk );
		}
		else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 
                        && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISRX2 
                        && !ISINVOKE && !ISSINFJ200 && !ISRX217 && !ISGEN2 && !ISIKU200 )
		{
			GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_INPUT ); // 40C0 0x80 0     ISSINP80
			GPIO_EnableInt( PD, 7, GPIO_INT_RISING );               // 40C0 7 0x10000
			GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN7_Msk );
		}

		if ( ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF1 = 1;                                                // 4944
		}
		else if ( ISRX300 )
		{
			PD1 = 1;                                                // 48C4
		}
		else if ( ISRX2 || ISRX217 || ISGEN2 )
		{
			PA3 = 1;                                                // 48C4
		}
                else if ( ISSINFJ200 )
                {
                        PA2 = 1;
                }
		else if ( !ISSINP80 )
		{
			PB7 = 1;                                                // 485C
		}

                if ( ISIKU200 ) 
                    PE13 = 1;
                
		SetADCState( 1, 1 );
		SetADCState( 2, 1 );
                
                if ( ISRX300 )
                    SetADCState( 17, 1 );
                else
                    SetADCState( 14, 1 );

		if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 
                        || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 || ISINVOKE 
                        || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
		{
			SetADCState( 3, 1 );
			SetADCState( 13, 1 );

			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 1 );
			}
                        else if ( ISSINFJ200 || ISIKU200 )
                        {
                                SetADCState( 4, 1 );
                        }
		}

		TIMER_EnableInt( TIMER0 );
		TIMER_EnableInt( TIMER1 );
		TIMER_EnableInt( TIMER2 );
		TIMER_EnableInt( TIMER3 );
	}
}


//=========================================================================
__myevic__ void LightSleep()
{
	// Switch Core Clock to HXT/3 (4MHz)
	CLK_SetHCLK( CLK_CLKSEL0_HCLKSEL_HXT, CLK_CLKDIV0_HCLK( 3 ) );

	// Update clock data
	SystemCoreClockUpdate();

	// Switch off the PLL Clock & the HIRC
	CLK_DisablePLL();
	CLK_DisableXtalRC( CLK_PWRCTL_HIRCEN_Msk );

	// Disable Clocks of Modules using HCLK/HXT or LIRC
	CLK_DisableModuleClock( PWM0_MODULE );
	CLK_DisableModuleClock( SPI0_MODULE );
	CLK_DisableModuleClock( CRC_MODULE );

        //CLK_EnableModuleClock( USBD_MODULE );
        
	gFlags.wake_up = 0;
        
	do
	{
		CLK_Idle();
	}
	while ( !gFlags.wake_up );

	// Wake up the HIRC
	CLK_EnableXtalRC( CLK_PWRCTL_HIRCEN_Msk );
	CLK_WaitClockReady( CLK_STATUS_HIRCSTB_Msk );

	// Wake up the PLL
	CLK_SetCoreClock( CPU_FREQ );
CLK_WaitClockReady( CLK_STATUS_PLLSTB_Msk );

	// Update clock data
	SystemCoreClockUpdate();

	// Wake up Modules

	CLK_EnableModuleClock( PWM0_MODULE );
	CLK_EnableModuleClock( SPI0_MODULE );
	CLK_EnableModuleClock( CRC_MODULE );
}


//=========================================================================
//----- (00005D14) --------------------------------------------------------
__myevic__ void FlushAndSleep()
{
    
	if ( !gFlags.light_sleep )
	{
		CLK_PowerDown();
	}
	else
	{
		LightSleep();
	}
}


//=========================================================================
//----- (00004F0C) --------------------------------------------------------

void GoToSleep()
{
	gFlags.light_sleep = !( gFlags.has_x32 
                || dfStatus.lsloff || gFlags.noclock );
        
        if ( VapeDelayTimer ) gFlags.light_sleep = 1;

        //if ( gFlags.power_down ) gFlags.light_sleep = 0;
            
	ScreenOff();
	LEDOff();
	gFlags.firing = 0;
	BatReadTimer = 50;
	RTCSleep();
	DevicesOnOff( 1 ); //off
	CLK_SysTickDelay( 250 );
	CLK_SysTickDelay( 250 );
	CLK_SysTickDelay( 250 );
	if ( dfStatus.off || PE0 || KeyPressTime == 1100 )
	{
		SYS_UnlockReg();
		WDT_Close();
		FlushAndSleep();
		PreheatDelay = 0;
                CurveDelay = 0;
	}       
	WDT_Open( WDT_TIMEOUT_2POW14, WDT_RESET_DELAY_18CLK, TRUE, FALSE );
	SYS_LockReg();
	gFlags.refresh_battery = 1;
        if ( dfStatus.invert ) gFlags.inverse = 1;
        
        //gFlags.power_down = 0;
                                        
	DevicesOnOff( 0 ); //on
	RTCWakeUp();
	InitDisplay();

}


//=========================================================================
//----- (0000782C) --------------------------------------------------------
__myevic__ void SleepIfIdle()
{
	if ( !gFlags.firing && !NoEventTimer )
	{
            if ( SleepTimer == 0 && gFlags.user_idle )
            {
                if ( Screen == 5 || Screen == 0 )
                {
                    //reset on charge too
                    PuffsOffCount = 0;
                    NextPreheatTimer = 0;
                    NextPreheatPower = 0;
                    AutoPuffTimer = 0;
                    AwakeTimer = 0;
                    MilliJoulesVapedOn = MilliJoules; //for last Session vaped
                    SessionPuffs = 0;
                    EditModeTimer = 0;
                    gFlags.apuff = 0;
                }
                
		if ( Screen == 0 )
		{                                        
			GoToSleep();
                        
			Set_NewRez_dfRez = 2; //we need recheck res after wakeup? yes 
			AtoProbeCount = 0;
			AtoRezMilli = 0;
                        //PuffsOffCount = 0;
                        //NextPreheatTimer = 0;
                        //AutoPuffTimer = 0;
                        //AwakeTimer = 0;
                        //MilliJoulesVapedOn = MilliJoules;
                        //SessionPuffs = 0;
                        //gFlags.apuff = 0;
			gFlags.sample_vbat = 1;
			ReadBatteryVoltage();
                        
			if ( dfDimOffMode == 1 || ( ( BatteryVoltage <= BatteryCutOff + 20 ) && !gFlags.usb_attached ) )
			{
                                SwitchOffCase = 4; //from sleep 
				dfStatus.off = 1;
				//Screen = 0;
                                //LEDOff(); ?
                                
			}
                        else if ( dfDimOffMode == 2 && !dfStatus.off )
                        {
                                dfStatus.keylock = 1;
                        }
                        
			gFlags.sample_btemp = 1;
                        
                        if ( ISSINFJ200 || ISIKU200 )
                            gFlags.sample_atemp = 1;
                        
                        
                        gFlags.asleep = 1;
		}
            }    
   
            NoEventTimer = 200; //2s
	}
}


//=========================================================================
// Monitoring
//-------------------------------------------------------------------------
/*
__myevic__ void Monitor()
{
	if ( gFlags.firing )
	{
		myprintf( "FIRING "
					"RESM=%d BATT=%d VOUT=%d CUR=%d",
					AtoRezMilli,
					RTBattVolts,
					AtoVolts,
					AtoCurrent
				);

		if ( ISMODETC(dfMode) )
		{
			myprintf( " SPWR=%d RPWR=%d CELS=%d STEMP=%d RTEMP=%d\n",
						dfTCPower,
						AtoPower( AtoVolts ),
						dfIsCelsius ? 1 : 0,
						dfTemp,
						dfIsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp
					);
		}
		else if ( ISMODEBY(dfMode) )
		{
			myprintf( " RPWR=%d\n",
						AtoPower( AtoVolts )
					);
		}
		else
		{
			myprintf( " SPWR=%d RPWR=%d\n",
						dfPower,
						AtoPower( AtoVolts )
					);
		}
	}
	else
	{
		myprintf( "STANDBY "
					"BATT=%d CHG=%d BRD=%d ATO=%d "
					"RES=%d RESM=%d MODE=%d",
					BatteryVoltage,
					gFlags.battery_charging ? 1 : 0,
					dfIsCelsius ? BoardTemp : CelsiusToF( BoardTemp ),
					AtoStatus,
					AtoRez,
					AtoRezMilli,
					dfMode
				);

		if ( ISMODETC(dfMode) )
		{
			ReadAtoTemp();
			myprintf( " SPWR=%d CELS=%d STEMP=%d RTEMP=%d\n",
						dfTCPower,
						dfIsCelsius ? 1 : 0,
						dfTemp,
						dfIsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp
					);
		}
		else if ( ISMODEBY(dfMode) )
		{
			myprintf( "\n" );
		}
		else
		{
			myprintf( " SPWR=%d\n",
						dfPower
					);
		}
	}
}
*/

__myevic__ void ResetMJDay()
{
        time_t t, mn;
        RTCGetEpoch ( &t );
        //GetRTC( &rtd );
        //RTCTimeToEpoch( &t, &rtd );
        
        mn = RTCGetMidnightDate();

        if ( t >= mn )
        {
            //IsRTCAlarmINT = 0;
            MilliJoulesDay = 0;
            dfJoulesDay = 0;
            //RTCWriteRegister( RTCSPARE_VV_MJOULESDAY, 0 );
            UpdateDFTimer = 50;
           
            //SetRTC( &rtd );
            
            RTCWriteNextMidnight();
                        
            if ( dfStatus.puffday )
                    ResetPuffCounters();
        
        }
        
}

//=========================================================================
//----- (00000148) --------------------------------------------------------
__myevic__ void Main()
{
    //Init
    
	InitDevices();
	InitDataFlash();
	//LEDGetColor();
	InitVariables();

	// Enable chip temp sensor sampling by ADC
        //SetADCState( 17, 1 ); we can use build-in Temp sensor for all mods too
	if ( ISRX300 )
	{
		SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;
	}

	InitHardware();

	//myprintf( "\n\nJoyetech APROM\n" ); // need for identify FW file
	//myprintf( "CPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock );

	SetBatteryModel();

	gFlags.sample_vbat = 1;
	ReadBatteryVoltage();
        
	gFlags.sample_btemp = 1;
	ReadBoardTemp();
        
        if ( ISSINFJ200 || ISIKU200 )
        {
                gFlags.sample_atemp = 1;
                ReadAkkuTemp();
        }
        
        //if ( ISGEN3 ) WaitOnTMR2( 1500 );
        
	InitDisplay();
        
        if ( dfStatus2.splash0 || dfStatus2.splash1 ) //01 10 on
        {
            SplashTimer = 2;
        }
        
	MainView();

	//CustomStartup();

/*
	if ( !PD3 || !PE0 )
	{
		DrawScreen();
		while ( !PD3 || !PE0 )
			;
	}
*/
	if ( !PE0 ) //need for reset mod from menu
	{
		DrawScreen();
		while ( !PE0 )
			;
	}
        
	while ( 1 )
	{            
            	while ( gFlags.playing_fb || gFlags.playing_tt )
		{
                        if ( gFlags.playing_fb )
                                fbCallTimeouts();
                        else
                                ttCallTimeouts();
                            
			if ( gFlags.tick_100hz )
			{
				// 100Hz
				gFlags.tick_100hz = 0;
				ResetWatchDog();
				TimedItems();
				//SleepIfIdle();
				GetUserInput();
                                if ( !SleepTimer )
                                {
                                    if ( gFlags.playing_fb )
                                    {
                                        gFlags.playing_fb = 0;
					fbInitTimeouts();
                                    }
                                    else
                                    {
                                        gFlags.playing_tt = 0;
					ttInitTimeouts();
                                    }   
                                }
                                if ( !PE0 || !PD2 || !PD3 )
                                    SleepTimer = 3000; //30 sec
			}
                        
			if ( gFlags.tick_10hz )
			{
				// 10Hz
				gFlags.tick_10hz = 0;
				DataFlashUpdateTick();
			}
		}
                
		if ( gFlags.firing )
		{
			ReadAtoCurrent();
		}

		if ( gFlags.tick_5khz )
		{
			// 5000Hz
			gFlags.tick_5khz = 0;

			if ( gFlags.firing )
			{
				RegulateBuckBoost();
			}
		}

		if ( gFlags.tick_1khz )
		{
			// 1000Hz
			gFlags.tick_1khz = 0;

			if ( gFlags.firing )
			{
				ReadAtomizer();

            //if ( !gFlags.pbank ) 
            //{
				if ( ISMODETC(dfMode) )
				{
					if ( gFlags.check_mode )
					{
						CheckMode();
					}
					TweakTargetVoltsTC();
				}
				else if ( ISMODEVW(dfMode) )
				{
                                    if ( dfStatus.keylock && dfStatus2.replay && ReplayRez )  //dfMode == 4 only ?
                                    {
                                        TweakTargetVoltsReplay();
                                    }
                                    else
                                    {
                                        TweakTargetVoltsVW();
                                        
                                    }
				}
            //}
			}

			//if ( dfStatus.vcom )
			//{
			//	VCOM_Poll();
			//}
		}

		if ( gFlags.tick_100hz )
		{
			// 100Hz
			gFlags.tick_100hz = 0;
            
			ResetWatchDog();

			if ( gFlags.read_battery )
			{
				gFlags.read_battery = 0;
			}

			TimedItems();
			SleepIfIdle();
			ReadBatteryVoltage();
                        
			ReadBoardTemp();
                        if ( ISSINFJ200 || ISIKU200 )
                        {
                                ReadAkkuTemp();
                        }

			if ( 
                                gFlags.firing 
                                && ( BoardTemp > dfMaxBoardTemp
                                || ( ( ISSINFJ200 || ISIKU200 ) && AkkuTemp > 70 ) ) 
                            )
			{
				Overtemp();
			}

			if ( ISVTCDUAL )
			{
				BatteryChargeDual();
                                gFlags.soft_charge = 1;
			}
			else if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISPRIMO1 
                                || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 || ISINVOKE 
                                || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
			{
				BatteryCharge(); // <- 2-3 batts for rx
                                gFlags.soft_charge = 1;
			}
                        else
                        {
                            gFlags.soft_charge = 0;
                        }

			if (( dfStatus2.anim3d ) && ( Screen == 1 ) && ( !EditModeTimer ) 
                                && !HideLogo && !SplashTimer && !gFlags.toggleclock ) //&& ( dfMode != 6 )
			{
				anim3d(); //as logo
			}

			if ( ( Screen == 60 || Screen == 0 ) && !dfStatus.off ) 
                            //!dfStatus.off = main contrast in off state and fire click (clock)
			{
                                if ( gFlags.MainContrast ) 
                                {
                                    DisplaySetContrast( dfContrast2 );
                                    gFlags.MainContrast = 0;
                                }
                                
				if ( Screen == 60 && gFlags.animready )     
                                    AnimateScreenSaver();
			}
                                
			if ( gFlags.firing )
			{
				if ( gFlags.read_bir && ( FireDuration > 10 ) )
				{
					ReadInternalResistance();
				}

				if ( PreheatTimer && !--PreheatTimer )
				{
					uint16_t pwr;

					if ( dfMode == 6 )
					{
						pwr = dfSavedCfgPwr[ConfigIndex];
					}
					else
					{
						pwr = dfPower;
					}

					if ( pwr > BatteryMaxPwr )
					{
                //if ( gFlags.pbank ) //stop PBank
                                                    
						gFlags.limit_power = 1;
                                                if ( dfStatus2.pwrlow )
                                                    PowerScale = 100 * BatteryMaxPwr / pwr;

                                                pwr = BatteryMaxPwr;
					}
					else
					{
						gFlags.limit_power = 0;
						PowerScale = 100;
					}

				}
			}

			if ( KeyTicks >= 5 )
			{
				KeyRepeat();
			}

			GetUserInput();
		}

		if ( gFlags.tick_10hz )
		{
			// 10Hz
			gFlags.tick_10hz = 0;

			DataFlashUpdateTick();
			LEDTimerTick();
                        
                        if ( !dfStatus.off && gFlags.asleep ) 
                            gFlags.asleep = 0;
                        
                                                               
                        if ( !gFlags.MainContrast && Screen != 60 && Screen != 5 && Screen != 0 && !gFlags.firing ) //&& Screen != 2)
			{
                                DisplaySetContrast( dfContrast );
                                gFlags.MainContrast = 1;
                        }
                        
                        if (dfStatus.invert )
                        {
                            
                            int st = (dfStealthOn == 1) || !gFlags.screen_on; //|| ScreenDuration == 0 ;
                        
//myprintf( "st=%d dfSt=%d scrOn=%d Scr=%d inv=%d dur=%d\n",
//			st, dfStealthOn, gFlags.screen_on, Screen, gFlags.inverse, ScreenDuration );

                        
                            if  
                                ( 
                                       ( Screen == 0 || Screen == 5 )
                                    || ( st && !ScreenDuration ) // || ( Screen == 5 && st && !ScreenDuration ) 
                                    || ( Screen == 2 && st )

                                ) // no inverse
                                {
                                    if ( gFlags.inverse )
                                    {
                                        gFlags.refresh_display = 1;
                                        DisplaySetInverse( 0 );
                                        gFlags.inverse = 0;
                                    }
                                }
                            else
                                {
                                    if ( !gFlags.inverse )
                                    {
                                        gFlags.refresh_display = 1;
                                        DisplaySetInverse( 1 );
                                        gFlags.inverse = 1;
                                    }
                                }
                        }
                      
			if ( gFlags.firing )
                        {                                                                              
                                if ( dfStatus.fireflip && gFlags.FireNotFlipped && FireDuration > 2 )
                                {
                                    gFlags.FireNotFlipped = 0;
                                    dfStatus.flipped ^= 1;
                                    InitDisplay();
                                    gFlags.refresh_display = 1;
                                }

                                if ( gFlags.apuff && dfStatus.endlessfire && FireDuration >= dfProtec )
                                {
                                    dfTimeCount += FireDuration;
                                    FireDuration = 0;
                                }
                                else
                                {
                                    ++FireDuration;
                                }
                                                               
                                if ( dfStatus.pcurve ) ++CurveRepeatTimerDuration;
                                        
				//if ( gFlags.monitoring )
				//{
				//	Monitor();
				//}
			}
                        else if  ( dfStatus.fireflip && !gFlags.FireNotFlipped && Screen == 1 )
                        {
                                    gFlags.FireNotFlipped = 1;
                                    dfStatus.flipped ^= 1;
                                    InitDisplay();  
                                    gFlags.refresh_display = 1;                           
                        }
                        
			if ( ShowWeakBatFlag )
				--ShowWeakBatFlag;

			//if ( ShowProfNum )
			//	--ShowProfNum;

			if ( !( gFlags.firing && ISMODETC(dfMode) ) ) //!(A && B) = !A || !B  DeMorgans Law
			{
                                        DrawScreen();
			}

			if ( KeyTicks < 5 )
			{
				KeyRepeat();
			}
		}

		if ( gFlags.tick_5hz )
		{
			// 5Hz
			gFlags.tick_5hz = 0;

			if ( !gFlags.rtcinit && NumBatteries )
			{
				InitRTC();
			}
                              
			if ( gFlags.firing )
			{
				if ( TargetVolts == 0 )
				{
					ProbeAtomizer();
				}
			}
			else
			{
				if
				(	!dfStatus.off
					&& Event == 0
					&& ( AtoProbeCount < 11 ) // 12 - gives 2 proba pikes
					&& ( Screen == 0 || Screen == 1 || Screen == 2 || Screen == 5 || Screen == 60 )
                                )
				{
					ProbeAtomizer();
				}
			}

			if ( IsClockOnScreen() )
			{
				static uint8_t u8Seconds = 61;
				S_RTC_TIME_DATA_T rtd;

				GetRTC( &rtd );

				if ( (uint8_t)rtd.u32Second != u8Seconds )
				{
					u8Seconds = (uint8_t)rtd.u32Second;
					gFlags.refresh_display = 1;
				}
			}                        
		}

		if ( gFlags.tick_2hz )
		{
			// 2Hz
			gFlags.tick_2hz = 0;
                        
			gFlags.osc_1hz ^= 1;
//
                        if ( !gFlags.battery_charging && !gFlags.nbcr && dfStatus.nbrc && gFlags.rtcinit && ( BatteryVoltage > 250 ) &&  ( BatteryVoltage > dfBattVolt + 20 )) 
                        {
                                gFlags.nbcr = 1; //new buttery counters reset
                                dfBattVolt = BatteryVoltage;
                                ResetAllCounters();     
                        }
 
			if ( gFlags.firing )
                        {
				if ( ISMODETC(dfMode) )
				{
					DrawScreen();
				}
			}
			else
                        {
                            //if ( AtoError == 2 ) 
                            if 
                                ( 
                                ( AtoStatus == 1 || AtoStatus == 5 || AtoStatus == 2 ) //short overcurrent low
                                && !dfStatus.off
                                && Event == 0
                                && AtoProbeCount >= 11 //>= 12 //11 - for recheck rez after error, with ( AtoProbeCount < 11 ) above
                                )
                            {
                                AtoStatus = 4;
                                ProbeAtomizer();
                            }

                        }
			//{
/*
				if
				(	!dfStatus.off
					&& Event == 0
					&& ( AtoProbeCount >= 12 )
					&& ( Screen == 0 || Screen == 1 || Screen == 5 || Screen == 60 ) )
				{
					ProbeAtomizer();
				}
*/

				//if ( gFlags.monitoring )
				//{
				//	Monitor();
				//}
			//}
		}
                
		if ( gFlags.tick_1hz )
		{
			// 1Hz
			gFlags.tick_1hz = 0;
                        
                        if ( gFlags.rtcinit )
                        {
                                ResetMJDay();                               
                        }
                        
                        if ( !gFlags.firing ) AtoProbeCount = 10; //for quick res mesure in idle (lower - better but with fire multiclicks damage)
                                
			if ( SplashTimer )
			{
				--SplashTimer;
				
				if ( !SplashTimer )
				{
					MainView();
				}
			}

			if ( !gFlags.firing && !dfStatus.off && !EditModeTimer )
			{
				if ( HideLogo )
				{
					if ( Screen == 1 )
					{
						--HideLogo;

						if ( !HideLogo )
						{
							gFlags.refresh_display = 1;
						}
					}
				}
			}
                        
                        if ( dfStatus2.vapedelay && !VapeDelayTimer )
                        {
                                dfStatus2.vapedelay = 0;
                                UpdateDFTimer = 10;
                        }
                        
                        if ( dfAwakeTimer 
                                && !gFlags.asleep 
                                && !VapeDelayTimer && Screen != 5 ) //!gFlags.battery_charging ) //not in charg
                        {
                                if ( ++AwakeTimer >= dfAwakeTimer && Screen < 100 )
                                {
                                    VapeDelayTimer = dfVapeDelayTimer;
                                    dfStatus2.vapedelay = 1;
                                    SwitchOffCase = 2;
                                    Event = 17;
                                }
                        }                                               
		}

		EventHandler();

	}
}

