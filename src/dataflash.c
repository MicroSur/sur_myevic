#include "myevic.h"
#include "myrtc.h"
#include "screens.h"
#include "atomizer.h"
#include "display.h"
#include "events.h"
#include "battery.h"
#include "atomizer.h"
#include "miscs.h"

#include "timers.h"
#include "dataflash.h"

//=========================================================================
// DataFlash parameters global structure

dfStruct_t DataFlash;

uint8_t ParamsBackup[DATAFLASH_PARAMS_SIZE];

//-------------------------------------------------------------------------
// Global variables

uint8_t		UpdateDFTimer;
uint8_t		UpdatePTTimer;
uint8_t		DFMagicNumber;
uint8_t		X32Off;
uint8_t		ScrFlip;
char            *BoxName;


//-------------------------------------------------------------------------
// Internal variables

uint16_t	fmcCntrsIndex;

#define __JOYETECH__ \
	__attribute__((aligned(4))) \
	__attribute__((section (".joyetech")))
//for updaters
const char joyetech	[14]	__JOYETECH__	= { 'J','o','y','e','t','e','c','h',' ','A','P','R','O','M' };
// myPID - 0x15C offset in binary file always
uint32_t __attribute__((section (".myPID"))) myPID = 0;

//-------------------------------------------------------------------------

#define __PIDATTR__ \
	__attribute__((aligned(4))) \
	__attribute__((section (".productid")))

const char pid_vtcmini	[8]	__PIDATTR__	= { 'E','0','5','2', 1, 1, 1, 0 };
const char pid_vtwomini	[8]	__PIDATTR__	= { 'E','1','1','5', 1, 0, 1, 0 };
const char pid_primomini[8]	__PIDATTR__	= { 'E','1','9','6', 1, 0, 1, 0 };
const char pid_primose  [8]	__PIDATTR__	= { 'E','2','3','5', 1, 0, 0, 0 };
const char pid_vtwo     [8]	__PIDATTR__	= { 'E','0','4','3', 1, 0, 1, 0 };
const char pid_evicaio	[8]	__PIDATTR__	= { 'E','0','9','2', 1, 0, 1, 0 };
const char pid_egripii	[8]	__PIDATTR__	= { 'E','0','8','3', 1, 0, 0, 0 };
const char pid_cubomini	[8]	__PIDATTR__	= { 'E','0','5','6', 1, 0, 2, 0 };
const char pid_evicbasic[8]	__PIDATTR__	= { 'E','1','5','0', 1, 0, 1, 0 };
const char pid_presa75w	[8]	__PIDATTR__	= { 'W','0','0','7', 1, 0, 3, 0 };
const char pid_presa100w[8]	__PIDATTR__	= { 'W','0','1','7', 1, 0, 0, 0 };
const char pid_wrx75tc	[8]	__PIDATTR__	= { 'W','0','2','6', 1, 0, 3, 0 };
const char pid_vtcdual	[8]	__PIDATTR__	= { 'E','0','7','9', 1, 0, 1, 0 };
const char pid_cuboid	[8]	__PIDATTR__	= { 'E','0','6','0', 1, 0, 2, 0 };
const char pid_cubo200	[8]	__PIDATTR__	= { 'E','1','6','6', 1, 0, 0, 0 };
const char pid_rx200s	[8]	__PIDATTR__	= { 'W','0','3','3', 1, 0, 0, 0 };
const char pid_rx23     [8]	__PIDATTR__	= { 'W','0','1','8', 1, 0, 2, 0 };
const char pid_rx300	[8]	__PIDATTR__	= { 'W','0','6','9', 1, 0, 0, 0 };
const char pid_rxmini	[8]	__PIDATTR__	= { 'W','0','7','3', 1, 0, 0, 0 };
const char pid_lpb      [8]	__PIDATTR__	= { 'W','0','4','3', 1, 0, 0, 0 };
const char pid_primo1   [8]	__PIDATTR__	= { 'E','1','8','2', 1, 0, 1, 0 };
const char pid_primo2   [8]	__PIDATTR__	= { 'E','2','0','3', 1, 0, 1, 0 };
const char pid_predator [8]	__PIDATTR__	= { 'W','0','7','8', 1, 0, 1, 0 };
const char pid_gen3     [8]	__PIDATTR__	= { 'W','0','9','1', 1, 0, 0, 0 };
const char pid_sinp80   [8]	__PIDATTR__	= { 'J','0','0','1', 1, 0, 0, 0 };
const char pid_sinfj200 [8]	__PIDATTR__	= { 'J','0','0','9', 1, 0, 0, 0 };
const char pid_rx2      [8]	__PIDATTR__	= { 'J','0','1','2', 1, 0, 0, 0 };
const char pid_invoke   [8]	__PIDATTR__	= { 'M','0','9','5', 1, 0, 0, 0 };
const char pid_rx217    [8]	__PIDATTR__	= { 'J','0','7','5', 1, 0, 0, 0 };
const char pid_gen2     [8]	__PIDATTR__	= { 'J','0','5','9', 1, 0, 0, 0 };
const char pid_iku200   [8]	__PIDATTR__	= { 'M','0','7','2', 1, 0, 0, 0 };
const char pid_fit      [8]	__PIDATTR__	= { 'E','2','3','9', 1, 0, 0, 0 };

#define PID_SCRAMBLE 0x12345678UL
#define MAKEPID(p) ((((p)[0])|((p)[1]<<8)|((p)[2]<<16)|((p)[3]<<24))^PID_SCRAMBLE)
#define MAKEHWV(p) (((p)[4])|((p)[5]<<8)|((p)[6]<<16)|((p)[7]<<24))

//#define HWV2INT(v) (((v)&0xff)*100+(((v)>>8)&0xff)*10+(((v)>>16)&0xff))

#define PID_VTCMINI	MAKEPID(pid_vtcmini)
#define PID_VTWOMINI    MAKEPID(pid_vtwomini)
#define PID_PRIMOMINI	MAKEPID(pid_primomini)
#define PID_PRIMOSE	MAKEPID(pid_primose)
#define PID_VTWO	MAKEPID(pid_vtwo)
#define PID_EVICAIO	MAKEPID(pid_evicaio)
#define PID_EGRIPII	MAKEPID(pid_egripii)
#define PID_CUBOMINI	MAKEPID(pid_cubomini)
#define PID_EVICBASIC	MAKEPID(pid_evicbasic)
#define PID_PRESA75W	MAKEPID(pid_presa75w)
#define PID_PRESA100W	MAKEPID(pid_presa100w)
#define PID_WRX75TC	MAKEPID(pid_wrx75tc)
#define PID_VTCDUAL	MAKEPID(pid_vtcdual)
#define PID_CUBOID	MAKEPID(pid_cuboid)
#define PID_CUBO200	MAKEPID(pid_cubo200)
#define PID_RX200S	MAKEPID(pid_rx200s)
#define PID_RX23	MAKEPID(pid_rx23)
#define PID_RX300	MAKEPID(pid_rx300)
#define PID_RXMINI	MAKEPID(pid_rxmini)
#define PID_LPB		MAKEPID(pid_lpb)
#define PID_PRIMO1	MAKEPID(pid_primo1)
#define PID_PRIMO2	MAKEPID(pid_primo2)
#define PID_PREDATOR	MAKEPID(pid_predator)
#define PID_GEN3	MAKEPID(pid_gen3)
#define PID_SINP80	MAKEPID(pid_sinp80)
#define PID_SINFJ200	MAKEPID(pid_sinfj200)
#define PID_RX2         MAKEPID(pid_rx2)
#define PID_INVOKE      MAKEPID(pid_invoke)
#define PID_RX217       MAKEPID(pid_rx217)
#define PID_GEN2        MAKEPID(pid_gen2)
#define PID_IKU200      MAKEPID(pid_iku200)
#define PID_FIT         MAKEPID(pid_fit)

