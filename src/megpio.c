#include "myevic.h"
#include "dataflash.h"
#include "events.h"
#include "atomizer.h"
#include "battery.h"


//=========================================================================
//----- (000022EC) --------------------------------------------------------
__myevic__ void GPD_IRQHandler()
{
	if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN7_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN7_Msk );
// usb detach?

		if ( gFlags.usb_attached )
		{
			BattProbeCount = 1;

			if ( gFlags.battery_charging )
			{
				Event = 13;
			}
		}
	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN1_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN1_Msk );

		if ( gFlags.usb_attached && ( NumBatteries == 1 ) && ( BatteryVoltage >= 414 ) ) //414
		{
			BattProbeCount = 1;

			if ( gFlags.battery_charging )
			{
				Event = 13; //Battery charge stop
			}
		}
	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN0_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN0_Msk );

		if ( ISPRESA75W || ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 
                        || ISRX300 || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISIKU200 )
		{
			if ( gFlags.firing || gFlags.probing_ato )
			{
				if ( Event != 28 )
				{
					Event = 28; //low battery
					StopFire();
				}
			}
		}
	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN2_Msk|GPIO_PIN_PIN3_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN2_Msk|GPIO_PIN_PIN3_Msk );
// +- buttons press
		if ( dfStatus.wakeonpm )
		{
			gFlags.wake_up = 1;
		}
	}
	else
	{
		PD->INTSRC = PD->INTSRC;
	}
}

//----- (00002334) --------------------------------------------------------
__myevic__ void GPE_IRQHandler()
{
	if ( GPIO_GET_INT_FLAG( PE, GPIO_PIN_PIN0_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PE, GPIO_PIN_PIN0_Msk );
//fire press
		gFlags.wake_up = 1;
	}
	else
	{
		PE->INTSRC = PE->INTSRC;
	}
}

//----- (00002342) --------------------------------------------------------
__myevic__ void GPF_IRQHandler()
{
	PF->INTSRC = PF->INTSRC;
}


//=========================================================================
//----- (00002384) --------------------------------------------------------
__myevic__ void InitGPIO()
{
	if ( ISVTCDUAL )
	{
		PA3 = 0;
		GPIO_SetMode( PA, GPIO_PIN_PIN3_Msk, GPIO_MODE_OUTPUT );            //pa 8 1                
	}
	else if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 )
	{
                //multi-function pins
		SYS->GPF_MFPL &= ~SYS_GPF_MFPL_PF0MFP_Msk;
		SYS->GPF_MFPL |= SYS_GPF_MFPL_PF0MFP_GPIO;
		PF0 = 0;
		GPIO_SetMode( PF, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );
	}
        else if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISINVOKE || ISSINFJ200 || ISIKU200 )
	{
            if ( !ISSINFJ200 && !ISIKU200 ) 
            {
                PD1 = 0;
                GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
            }
            else
            {
                GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_INPUT );
            }
	}
        else if ( ISRX2 || ISRX217 || ISGEN2 )
        {
                PF2 = 0;
                GPIO_SetMode( PF, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );        //pf 4 1                    
        }
        
        //in RX23: SYS->GPD_MFPL : ORR.W   R0, R0, #0x30
	// PD1 = Data transmitter output pin for UART0
	//#if (ENABLE_UART)
	//SYS->GPD_MFPL |= SYS_GPD_MFPL_PD1MFP_UART0_TXD;
	//#endif

        