#define HWV_VTCMINI	MAKEHWV(pid_vtcmini)
#define HWV_VTWOMINI	MAKEHWV(pid_vtwomini)
#define HWV_PRIMOMINI	MAKEHWV(pid_primomini)
#define HWV_PRIMOSE	MAKEHWV(pid_primose)
#define HWV_VTWO	MAKEHWV(pid_vtwo)
#define HWV_EVICAIO	MAKEHWV(pid_evicaio)
#define HWV_EGRIPII	MAKEHWV(pid_egripii)
#define HWV_CUBOMINI	MAKEHWV(pid_cubomini)
#define HWV_EVICBASIC	MAKEHWV(pid_evicbasic)
#define HWV_PRESA75W	MAKEHWV(pid_presa75w)
#define HWV_PRESA100W	MAKEHWV(pid_presa100w)
#define HWV_WRX75TC	MAKEHWV(pid_wrx75tc)
#define HWV_VTCDUAL	MAKEHWV(pid_vtcdual)
#define HWV_CUBOID	MAKEHWV(pid_cuboid)
#define HWV_CUBO200	MAKEHWV(pid_cubo200)
#define HWV_RX200S	MAKEHWV(pid_rx200s)
#define HWV_RX23	MAKEHWV(pid_rx23)
#define HWV_RX300	MAKEHWV(pid_rx300)
#define HWV_RXMINI	MAKEHWV(pid_rxmini)
#define HWV_LPB		MAKEHWV(pid_lpb)
#define HWV_PRIMO1	MAKEHWV(pid_primo1)
#define HWV_PRIMO2	MAKEHWV(pid_primo2)
#define HWV_PREDATOR	MAKEHWV(pid_predator)
#define HWV_GEN3	MAKEHWV(pid_gen3)
#define HWV_SINP80	MAKEHWV(pid_sinp80)
#define HWV_SINFJ200	MAKEHWV(pid_sinfj200)
#define HWV_RX2         MAKEHWV(pid_rx2)
#define HWV_INVOKE      MAKEHWV(pid_invoke)
#define HWV_RX217       MAKEHWV(pid_rx217)
#define HWV_GEN2        MAKEHWV(pid_gen2)
#define HWV_IKU200      MAKEHWV(pid_iku200)
#define HWV_FIT         MAKEHWV(pid_fit)

//=========================================================================
// Reset device to LDROM
//-------------------------------------------------------------------------
__myevic__ void ResetToLDROM()
{
	dfBootFlag = 1;
	UpdateDataFlash();

	SYS_UnlockReg();

	FMC_SELECT_NEXT_BOOT( 1 );
	SCB->AIRCR = 0x05FA0004;

	while ( 1 )
		;
}

//=========================================================================
        
__myevic__ void RestartMod()
{
	//if ( UpdateDFTimer ) 
        UpdateDataFlash();
	//if ( UpdatePTTimer ) 
        UpdatePTCounters();

	if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISPRIMO1 
                || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISINVOKE || ISRX2 || ISSINFJ200 
                || ISRX217 || ISGEN2 || ISIKU200 )
	{
		PD7 = 0;                                            //48DC
		BBC_Configure( BBC_PWMCH_CHARGER, 0 );              // 5 0
		PD7 = 0;
		ChargerDuty = 0;

		if ( ISVTCDUAL )
		{
			PA3 = 0;
			PC3 = 0;
			PA2 = 0;
		}
                else if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISINVOKE )
                {
                        PD1 = 0;                                    //48C4
                }
                else if ( ISRX2 || ISRX217 || ISGEN2 )
                {
                        PF2 = 0;                                    //4948
                }
		else if ( !ISSINFJ200 && !ISIKU200 )
                    // if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 gen3 )
				// (currently useless, restore if needed)
		{
			PF0 = 0;
		}
	}

	if ( !gFlags.has_x32 )
	{
		RTCAdjustClock( dfBootFlag ? 9 : 1 );
		RTCAdjustClock( 0 );
		CLK_SysTickDelay( 500 );
	}

	SYS_UnlockReg();

	FMC_SELECT_NEXT_BOOT( dfBootFlag );
	SCB->AIRCR = 0x05FA0004;

	while ( 1 )
		;
}

//=========================================================================

__myevic__ void ResetDFlashRes()
{
    int p = dfProfile;
    ResetDataFlash();
    dfProfile = p;      
    
    ///Halt WaitOnTMR2( 200 );
    //RestartMod();
            
    //rx23 not found coil some time
    //RereadRez();
    
    //Halt    ResetResistance();
    //if ( AtoStatus == 4 ) SwitchRezLock( 1 );

}

//=========================================================================
//----- (00002064) --------------------------------------------------------
__myevic__ void SetProductID()
{
	SYS_UnlockReg();
	FMC_ENABLE_ISP();

	NumBatteries = 1;
	MaxBatteries = 1;
	MaxCurrent = 25;

	uint32_t offset;
	uint32_t u32Data;

        //MemCpy( BoxID, pid_invoke, 4 );
        BoxName = ""; //9 max
        DFMagicNumber = 0xDA;
                
	for ( offset = 0 ; offset < LDROM_SIZE ; offset += 4 )
	{
                if ( myPID )
                {
                    u32Data = myPID;
                }
                else
                {
                    u32Data = FMC_Read( LDROM_BASE + offset );
                }            

		u32Data ^= PID_SCRAMBLE;

		if ( u32Data == PID_VTCMINI )
		{
			dfMaxHWVersion = HWV_VTCMINI;
			//DFMagicNumber = 0x36;
			BoxModel = BOX_VTCMINI;
			X32Off = 1;
                        BoxName = "VTCm";
			break;
		}
		else if ( u32Data == PID_VTWOMINI )
		{
			dfMaxHWVersion = HWV_VTWOMINI;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_VTWOMINI;
                        BoxName = "VTWOm";
			break;
		}
		else if ( u32Data == PID_PRIMOMINI )
		{
			dfMaxHWVersion = HWV_PRIMOMINI;
			//DFMagicNumber = 0x11;
			BoxModel = BOX_PRIMOMINI;
                        BoxName = "PRIMOm";
			break;
		}  
		else if ( u32Data == PID_PRIMOSE )
		{
			dfMaxHWVersion = HWV_PRIMOSE;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_PRIMOSE;
                        BoxName = "PrimoSE";
			break;
		}
		else if ( u32Data == PID_FIT )
		{
			dfMaxHWVersion = HWV_FIT;
			BoxModel = BOX_FIT;
                        BoxName = "FIT";
			break;
		}
		else if ( u32Data == PID_VTWO )
		{
			dfMaxHWVersion = HWV_VTWO;
			//DFMagicNumber = 0x40;
			BoxModel = BOX_VTWO;
                        BoxName = "VTwo";
			break;
		}
		else if ( u32Data == PID_VTCDUAL )
		{
			dfMaxHWVersion = HWV_VTCDUAL;
			//DFMagicNumber = 0x12;
			BoxModel = BOX_VTCDUAL;
			NumBatteries = 0;
			MaxBatteries = 2;
			gFlags.pwm_pll = 1;
                        BoxName = "VTCd";
			break;
		}
                else if ( u32Data == PID_PRIMO1 )
		{
			dfMaxHWVersion = HWV_PRIMO1;
			//DFMagicNumber = 0x11;
			BoxModel = BOX_PRIMO1;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        BoxName = "Primo1";
			break;
		}
                else if ( u32Data == PID_PRIMO2 )
		{
			dfMaxHWVersion = HWV_PRIMO2;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_PRIMO2;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        BoxName = "Primo2";
			break;
		}     
                else if ( u32Data == PID_PREDATOR )
		{
			dfMaxHWVersion = HWV_PREDATOR;
			//DFMagicNumber = 0x11;
			BoxModel = BOX_PREDATOR;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        BoxName = "P228";
			break;
		}         
                else if ( u32Data == PID_GEN3 )
		{
			dfMaxHWVersion = HWV_GEN3;
			//DFMagicNumber = 0x14;
			BoxModel = BOX_GEN3;
			NumBatteries = 3;
			MaxBatteries = 3;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        ScrFlip = 1;
                        X32Off = 1;
                        BoxName = "GEN3";
			break;
		}
                else if ( u32Data == PID_SINP80 )
		{
			dfMaxHWVersion = HWV_SINP80;
			//DFMagicNumber = 0x11;
			BoxModel = BOX_SINP80;
                        ScrFlip = 1;
                        X32Off = 1;
                        BoxName = "P80";
			break;
		}
                else if ( u32Data == PID_SINFJ200 )
		{
			dfMaxHWVersion = HWV_SINFJ200;
			//DFMagicNumber = 0x13;
			BoxModel = BOX_SINFJ200;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        ScrFlip = 1;
                        X32Off = 1;
                        BoxName = "FJ200";
			break;
		}
                else if ( u32Data == PID_IKU200 )
		{
			dfMaxHWVersion = HWV_IKU200;
			//DFMagicNumber = ;
			BoxModel = BOX_IKU200;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        //ScrFlip = 1;
                        X32Off = 1;
                        BoxName = "i200";
			break;
		}                
                else if ( u32Data == PID_RX2 || u32Data == PID_RX217 || u32Data == PID_GEN2 )
		{
			//DFMagicNumber = 0x12;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        //ScrFlip = 1;
                        X32Off = 1;
                      
                        if ( u32Data == PID_RX217 )
                        {
                            dfMaxHWVersion = HWV_RX217;
                            BoxModel = BOX_RX217;
                            BoxName = "RX217";
                        }
                        else if ( u32Data == PID_GEN2 )
                        {
                            dfMaxHWVersion = HWV_GEN2;
                            BoxModel = BOX_GEN2;
                            BoxName = "Gen3D";
                            ScrFlip = 1;
                        }
                        else
                        {
                            dfMaxHWVersion = HWV_RX2;
                            BoxModel = BOX_RX2;
                            BoxName = "RX2";
                        }
			break;
		}
                else if ( u32Data == PID_INVOKE )
		{
			dfMaxHWVersion = HWV_INVOKE;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_INVOKE;
			NumBatteries = 2;
			MaxBatteries = 2;
                        MaxCurrent = 50;
			gFlags.pwm_pll = 1;
                        //ScrFlip = 1; //noflip
                        X32Off = 1;
                        BoxName = "Invoke";
			break;
		}                
		else if ( u32Data == PID_EVICAIO )
		{
			dfMaxHWVersion = HWV_EVICAIO;
			//DFMagicNumber = 0x50;
			BoxModel = BOX_EVICAIO;
			ScrFlip = 1;
                        BoxName = "AIO";                        
			break;
		}
		else if ( u32Data == PID_EGRIPII )
		{
			dfMaxHWVersion = HWV_EGRIPII;
			//DFMagicNumber = 0x15;
			BoxModel = BOX_EGRIPII;  
			break;
		}
		else if ( u32Data == PID_CUBOMINI )
		{
			dfMaxHWVersion = HWV_CUBOMINI;
			//DFMagicNumber = 0x50;
			BoxModel = BOX_CUBOMINI;
			ScrFlip = 1;
			X32Off = 1;
                        BoxName = "CUBm";                        
			break;
		}
		else if ( u32Data == PID_CUBOID )
		{
			dfMaxHWVersion = HWV_CUBOID;
			//DFMagicNumber = 0x39;
			BoxModel = BOX_CUBOID;
			NumBatteries = 2;
			MaxBatteries = 2;
			gFlags.pwm_pll = 1;
			ScrFlip = 1;
			X32Off = 1;
                        BoxName = "Cuboid";                        
			break;
		}
		else if ( u32Data == PID_CUBO200 )
		{
			dfMaxHWVersion = HWV_CUBO200;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_CUBO200;
			NumBatteries = 3;
			MaxBatteries = 3;
			MaxCurrent = 50;
			gFlags.pwm_pll = 1;
			X32Off = 1;
                        BoxName = "Cub200";
			break;
		}
		else if ( u32Data == PID_EVICBASIC )
		{
			dfMaxHWVersion = HWV_EVICBASIC;
			//DFMagicNumber = 0x13;
			BoxModel = BOX_EVICBASIC;
			ScrFlip = 1;
                        BoxName = "Basic";                        
			break;
		}
		else if ( u32Data == PID_PRESA75W )
		{
			dfMaxHWVersion = HWV_PRESA75W;
			//DFMagicNumber = 0x30;
			BoxModel = BOX_PRESA75W;
			X32Off = 1;
                        BoxName = "Presa75";                        
			break;
		}
		else if ( u32Data == PID_PRESA100W )
		{
			dfMaxHWVersion = HWV_PRESA100W;
			//DFMagicNumber = 0x40;
			BoxModel = BOX_PRESA100W;
			X32Off = 1;
			break;
		}
		else if ( u32Data == PID_WRX75TC )
		{
			dfMaxHWVersion = HWV_WRX75TC;
			//DFMagicNumber = 0x32;
			BoxModel = BOX_WRX75TC;
			X32Off = 1;
			break;
		}
		else if ( u32Data == PID_RXMINI )
		{
			dfMaxHWVersion = HWV_RXMINI;
			//DFMagicNumber = 0x10;
			BoxModel = BOX_RXMINI;
			X32Off = 1;
			break;
		}
		else if ( u32Data == PID_LPB )
		{
                        //Vaponaute La Petite Box
			dfMaxHWVersion = HWV_LPB;
			//DFMagicNumber = 0x31;
			BoxModel = BOX_PRESA75W;	// Act as Presa 75W
			X32Off = 1;
			break;
		}
		else if ( u32Data == PID_RX200S )
		{
			dfMaxHWVersion = HWV_RX200S;
			//DFMagicNumber = 0x14;
			BoxModel = BOX_RX200S;
			NumBatteries = 3;
			MaxBatteries = 3;
			MaxCurrent = 50;
			gFlags.pwm_pll = 1;
			X32Off = 1;
                        BoxName = "200S";                        
			break;
		}
		else if ( u32Data == PID_RX23 )
		{
			dfMaxHWVersion = HWV_RX23;
			//DFMagicNumber = 0x14;
			BoxModel = BOX_RX23;
			NumBatteries = 3;
			MaxBatteries = 3;
			MaxCurrent = 50;
			gFlags.pwm_pll = 1;
			X32Off = 1;
                        BoxName = "RX2/3";
			break;
		}
		else if ( u32Data == PID_RX300 )
		{
			dfMaxHWVersion = HWV_RX300;
			//DFMagicNumber = 0x12;
			BoxModel = BOX_RX300;
			NumBatteries = 4;
			MaxBatteries = 4;
			MaxCurrent = 50;
			gFlags.pwm_pll = 1;
			X32Off = 1;
                        BoxName = "RX300";
			break;
		}
	}
        
        //gFlags.FireNotFlipped = 1; //!ScrFlip;
        
	FMC_DISABLE_ISP();
	SYS_LockReg();

	if ( offset < LDROM_SIZE )
	{
		dfProductID = u32Data ^ PID_SCRAMBLE;

	//	What's the right behavior in case of bad
	//	hardware version?
	//
	//	if ( HWV2INT(dfMaxHWVersion) < dfHWVersion )
	//	{
	//		ResetToLDROM();
	//	}
	}
	else
	{
		ResetToLDROM();
	}
}


//=========================================================================
//----- (00002080) --------------------------------------------------------
__myevic__ void FMCReadCounters()
{
	uint32_t pc, v;
	uint32_t idx;

	SYS_UnlockReg();
	FMC_ENABLE_ISP();

	for ( idx = 0 ; idx < FMC_FLASH_PAGE_SIZE ; idx += 4 )
	{
		v = FMC_Read( DATAFLASH_PUFFCNTR_BASE + idx );
		if ( v == ~0 ) break;
		pc = v;
	}

	fmcCntrsIndex = idx;

	if ( idx )
	{
		dfPuffCount = pc;
		dfTimeCount = FMC_Read( DATAFLASH_TIMECNTR_BASE + idx - 4 );
	}
	else
	{
		dfPuffCount = 0;
		dfTimeCount = 0;
	}
               
	FMC_DISABLE_ISP();
	SYS_LockReg();
}