//      MOV.W   R0, #0x40000000
//      LDR     R1, [R0,#0x48]
//      BIC.W   R1, R1, #0xFF
//      STR     R1, [R0,#0x48]
//      LDR     R1, [R0,#0x48]
//      STR     R1, [R0,#0x48]
	if ( ISRX300 || ISPRIMO1 || ISPRIMO2 || ISPRIMOMINI || ISPREDATOR 
                || ISPRIMOSE || ISFIT || ISGEN3 || ISINVOKE || ISSINP80 || ISRX2 
                || ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
	{
		SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD0MFP_Msk|SYS_GPD_MFPL_PD1MFP_Msk);
		SYS->GPD_MFPL |= SYS_GPD_MFPL_PD0MFP_GPIO|SYS_GPD_MFPL_PD1MFP_GPIO;
	}
        
	// PC0 = PWM0 CH0
	BBC_Configure( BBC_PWMCH_BUCK, 1 );     // 0 1
        
    if ( !ISINVOKE && !ISIKU200 )
    {
	// PC2 = PWM0 CH2
	BBC_Configure( BBC_PWMCH_BOOST, 1 );    // 2 1
    }
        
	if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || 
                ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISINVOKE || ISRX2 || 
                ISSINFJ200 || ISRX217 || ISGEN2 || ISIKU200 )
	{
		PD7 = 0;
		BBC_Configure( BBC_PWMCH_CHARGER, 0 );          // 5 0
		PD7 = 0;
	}

        if ( ISGEN3 || ISFIT )
	{
		PD1 = 0;
		GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
                PD1 = 0;
	}
        else if ( ISSINFJ200 )
        {
            	PA1 = 1;
		//? BBC_Configure( 4, 0 );                          // 4 0
                //PA1 = 1;
                
                PB7 = 0; // LED
		GPIO_SetMode( PB, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );    // PB #0x80 1
        }
        else if ( ISIKU200 )
        {
                PA2 = 1;
                BBC_Configure( 3, 0 ); //need for charge
                PA2 = 1;                
        }
        
	// BUTTONS
	GPIO_SetMode( PE, GPIO_PIN_PIN0_Msk, GPIO_MODE_INPUT );     // 0x40004100 1 0
	GPIO_SetMode( PD, GPIO_PIN_PIN2_Msk, GPIO_MODE_INPUT );     // 0x400040C0 4 0
	GPIO_SetMode( PD, GPIO_PIN_PIN3_Msk, GPIO_MODE_INPUT );     // 0x400040C0 8 0

	if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 )
	{
		PF2 = 1;                                                                // 0x40004948
		GPIO_SetMode( PF, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );                // 0x40004140 4 1
	}
	else if ( ISRX300 || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISINVOKE 
                || ISRX2 || ISSINFJ200 || ISRX217 || ISGEN2 )
	{
            if ( ISRX300 || ISRX2 || ISSINFJ200 || ISRX217 || ISGEN2 )
            {
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF5MFP_Msk|SYS_GPF_MFPL_PF6MFP_Msk);
		SYS->GPF_MFPL |= SYS_GPF_MFPL_PF5MFP_GPIO|SYS_GPF_MFPL_PF6MFP_GPIO;
		PF5 = 0;
		GPIO_SetMode( PF, GPIO_PIN_PIN5_Msk, GPIO_MODE_OUTPUT );
		PF6 = 0;
		GPIO_SetMode( PF, GPIO_PIN_PIN6_Msk, GPIO_MODE_OUTPUT );
            }
            else if ( ISGEN3 )
            {
                SYS->GPF_MFPL &= ~SYS_GPF_MFPL_PF5MFP_Msk;
		SYS->GPF_MFPL |= SYS_GPF_MFPL_PF5MFP_GPIO;
		PF5 = 0;
		GPIO_SetMode( PF, GPIO_PIN_PIN5_Msk, GPIO_MODE_OUTPUT );   
            }
            
            if ( !ISRX2 && !ISRX217 && !ISGEN2 )
            {
                PA3 = 0;
                GPIO_SetMode( PA, GPIO_PIN_PIN3_Msk, GPIO_MODE_OUTPUT );
                
                if ( !ISSINFJ200 )
                {
                    PA2 = 0;
                    GPIO_SetMode( PA, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );
                }
            }
	}
        else if ( ISIKU200 )
            {
                PB7 = 1;
                GPIO_SetMode( PB, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
                PC2 = 0;
                GPIO_SetMode( PC, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );
            }
        
	// BUCK/BOOST CONVERTER CONTROL LINES
        PC1 = 0;                                                                // 0x40004884
        GPIO_SetMode( PC, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );                // 0x40004080 2 1

        if ( !ISINVOKE )
        {
            PC3 = 0;                                                                // 0x4000488C
            GPIO_SetMode( PC, GPIO_PIN_PIN3_Msk, GPIO_MODE_OUTPUT );                // 0x40004080 8 1
        }

	// SSD RESET/VDD/VCC
	PA0 = 0;                                                                // 0x40004800
	GPIO_SetMode( PA, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );                // 0x40004000 1 1
        
        if ( !ISINVOKE )
        {
            PA1 = 0;                                                                // 0x40004804
            GPIO_SetMode( PA, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );                // 0x40004000 2 1
        }
        
	PC4 = 0;                                                                // 0x40004890
	GPIO_SetMode( PC, GPIO_PIN_PIN4_Msk, GPIO_MODE_OUTPUT );                // 0x40004080 0x10 1

	// BATTERY
        if ( !ISSINP80 && !ISINVOKE && !ISRX2 && !ISRX217 && !ISGEN2 )
        {
            GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk, GPIO_MODE_INPUT );             // 0x400040C0 1 0
            GPIO_EnableInt( PD, 0, GPIO_INT_FALLING );                          // 0x400040C0 0 1
        }
        
	if ( ISVTCDUAL )
	{
		PA2 = 0;
		GPIO_SetMode( PA, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );
		PF2 = 0;
		GPIO_SetMode( PF, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );

		GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_INPUT );
		GPIO_EnableInt( PD, 1, GPIO_INT_RISING );
		GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN1_Msk );
	}
	else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 && !ISPRIMO1 
                && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISINVOKE && !ISRX2 
                && !ISSINFJ200 && !ISRX217 && !ISGEN2 && !ISIKU200 )
	{
		GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_INPUT );
		GPIO_EnableInt( PD, 7, GPIO_INT_RISING );
		GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN7_Msk );
	}

	// SPI0 (Display control)
	PE10 = 0;                                                               // 0x40004928
	GPIO_SetMode( PE, GPIO_PIN_PIN10_Msk, GPIO_MODE_OUTPUT );               // 0x40004100 (0x400 = 0x40004928 ASR 0x14) 1
        
        if ( ISRX2 || ISINVOKE || ISRX217 || ISGEN2 )
            PE12 = 1;
        else
            PE12 = 0;                                                               // 0x40004930
        
	GPIO_SetMode( PE, GPIO_PIN_PIN12_Msk, GPIO_MODE_OUTPUT );               // 0x40004100 (0x1000 = 0x40004930 ASR 0x12) 1
        
	// LED Control
	if ( ISEGRIPII || ISEVICAIO )
	{
		PB3 = 0;	// Blue
		PB4 = 0;	// Red
		PB5 = 0;	// Green
		GPIO_SetMode( PB, GPIO_PIN_PIN3_Msk|GPIO_PIN_PIN4_Msk|GPIO_PIN_PIN5_Msk, GPIO_MODE_OUTPUT );
	}

	if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISGEN3 )
	{
		SYS->GPF_MFPL &= ~SYS_GPF_MFPL_PF1MFP_Msk;
		SYS->GPF_MFPL |= SYS_GPF_MFPL_PF1MFP_GPIO;
  
                if ( ISRX300 )
                {
                    PD1 = 1;
                    GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
                }
                else if ( ISGEN3 )
                {
                    PB7 = 1;
                    GPIO_SetMode( PB, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );    
                }
                else
                {
                    PF1 = 1;
                    GPIO_SetMode( PF, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );    
                }
	}
        else if ( ISSINP80 )
        {
                PD0 = 0;      // LED R G(01) Y Off                                                 // 0x400048C0
                PD1 = 0;                                                                        // 0x400048C4
                GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk|GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );      // 0x400040C0 3 1
        }
        else if ( ISRX2 || ISRX217 || ISGEN2 )
        {
            	PA3 = 1;
		GPIO_SetMode( PA, GPIO_PIN_PIN3_Msk, GPIO_MODE_OUTPUT );            //pa 8 1
        }
        else if ( ISSINFJ200 )
        {
                PA2 = 1;
                GPIO_SetMode( PA, GPIO_PIN_PIN2_Msk, GPIO_MODE_OUTPUT );            //pa 4 1
        }
        else if ( ISIKU200 )
        {
            PE13 = 1;
            GPIO_SetMode( PE, GPIO_PIN_PIN13_Msk, GPIO_MODE_OUTPUT );
        }        
	else
	{
		// ? (What is PB.7?) ISINVOKE
		PB7 = 1;
		GPIO_SetMode( PB, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
	}

        //Nested Vector Interrupt Controller
/*
        if ( ISRX217 || ISGEN2 || ISFIT ) //what they do?
        {
        NVIC_EnableIRQ( GPA_IRQn );                                         // 0x10
        NVIC_EnableIRQ( GPB_IRQn );                                         // 0x11
        }
*/
        
	NVIC_EnableIRQ( GPD_IRQn );                                         // 0x13
	NVIC_EnableIRQ( GPE_IRQn );                                         // 0x14
	NVIC_EnableIRQ( GPF_IRQn );                                         // 0x15

	// Debounce time = 100ms
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024 );       // 0x3A 0x40004440
}