//=========================================================================
//----- (000020CC) --------------------------------------------------------
__myevic__ void UpdatePTCounters()
{
	SYS_UnlockReg();
	FMC_ENABLE_ISP();

	if ( fmcCntrsIndex >= FMC_FLASH_PAGE_SIZE )
	{
		FMC_Erase( DATAFLASH_PUFFCNTR_BASE );
		FMC_Erase( DATAFLASH_TIMECNTR_BASE );
		fmcCntrsIndex = 0;
	}

	FMC_Write( DATAFLASH_PUFFCNTR_BASE + fmcCntrsIndex, dfPuffCount );
	FMC_Write( DATAFLASH_TIMECNTR_BASE + fmcCntrsIndex, dfTimeCount );
	fmcCntrsIndex += 4;

	FMC_DISABLE_ISP();
	SYS_LockReg();
}


//=========================================================================
__myevic__ void ResetPowerCurve()
{
	for ( int i = 0 ; i < PWR_CURVE_PTS ; ++i )
	{
		dfPwrCurve[i].time = 0;
		dfPwrCurve[i].power = 100;
	}
}

__myevic__ void InitSetPowerVoltMax()
{
	if ( ISRX300 )
	{
		SetMaxVolts ( 990 ); //MaxVolts = 990;
	}
        else if ( ISINVOKE || ISIKU200 )
        {
                SetMaxVolts ( 800 );
        }
	else
	{
		SetMaxVolts ( 900 );
	}

	if ( ISEVICBASIC )
	{
		SetMaxPower ( 600 ); //MaxPower = 600;
	}
	//else if ( ISPRIMOSE || ISPRIMOMINI || ISVTWO || ISEGRIPII || ISCUBOMINI || ISRXMINI || ISSINP80 )
	//{
	//	SetMaxPower ( 800 );
	//}
	//else if ( ISPRESA100W )
	//{
	//	SetMaxPower ( 1000 );
	//}
	else if ( ISVTCDUAL )
	{
		SetMaxPower ( 1500 );
		gFlags.batt_unk = 1;
	}
	//else if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISRX200S || ISRX23 )
	//{
	//	SetMaxPower ( 2500 );
	//}        
	else if ( ISCUBOID || ISCUBO200 || ISINVOKE || ISSINFJ200 || ISRX2 || ISRX217 
                || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISRX200S || ISRX23 || ISGEN2 
                || ISGEN3 || ISRX300 || ISIKU200 )
	{
		SetMaxPower ( 2000 );
	}
        //else if ( ISGEN3 )
        //{
        //        SetMaxPower ( 3000 );
        //}
	//else if ( ISRX300 )
	//{
	//	SetMaxPower ( 4000 );
	//}
	else
	{
		SetMaxPower ( 800 );
	}

	//MaxTCPower = MaxPower;    
}

//=========================================================================
//----- (00001C30) --------------------------------------------------------
__myevic__ void ResetDataFlash()
{
	int hwv;
        uint8_t		tmpBVOffset[4];
        dfPCPoint_t     tmpPwrCurve[PWR_CURVE_PTS];
        int8_t          tmpAkkuTempCorr;
        int8_t		tmpBoardTempCorr;
        
        MemCpy(tmpBVOffset, dfBVOffset, 4); // I dnt remember
        MemCpy(tmpPwrCurve, dfPwrCurve, PWR_CURVE_PTS);
        tmpAkkuTempCorr = dfAkkuTempCorr;
        tmpBoardTempCorr = dfBoardTempCorr;
	hwv = dfHWVersion;
        
	MemClear( DataFlash.params, DATAFLASH_PARAMS_SIZE );
	
	// Parameters whose reset value is zero are commented out
	// since we start by clearing the memory.
        
        dfHWVersion = hwv;
        MemCpy(dfBVOffset, tmpBVOffset, 4);
        MemCpy(dfPwrCurve, tmpPwrCurve, PWR_CURVE_PTS);
        dfAkkuTempCorr = tmpAkkuTempCorr;
        dfBoardTempCorr = tmpBoardTempCorr;
        
	dfMagic = DFMagicNumber;       
	dfMode = 4;
	dfTempAlgo = 3;
        dfVWTempAlgo = 3;         //M1
	dfProtec = FIRE_PROTEC_DEF;
//	dfVWVolts = 300; 
	dfPower = 200;
	dfTCPower = 300;
	dfPreheatPwr = dfTCPower; //300;        
	dfStatus.IsCelsius = 1;
//	dfRezType = 1;
	dfTemp = 200;
//	dfResistance = 0;
//	dfUIVersion = 2;
	dfAPT = 10;
//      dfAPT3 = 0; //7
//	dfStealthOn = 0;
//	dfTempCoefsNI = 201;
//	dfTempCoefsTI = 101;
	dfLEDColor = 151*175; //all //red = 25 << 10;
//	dfStatus.off = 0;
//	dfStatus.keylock = 0;
	dfStatus.flipped = ScrFlip;
//      dfStatus.fireflip = 0;
//      dfStatus.vvlite = 0;        //VVolt
	dfStatus.nologo = 1;
        dfAutoPuffTimer = 20;
	dfStatus.x32off = X32Off;
	dfStatus.onewatt = 1;
	dfStatus.digclk = 1;
//      dfStatus2.digclk2 = 0;      //00 01 10
        dfStatus2.splash1 = 1;      //splash on with box name
//	dfStatus.battpc = 1;
//        dfBattLine = 1;
	dfStatus.wakeonpm = 1;
	dfScrMainTime = 5;          //index of ScrMainTimes[6] = { 30, 60, 5, 10, 15, 20 };
//      dfDimTimeout = 20;          //see dfScrMainTime; set in GetMainScreenDuration()
        dfDimOffTimeout = 120;      //sleep timeout     
	dfTCRM[0] = 90;
	dfTCRM[1] = 100;
	dfTCRM[2] = 110;
        dfUSBMaxCharge = 2000; 
        dfMaxBoardTemp = 70;
        dfBattVolt = 420;           //for zero counters on new battery
	dfScreenSaver = SSAVER_SF;
	dfScreenProt = 3;
//	MemClear( dfSavedCfgRez, sizeof(dfSavedCfgRez) );
//	MemClear( dfSavedCfgPwr, sizeof(dfSavedCfgPwr) );
//        dfTTSpeed = 2;
        dfColdLockTemp = 20;        //cels
        dfNewRezPerc = 5;           //%
	dfContrast = 45;            //17%
//      dfContrast2 = 0;
//	dfModesSel = 0;
	dfClkRatio = RTC_DEF_CLK_RATIO;
	dfVVRatio = VVEL_DEF_RATIO; ////#define VVEL_DEF_RATIO	 300 //360
        dfPuffsOff = PUFFS_OFF_DEF;
        dfHideLogo = 3;
	dfPreheatTime = 10;
	dfClick[0] = CLICK_ACTION_EDIT;
	dfClick[1] = CLICK_ACTION_ON_OFF;
	dfClick[2] = CLICK_ACTION_TETRIS;
        dfClick[3] = CLICK_ACTION_ON_OFF;
//	dfBatteryModel = 0;
//	dfTCAlgo = TCALGO_DEF; // =0 
	dfTCBoost = 50;
	dfPID.P = PID_P_DEF;
	dfPID.I = PID_I_DEF;
	dfPID.D = PID_D_DEF;
//	dfMillis = 0;
//	dfProfile = 0;
//      dfStatus.offmodclock = 0;
        dfFireScrDuration = 2;
        Object3D = 7;
//      dfMaxPower = 0;
//      dfMaxVolts = 0;
        
        RereadRez();
        InitSetPowerVoltMax();
        SetAtoLimits();
        
//      dfStatus.nbrc = 0;        
        
//	ResetCustomBattery();
//	ResetPowerCurve();
        
	UpdateDataFlash();

	//dfPuffCount = 0;
	//dfTimeCount = 0;
	//UpdatePTCounters();

	AtoShuntRez = GetShuntRezValue();
}


//=========================================================================
/*
__myevic__ void DFCheckValuesValidity()
{   

	int i,v;

        //dfDimOffTimeout

	if ( dfMode >= 7 )
		dfMode = 4;

	if (( dfProtec < FIRE_PROTEC_MIN ) || ( dfProtec > FIRE_PROTEC_MAX ))
		dfProtec = FIRE_PROTEC_DEF;

	if ( dfVWVolts > MaxVolts || dfVWVolts < 50 )
		dfVWVolts = 330;

	if ( dfPower > MaxPower || dfPower < 10 )
		dfPower = 200;

	if ( dfTCPower > MaxTCPower || dfTCPower < 10 )
		dfTCPower = 200;

	if ( dfPuffCount > 99999 || dfTimeCount > 999999 )
	{
		dfPuffCount = 0;
		dfTimeCount = 0;
		UpdatePTCounters();
	}

	if ( dfUIVersion > 1 )
		dfUIVersion = 0;

	if ( dfAPT > 11 )
		dfAPT = 0;
        
	if ( dfAPT3 > 11 )
		dfAPT3 = 0;
        
        if ( dfBattLine > 3 )
		dfBattLine = 1;
        
        if ( dfAutoPuffTimer > 250 )
		dfAutoPuffTimer = 20;
               
	if ( dfTempAlgo != 1 && dfTempAlgo != 2 && dfTempAlgo != 3 && dfTempAlgo != 4 )
		dfTempAlgo = 1;

	if ( dfIsCelsius > 1 )
	{
		dfIsCelsius = 1;
		//dfTemp = 200;
	}
	
        if ( dfIsCelsius )
	{
		//if ( dfTemp < 100 || dfTemp > 315 )
                if ( dfTemp < 1 || dfTemp > 999 )
			dfTemp = 200;
	}
	else
	{
		//if ( dfTemp < 200 || dfTemp > 600 )
                if ( dfTemp < 32 || dfTemp > 999 )
			dfTemp = 450;
	}

	if ( dfRezTI > 150 )
		dfRezTI = 0;

	if ( dfRezNI > 150 )
		dfRezNI = 0;

	if ( dfRezLockedTI > 1 )
		dfRezLockedTI = 0;

	if ( dfRezLockedNI > 1 )
		dfRezLockedNI = 0;

	if ( dfStealthOn > 2 )
		dfStealthOn = 0;

	//if (( dfTempCoefsNI <= 200 ) || ( dfTempCoefsTI <= 100 ))
	//{
	//	dfTempCoefsNI = 201;
	//	dfTempCoefsTI = 101;

		//ResetCustomBattery();
		//ResetPowerCurve();
	//}
	//else
	//{
		LoadCustomBattery();

		if ( !CheckCustomBattery() )
		{
			ResetCustomBattery();
		}

		for ( i = 0 ; i < PWR_CURVE_PTS ; ++i )
		{
                        if ( i > 0 && dfPwrCurve[i].time == 0 )
				break;
                        
			if (( dfPwrCurve[i].time > 250 || dfPwrCurve[i].power > 200 )
			||	( i == 0 && dfPwrCurve[i].time != 0 )
			||	( i != 0 && dfPwrCurve[i].time <= dfPwrCurve[i-1].time ))
			{
				ResetPowerCurve();
				break;
			}
		}
	//}

	//MemSet( DataFlash.p.Unused4E, 0, sizeof(DataFlash.p.Unused4E) );

	if ( dfShuntRez < SHUNT_MIN_VALUE || dfShuntRez > SHUNT_MAX_VALUE ) //75 150
		dfShuntRez = 0;

	if ( dfRezSS > 150 )
		dfRezSS = 0;

	if ( dfRezLockedSS > 1 )
		dfRezLockedSS = 0;

	if ( dfScrMainTime > 5 )    //constant table index
		dfScrMainTime = 0;

	if ( dfRezTCR > 150 )
		dfRezTCR = 0;

	if ( dfScreenSaver >= SSAVER_MAX )
		dfScreenSaver = SSAVER_CLOCK;

	if ( dfRezLockedTCR > 1 )
		dfRezLockedTCR = 0;

	if ( dfScreenProt > 7 )     //constant table index
		dfScreenProt = 1;   //15s

	if ( dfTCRIndex > 2 )
		dfTCRIndex = 0;

	for ( i = 0 ; i < 3 ; ++i )
	{
		if ( dfTCRM[i] > 999 )
			dfTCRM[i] = 120;
	}

	if ( dfFBSpeed > 2 )
		dfFBSpeed = 0;
	if ( dfTTSpeed > 2 )
		dfTTSpeed = 2;
        
	for ( i = 0 ; i < 10 ; ++i )
	{
		v = dfSavedCfgRez[i];
		if ( v > 350 || ( v < 5 && v ) )
			break;
		v = dfSavedCfgPwr[i];
		if ( v > MaxPower || ( v < 10 && v ) )
			break;
	}
	if ( i < 10 )
	{
		MemClear( dfSavedCfgRez, sizeof(dfSavedCfgRez) );
		MemClear( dfSavedCfgPwr, sizeof(dfSavedCfgPwr) );
	}

	if ( dfModesSel & 0x80 || ( dfModesSel & 0x7F ) == 0x7F )
		dfModesSel = 0;

	if ( dfClkRatio < RTC_MIN_CLOCK_RATIO || dfClkRatio > RTC_MAX_CLOCK_RATIO )
		dfClkRatio = RTC_DEF_CLK_RATIO;

	if ( dfVVRatio < VVEL_MIN_RATIO || dfVVRatio > VVEL_MAX_RATIO )
		dfVVRatio = VVEL_DEF_RATIO;

        if ( dfPuffsOff > PUFFS_OFF_MAX )
		dfPuffsOff = PUFFS_OFF_DEF;
        
	if ( dfPHDelay > 180 )
		dfPHDelay = 0;

	v = 0;
	for ( i = 0 ; i < 3 ; ++i )
	{
		if ( dfClick[i] >= CLICK_ACTION_MAX )
			break;
		if ( dfClick[i] == CLICK_ACTION_EDIT )
			v = 1;
	}
	if ( i < 3 )
	{
		dfClick[0] = CLICK_ACTION_CLOCK;
		dfClick[1] = CLICK_ACTION_EDIT;
		dfClick[2] = CLICK_ACTION_NONE;
	}
	else if ( v == 0 )
	{
		dfClick[1] = CLICK_ACTION_EDIT;
	}

	if ( dfDimTimeout < 5 || dfDimTimeout > 60 )
		dfDimTimeout = ScrMainTimes[dfScrMainTime];

	if (( dfBatteryModel >= GetNBatteries() ) && ( dfBatteryModel != BATTERY_CUSTOM ))
		dfBatteryModel = 0;

	if ( dfStatus.phpct )
	{
		if (( dfPreheatPwr < 50 ) || ( dfPreheatPwr > 300 ))
			dfPreheatPwr = 100;
	}
	else
	{
		if ( dfPreheatPwr > MaxPower )
			dfPreheatPwr = dfPower;
	}

//	MemSet( DataFlash.p.UnusedCA, 0, sizeof(DataFlash.p.UnusedCA) );

	if ( dfPreheatTime > 200 )
		dfPreheatTime = 10;

	if ( dfTCAlgo >= TCALGO_MAX )
		dfTCAlgo = TCALGO_DEF;

	if ( dfTCBoost > 100 )
		dfTCBoost = 50;

	for ( i = 0 ; i < 3 ; ++i )
		if ( dfTCRP[i] > 999 )
			dfTCRP[i] = 0;

	if (( dfPID.P < PID_P_MIN || dfPID.P > PID_P_MAX )
	||	( dfPID.I > PID_I_MAX )
	||	( dfPID.D > PID_D_MAX ))
	{
		dfPID.P = PID_P_DEF;
		dfPID.I = PID_I_DEF;
		dfPID.D = PID_D_DEF;
	}

	for ( i = 0 ; i < 4 ; ++i )
	if ( ( dfBVOffset[i] < BVO_MIN ) || ( dfBVOffset[i] > BVO_MAX ) )
		dfBVOffset[i] = 0;

        if ( dfMaxBoardTemp > 99 || dfMaxBoardTemp < 20 ) dfMaxBoardTemp = 70;
        if ( dfMaxVolts > 999 ) dfMaxVolts = 999;
        if ( dfMaxPower > 5000 ) dfMaxVolts = 750;
        if ( dfUSBMaxCharge > 2000 ) dfUSBMaxCharge = 2000;
        
        if ( !dfColdLockTemp || dfColdLockTemp > 40 ) dfColdLockTemp = 20;
        if ( !dfNewRezPerc || dfNewRezPerc > 50 ) dfNewRezPerc = 5;
        
        if ( dfBattVolt < 250 || dfBattVolt > 420 ) dfBattVolt = 420 ;

}
*/

//=========================================================================
//----- (000018D0) --------------------------------------------------------
__myevic__ int FMCCheckConfig( unsigned long cfg[] )
{
	if ( cfg[0] & 1 || cfg[1] != DATAFLASH_PARAMS_BASE )
	{
		FMC_EnableConfigUpdate();
		FMC_Erase( FMC_CONFIG_BASE );

		cfg[0] &= ~1;
		cfg[1] = DATAFLASH_PARAMS_BASE;

		if ( FMC_WriteConfig( cfg, 2 ) < 0 )
			return 0;

		FMC_ReadConfig( cfg, 2 );

		if ( cfg[0] & 1 || cfg[1] != DATAFLASH_PARAMS_BASE )
			return 0;

		SYS_ResetChip();
	}
	return 1;
}


//=========================================================================
//----- (00001926) --------------------------------------------------------
__myevic__ void FMCRead256( uint32_t u32Addr, uint32_t *pu32Buf )
{
	for ( uint32_t offset = 0 ; offset < 0x100 ; offset += 4 )
	{
		*pu32Buf = FMC_Read( u32Addr + offset );
		++pu32Buf;
	}
}


//=========================================================================
//----- (00001CEC) --------------------------------------------------------
__myevic__ uint32_t ReadDataFlash( uint32_t u32Addr, uint32_t *pu32Buf )
{
	uint32_t offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SPACE ; offset += DATAFLASH_PARAMS_SIZE )
	{
		if ( FMC_Read( u32Addr + offset ) == ~0 && FMC_Read( u32Addr + offset + 4 ) == ~0 )
		{
			break;
		}
		offset += DATAFLASH_PARAMS_SIZE;
	}

	if ( offset )
	{
		u32Addr += offset - DATAFLASH_PARAMS_SIZE;
	}

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SIZE ; offset += 0x100 )
	{
		FMCRead256( u32Addr + offset, pu32Buf + offset / 4 );
	}

	return u32Addr;
}


//=========================================================================
//----- (0000119C) --------------------------------------------------------
__myevic__ uint32_t CalcPageCRC( uint32_t *pu32Addr )
{
	uint32_t idx;
	uint16_t *addr;
	uint32_t crc;

	CRC_Open( CRC_CCITT, 0, 0xFFFF, CRC_CPU_WDATA_16 );

	idx = 0;
	addr = (uint16_t*)(pu32Addr+1);

	do
	{
		CRC_WRITE_DATA( addr[idx] );
	}
	while ( ++idx < ( DATAFLASH_PARAMS_SIZE - 4 ) / 2 );

	crc = CRC_GetChecksum();

	CRC->CTL &= ~1;

	return crc;
}


//=========================================================================
//----- (00001FD0) --------------------------------------------------------
// Writes 256 bytes from address from pu32Data to first free page
// in DF after u32Addr
__myevic__ void WriteDataFlash( uint32_t u32Addr, const uint32_t *pu32Data )
{
	uint32_t offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SPACE ; offset += DATAFLASH_PARAMS_SIZE )
	{
		if ( FMC_Read( u32Addr + offset ) == ~0 && FMC_Read( u32Addr + offset + 4 ) == ~0 )
		{
			break;
		}
	}

	if ( offset >= DATAFLASH_PARAMS_SPACE )
	{
		offset = 0;
		FMC_Erase( u32Addr );
	}
	else if ( offset < DATAFLASH_PARAMS_SPACE - FMC_FLASH_PAGE_SIZE )
	{
		if ( offset % FMC_FLASH_PAGE_SIZE == DATAFLASH_PARAMS_SIZE )
		{
			FMC_Erase( u32Addr + offset - DATAFLASH_PARAMS_SIZE + FMC_FLASH_PAGE_SIZE );
		}
	}

	u32Addr += offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SIZE ; offset += 4 )
	{
		FMC_Write( u32Addr + offset, pu32Data[ offset / 4 ] );
	}
}


//=========================================================================
//----- (00001D30) --------------------------------------------------------
__myevic__ void UpdateDataFlash()
{
	uint8_t *df;
	uint32_t idx;

//	dfAtoRez = AtoRez;
//	dfAtoStatus = AtoStatus; not used in df
	UpdateDFTimer = 0;

	df = (uint8_t*)&DataFlash.params;

	for ( idx = 0 ; idx < DATAFLASH_PARAMS_SIZE ; ++idx )
	{
		if ( df[idx] != ParamsBackup[idx] )
			break;
	}

	if ( idx != DATAFLASH_PARAMS_SIZE )
	{
		dfCRC = CalcPageCRC( DataFlash.params );
		MemCpy( ParamsBackup, DataFlash.params, DATAFLASH_PARAMS_SIZE );
		SYS_UnlockReg();
		FMC_ENABLE_ISP();
		WriteDataFlash( DATAFLASH_PARAMS_BASE, DataFlash.params );
		FMC_DISABLE_ISP();
		SYS_LockReg();
	}      
}


//=========================================================================
//----- (00001940) --------------------------------------------------------
__myevic__ void InitDataFlash()
{
	int i;
	unsigned long cfg[2];
	unsigned long hwv;
	unsigned long addr;

	SYS_UnlockReg();
	FMC_ENABLE_ISP();

	MemClear( &DataFlash, sizeof( DataFlash ) );

	dffmcCID = FMC_ReadCID();

	FMC->ISPCMD = FMC_ISPCMD_READ_DID;
	FMC->ISPADDR = 0;
	FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
	while( FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk )
		;
	dffmcDID = FMC->ISPDAT;

	dffmcPID = FMC_ReadPID();

	for ( i = 0 ; i < 3 ; ++i )
	{
		dffmcUID[i] = FMC_ReadUID( i );
	}

	for ( i = 0 ; i < 4 ; ++i )
	{
		dffmcUCID[i] = FMC_ReadUCID( i );
	}

	FMC_ReadConfig( cfg, 2 );

	addr = DATAFLASH_PARAMS_BASE;

	if ( FMCCheckConfig( cfg ) )
	{
		addr = ReadDataFlash( addr, DataFlash.params );
	}

	if ( CalcPageCRC( DataFlash.params ) != dfCRC )
	{
		if ( addr == DATAFLASH_PARAMS_BASE )
		{
			addr = DATAFLASH_PARAMS_END - DATAFLASH_PARAMS_SIZE;
		}
		else
		{
			addr -= DATAFLASH_PARAMS_SIZE;
		}

		hwv = dfHWVersion;

		for ( i = 0 ; i < DATAFLASH_PARAMS_SIZE ; i += 0x100 )
		{
			FMCRead256( addr + i, &DataFlash.params[ i / 4 ] );
		}

		if ( CalcPageCRC( DataFlash.params ) == dfCRC )
		{
			UpdateDataFlash();
		}
		else
		{
			dfHWVersion = hwv;
		}
	}

	FMC_DISABLE_ISP();
	SYS_LockReg();

	FMCReadCounters();

	SetProductID();

	if ( ISVTWO || ISEVICAIO || ISCUBOMINI || ISEVICBASIC )
	{
		switch ( dfHWVersion )
		{
			case 101:
				DisplayModel = 1;
				break;
			default:
				DisplayModel = 0;
				break;
		}
	}
	else if ( ISPRESA75W )
	{
		switch ( dfHWVersion )
		{
			case 102:
			case 103:
				DisplayModel = 1;
				break;
			default:
				DisplayModel = 0;
				break;
		}
	}
	else if ( ISCUBOID )
	{
		switch ( dfHWVersion )
		{
			case 102:
				DisplayModel = 1;
				break;
			default:
				DisplayModel = 0;
				break;
		}
	}
	else if ( ISVTCMINI )
	{
		switch ( dfHWVersion )
		{
			case 102:
			case 103:
			case 106:
			case 108:
			case 109:
			case 111:
				DisplayModel = 1;
				break;
			default:
				DisplayModel = 0;
				break;
		}
	}
	else
	{
		DisplayModel = 0;
	}

	AtoShuntRez = GetShuntRezValue();

	dfFWVersion	= FWVERSION;
        dfBuild = __BUILD3;


        InitSetPowerVoltMax();

/*
	myprintf( "  APROM Version ......................... [%d.%d%d]\n",
				FWVERSION / 100,
				FWVERSION / 10 % 10,
				FWVERSION % 10 );
	myprintf( "  Hardware Version ...................... [%d.%d%d]\n",
				dfHWVersion / 100,
				dfHWVersion / 10 % 10,
				dfHWVersion % 10 );
*/


	if ( !( ( dfMagic == DFMagicNumber ) && ( CalcPageCRC( DataFlash.params ) == dfCRC ) ) )
	//{
	//	DFCheckValuesValidity();
	//}
	//else
	{
		//myprintf( "Data Flash Re-Initialization\n" );
		ResetDataFlash();
	}

	if ( X32Off )
	{
		dfStatus.x32off = 1;
	}

	dfStatus.off = 0;

	MemCpy( ParamsBackup, DataFlash.params, DATAFLASH_PARAMS_SIZE );

	if ( dfShuntRez != 0 )
	{
		AtoShuntRez = dfShuntRez;
	}

	if ( dfBootFlag )
	{
		dfBootFlag = 0;
		UpdateDFTimer = 1;
	}
}


//=========================================================================
//----- (0000169C) --------------------------------------------------------
// Writes 2kB from RAM R1 to DF R0
__myevic__ void FMCWritePage( uint32_t u32Addr, uint32_t *pu32Data )
{
	for ( uint32_t idx = 0 ; idx < FMC_FLASH_PAGE_SIZE / 4 ; ++idx )
	{
		FMC_Write( u32Addr + 4 * idx, pu32Data[ idx ] );
	}
}

//=========================================================================
//----- (000016D0) --------------------------------------------------------
// Compares 2kB (0x800) DF @R0 with RAM @R1
/*
__myevic__ uint32_t FMCVerifyPage( uint32_t u32Addr, uint32_t *pu32Data )
{
	for ( uint32_t idx = 0 ; idx < FMC_FLASH_PAGE_SIZE / 4 ; ++idx )
	{
		if ( FMC_Read( u32Addr + 4 * idx ) != pu32Data[ idx ] )
		{
			return idx + 1;
		}
	}
	return 0;
}
*/


//=========================================================================
//----- (0000170C) --------------------------------------------------------
// Erase & writes 2kB from RAM R1 to DF R0
__myevic__ int FMCEraseWritePage( uint32_t u32Addr, uint32_t *src )
{
	if ( FMC_Erase( u32Addr ) == -1 )
	{
		return 1;
	}
	else
	{
		FMCWritePage( u32Addr, src );
		return 0;
	}
}


//=========================================================================
//----- (00002030) --------------------------------------------------------
__myevic__ void DataFlashUpdateTick()
{
	//if ( UpdateDFTimer )
        if ( gFlags.firing )
	{
		//if ( !--UpdateDFTimer )
		//UpdateDataFlash();
                UpdateDFTimer = 50;
	}
	//if ( UpdatePTTimer )
        else
	{
		//if ( !--UpdatePTTimer )
		//UpdatePTCounters();
		if ( UpdateDFTimer )
		{
			if ( !--UpdateDFTimer )
			UpdateDataFlash();
 		}
 		if ( UpdatePTTimer )
 		{
 			if ( !--UpdatePTTimer )
 			UpdatePTCounters();
 		}
	}
}


//=========================================================================
// Set the shunt resistance value
//-------------------------------------------------------------------------
__myevic__ uint16_t GetShuntRezValue()
{
	uint16_t rez;

	if ( ISPRESA75W || ISEVICAIO || ISRXMINI || ISSINP80 || ISSINFJ200 || ISGEN2 )
	{
		rez = 100;
	}
        else if ( ISPREDATOR )
        {
            	switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 100;
				break;

			case 101:
				rez = 103;
				break;
		}
        }
        else if ( ISIKU200 )
        {
                rez = 102;
        }        
        else if ( ISRX2 )
        {
                rez = 103;
        }
        else if ( ISRX217 )
        {
                rez = 101;
        }        
        else if ( ISINVOKE )
        {
                rez = 97;            
        }
	else if ( ISPRESA100W )
	{
		rez = 92;
	}
	else if ( ISVTWO || ISCUBO200 )
	{
		rez = 115;
	}
	else if ( ISVTWOMINI )
	{
		switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 115;
				break;

			case 101:
				rez = 119;
				break;
		}
	}
	else if ( ISPRIMOSE )
 	{
		rez = 109;
	}
	else if ( ISPRIMOMINI )
 	{
            	switch ( dfHWVersion )
		{
			case 100:
                            rez = 109;
                            break;
			case 101:
                            rez = 107;
                            break;
                }
	}       
	else if ( ISEGRIPII || ISEVICBASIC )
	{
		rez = 120;
	}
	else if ( ISCUBOMINI )
	{
		switch ( dfHWVersion )
		{
			case 100:
			case 101:
			default:
				rez = 100;
				break;

			case 102:
				rez = 105;
				break;
		}
	}
	else if ( ISCUBOID )
	{
		rez = 105;
	}
	else if ( ISWRX75TC )
	{
		switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 123;
				break;

			case 101:
				rez = 107;
				break;
		}
	}
	else if ( ISVTCDUAL )
	{
		switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 107;
				break;

			case 101:
				rez = 105;
				break;
		}
	}
	else if ( ISPRIMO1 )
	{
                switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 108;
				break;

			case 101:
				rez = 111;
				break;
		}
	}
	else if ( ISPRIMO2 )
	{
                switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 110;
				break;

			case 101:
				rez = 111;
				break;
		}
	}
	else if ( ISRX23 )
	{
		switch ( dfHWVersion )
		{
			case 100:
			default:
				rez = 112;
				break;

			case 101:
				rez = 109;
				break;
		}
	}
	else if ( ISRX200S )
	{
		rez = 110;
	}
	else if ( ISRX300 )
	{
		rez = 106;
	}
        else if ( ISGEN3 )
        {
                rez = 95;
        }
        else if ( ISFIT )
        {
                rez = 104;            
        }
	else //vtc mini
	{
		switch ( dfHWVersion )
		{
			case 100:
			case 102:
			default:
				rez = 115;
				break;
			case 101:
				rez = 125;
				break;
			case 108:
				rez = 125;
				break;
			case 103:
			case 104:
			case 105:
			case 106:
				rez = 110;
				break;
			case 107:
			case 109:
				rez = 120;
				break;
			case 110:
			case 111:
				rez = 105;
				break;
		}
	}

	return rez;
}


//=========================================================================
// Profile management
//-------------------------------------------------------------------------
// Reload data filter.
// Filters relevant dataflash data for storage in a profile.
// This array will have to be extended if parameters comes to be bigger
// than 256 bytes one day.
//-------------------------------------------------------------------------
const uint8_t ProfileFilter[32] =
{
/* 0000 */	0b00000000,
/* 0008 */	0b00101111, //8 9 A B C D E F   see typedef struct dfParams in dataflash.h
/* 0010 */	0b11111111,
/* 0018 */	0b11111111,
/* 0020 */	0b00000000,
/* 0028 */	0b00000000,
/* 0030 */	0b00000000,
/* 0038 */	0b00111111,
/* 0040 */	0b11111111,
/* 0048 */	0b11111100,
/* 0050 */	0b00000000,
/* 0058 */	0b00000000,
/* 0060 */	0b00000000,
/* 0068 */	0b00000000,
/* 0070 */	0b00001011,
/* 0078 */	0b11110000,     //status
/* 0080 */	0b11111011,
/* 0088 */	0b11111110,
/* 0090 */	0b10000000,
/* 0098 */	0b00000000,
/* 00A0 */	0b00000000,
/* 00A8 */	0b00000000,
/* 00B0 */	0b00000000,
/* 00B8 */	0b00000101,
/* 00C0 */	0b00111000,
/* 00C8 */	0b00000111,
/* 00D0 */	0b11111111,
/* 00D8 */	0b11111111,
/* 00E0 */	0b11100000,
/* 00E8 */	0b00000100,     // APT3
/* 00F0 */	0b00001111,     // status2
/* 00F8 */	0b00111100      // max v w
};
// Saved status bits
//                                 3         2         1          
//                               21098765432109876543210987654321
const uint32_t  StatusFilter = 0b10010000000111001110111111001010;
//                                      3       2       1       0  
//                               84218421842184218421842184218421
const uint32_t StatusFilter2 = 0b00000000000110001111100110000101;

//-------------------------------------------------------------------------
// Apply newly reloaded parameters
//-------------------------------------------------------------------------
__myevic__ void ApplyParameters()
{
    //from LoadProfile
	InitDisplay();
	//LEDGetColor();
        InitSetPowerVoltMax();
        InitVariables();
	SetBatteryModel();
        dfRezVW = 0; //not use locked vvres (while it not in DF r                                            eally) on profile change
	ModeChange();

	gFlags.refresh_display = 1;
}


//-------------------------------------------------------------------------
// Restore a given profile
//-------------------------------------------------------------------------
__myevic__ void LoadProfile( int p )
{
	uint32_t addr, idx;
	dfParams_t *params;
	uint8_t *s, *d, *s2, *d2;

	//if ( p >= DATAFLASH_PROFILES_MAX )
	//	return;

	addr = DATAFLASH_PROFILES_BASE + p * DATAFLASH_PARAMS_SIZE;

	params = (dfParams_t*)addr;

	if (( params->Magic == DFMagicNumber ) && ( params->PCRC == CalcPageCRC( (uint32_t*)params ) ))
	{
		s = (uint8_t*)params;
		d = (uint8_t*)DataFlash.params;
		s2 = (uint8_t*)params;
		d2 = (uint8_t*)DataFlash.params;
                
		uint32_t new_status = *(uint32_t*)&s[offsetof(dfParams_t,Status)];
		uint32_t old_status = *(uint32_t*)&d[offsetof(dfParams_t,Status)];
		uint32_t new_status2 = *(uint32_t*)&s2[offsetof(dfParams_t,Status2)];
		uint32_t old_status2 = *(uint32_t*)&d2[offsetof(dfParams_t,Status2)];
                
		new_status &= StatusFilter;
		old_status &= ~StatusFilter;
		new_status2 &= StatusFilter2;
		old_status2 &= ~StatusFilter2;
                
		for ( idx = 0 ; idx < DATAFLASH_PARAMS_SIZE ; ++idx )
                {
			if ( ProfileFilter[idx/8] & ( 0x80 >> ( idx & 7 ) ) )
                        {
				d[idx] = s[idx];
                                d2[idx] = s2[idx];
                        }
                }
		*(uint32_t*)&d[offsetof(dfParams_t,Status)] = old_status | new_status;
                *(uint32_t*)&d2[offsetof(dfParams_t,Status2)] = old_status2 | new_status2;                
                                
		//DFCheckValuesValidity();
		ApplyParameters();
	}

	dfProfile = p;
	UpdateDataFlash();
}


//-------------------------------------------------------------------------
// Save the current profile
//-------------------------------------------------------------------------
__myevic__ void SaveProfile()
{
	uint8_t *df;
	uint8_t *profile;

	uint32_t idx;
	uint32_t offset, addr;

	uint8_t page[FMC_FLASH_PAGE_SIZE] __attribute__((aligned(4)));

	offset = dfProfile * DATAFLASH_PARAMS_SIZE;
	addr   = DATAFLASH_PROFILES_BASE + offset;

	profile = (uint8_t*)addr;

	df = (uint8_t*)&DataFlash.params;

	// Only save profile if some of the relevant data has been modified

	for ( idx = 0 ; idx < DATAFLASH_PARAMS_SIZE ; ++idx )
	{
		if ( ProfileFilter[idx/8] & ( 0x80 >> ( idx & 7 ) ) )
			if ( df[idx] != profile[idx] )
				break;
	}

	if ( idx != DATAFLASH_PARAMS_SIZE )
	{
		dfCRC = CalcPageCRC( DataFlash.params );

		MemCpy( page, (void*)DATAFLASH_PROFILES_BASE, FMC_FLASH_PAGE_SIZE );
		MemCpy( page + offset, df, DATAFLASH_PARAMS_SIZE );

		SYS_UnlockReg();
		FMC_ENABLE_ISP();
		FMC_ENABLE_AP_UPDATE();

		FMCEraseWritePage( DATAFLASH_PROFILES_BASE, (uint32_t*)page );

		FMC_DISABLE_AP_UPDATE();
		FMC_DISABLE_ISP();
		SYS_LockReg();
	}
}

//-------------------------------------------------------------------------
// Erase a profile
//-------------------------------------------------------------------------
__myevic__ void EraseProfile( int p )
{
	uint32_t offset;

	uint8_t page[FMC_FLASH_PAGE_SIZE] __attribute__((aligned(4)));
	
	offset = p * DATAFLASH_PARAMS_SIZE;
	
	MemCpy( page, (void*)DATAFLASH_PROFILES_BASE, FMC_FLASH_PAGE_SIZE );
	MemSet( page + offset, 0xFF, DATAFLASH_PARAMS_SIZE );
	
	SYS_UnlockReg();
	FMC_ENABLE_ISP();
	FMC_ENABLE_AP_UPDATE();
	
	FMCEraseWritePage( DATAFLASH_PROFILES_BASE, (uint32_t*)page );
	
	FMC_DISABLE_AP_UPDATE();
	FMC_DISABLE_ISP();
	SYS_LockReg();
}
        
//-------------------------------------------------------------------------
// Test a given profile
//-------------------------------------------------------------------------
__myevic__ int IsProfileValid( int p )
{
	uint32_t addr;
	dfParams_t *params;
        
	//if ( p >= DATAFLASH_PROFILES_MAX )
	//	return 0;
        
	addr = DATAFLASH_PROFILES_BASE + p * DATAFLASH_PARAMS_SIZE;
	params = (dfParams_t*)addr;
	if (( params->Magic == DFMagicNumber ) && ( params->PCRC == CalcPageCRC( (uint32_t*)params ) ))
	{
		return 1;
	}
	return 0;
}

__myevic__ uint16_t GetProfileRes( int p )
{
	uint32_t addr;
	dfParams_t *params;
        // 0018	uint16_t	Resistance;
        
        if ( IsProfileValid( p ) )
        {
            addr = DATAFLASH_PROFILES_BASE + p * DATAFLASH_PARAMS_SIZE;
            params = (dfParams_t*)addr;
        
            return params->Resistance;
        }
        else
        {
            return 0;
        }
}

__myevic__ void SetProfile()
{
    //auto set profile by resistance
    
    uint16_t r;
   
    for ( int i = 0; i < DATAFLASH_PROFILES_MAX; ++i )
    {
        r = GetProfileRes( i );
        
        if ( 
           r 
           && ( AtoRez - AtoRez * dfNewRezPerc / 100 <= r )
           && ( AtoRez + AtoRez * dfNewRezPerc / 100 >= r )                
           )
        {
            if ( dfProfile != i ) LoadProfile( i );
            return;
        }
                                
    }
}
