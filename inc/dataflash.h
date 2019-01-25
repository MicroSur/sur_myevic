#ifndef __DATAFLASH_H__
#define __DATAFLASH_H__

#include "M451Series.h"

//=========================================================================
// Configuration constants
//-------------------------------------------------------------------------

// End of APROM address (on a 128k chip)
#define DATAFLASH_APROM_END			0x00020000

//#define FMC_FLASH_PAGE_SIZE     0x800           /*!< Flash Page Size (2048 Bytes) */
//#define FMC_LDROM_SIZE          0x1000          /*!< LDROM Size (4 kBytes)       */

// Time & Puffs counters space
#define DATAFLASH_TIMECNTR_BASE		(DATAFLASH_APROM_END-FMC_FLASH_PAGE_SIZE)
#define DATAFLASH_PUFFCNTR_BASE		(DATAFLASH_TIMECNTR_BASE-FMC_FLASH_PAGE_SIZE)

// Available space for parameters
#define DATAFLASH_PARAMS_SPACE		(2*FMC_FLASH_PAGE_SIZE)
#define DATAFLASH_PARAMS_END		DATAFLASH_PUFFCNTR_BASE
#define DATAFLASH_PARAMS_BASE		(DATAFLASH_PARAMS_END-DATAFLASH_PARAMS_SPACE)

//  DATAFLASH_PARAMS_BASE 
//= DATAFLASH_PARAMS_END - DATAFLASH_PARAMS_SPACE
//= DATAFLASH_PUFFCNTR_BASE - 2*FMC_FLASH_PAGE_SIZE 
//= DATAFLASH_TIMECNTR_BASE - FMC_FLASH_PAGE_SIZE - 2*FMC_FLASH_PAGE_SIZE 
//= DATAFLASH_APROM_END - FMC_FLASH_PAGE_SIZE - FMC_FLASH_PAGE_SIZE - 2*FMC_FLASH_PAGE_SIZE 
//= 0x20000 - 0x800 - 0x800 - 2*0x800
//= 0x1E000
        
// Profiles
#define DATAFLASH_PROFILES_SPACE	FMC_FLASH_PAGE_SIZE
#define DATAFLASH_PROFILES_END		DATAFLASH_PARAMS_BASE
#define DATAFLASH_PROFILES_BASE		(DATAFLASH_PROFILES_END-DATAFLASH_PROFILES_SPACE)

#define DATAFLASH_PROFILES_MAX		5

// Custom Logo space
// Those addresses are hardcoded in Joyetech's custom logo utility so
//  we have to hardcode them here too.
#define DATAFLASH_LOGO_SIZE			0x200
//#define DATAFLASH_LOGO_1327_BASE	0x19000 // 102400
#define DATAFLASH_LOGO_1306_BASE	0x19200

// To read product ID
#define LDROM_BASE 0x100000
#define LDROM_SIZE 0x1000

// NFE Magic Number
//#define DATAFLASH_NFE_MAGIC	0xFE


//=========================================================================
// DataFlash content structure
// Padding bytes added by compiler to ensure correct alignements are
//  indicated and can eventually be used to store usefull parameters
//  without increasing the total data size.
// Useless fields may also be recycled.
//-------------------------------------------------------------------------

typedef struct
{
/* 00000001 */	unsigned int off:1;         //lsls 0x1F
/* 00000002 */	unsigned int keylock:1;     //lsls 0x1E
/* 00000004 */	unsigned int flipped:1;     //lsls 0x1D
/* 00000008 */	unsigned int nologo:1;      //lsls 0x1C 3-d bit for menu
/* 00000010 */	unsigned int clock:1;       //lsls 0x1B 4
/* 00000020 */	unsigned int vcom:1;        //lsls 0x1A // deleted
/* 00000040 */	unsigned int autopuff:1;     //lsls 0x19 //was storage
/* 00000080 */	unsigned int endlessfire:1;      //lsls 0x18

/* 00000100 */	unsigned int x32off:1;      //lsls 0x17
/* 00000200 */	unsigned int priopwr:1;     //lsls 0x16
/* 00000400 */	unsigned int onewatt:1;     //lsls 0x15     //10 bit for menu
/* 00000800 */	unsigned int digclk:1;      //lsls 0x14
/* 00001000 */	unsigned int nbrc:1;      //lsls 0x13        new battery reset counters
/* 00002000 */	unsigned int phpct:1;       //lsls 0x12     // preheat in percent or watts
/* 00004000 */	unsigned int wakeonpm:1;    //lsls 0x11
/* 00008000 */	unsigned int preheat:1;     //lsls 0x10  15 bit

/* 00010000 */	unsigned int IsCelsius:1;   //lsls 0x0F 
/* 00020000 */	unsigned int dfmt1:1;       //lsls 0x0E data format 1(2) bit
/* 00040000 */	unsigned int invert:1;      //lsls 0x0D
/* 00080000 */	unsigned int puffday:1;     //lsls 0x0C     was vapedml
/* 00100000 */	unsigned int onedegree:1;   //lsls 0x0B
/* 00200000 */	unsigned int usbchghotoff:1;       //lsls 0x0A
/* 00400000 */	unsigned int lsloff:1;      //lsls 0x09
/* 00800000 */	unsigned int logomid:1;     //lsls 0x08  23-d bit

/* 01000000 */	unsigned int timebig:1;     //lsls 0x07
/* 02000000 */	unsigned int usbchgoff:1;   //lsls 0x06
/* 04000000 */	unsigned int chkmodeoff:1;  //lsls 0x05
/* 08000000 */	unsigned int dfmt2:1;       //lsls 0x04     data format 2(2) bit
/* 10000000 */	unsigned int pcurve:1;      //lsls 0x03
/* 20000000 */  unsigned int offmodclock:1; //lsls 0x02 29
/* 40000000 */  unsigned int fireflip:1;   //lsls 0x01
/* 80000000 */  unsigned int vvlite:1;      //lsls 0    31
// Do not exceed 32 bits;
// if you may do so, create another bitfield.
}
dfStatus_t;

typedef struct
{
/* 00000001 */	unsigned int anim3d:1;         //lsls 0x1F  0-bit
/* 00000002 */	unsigned int vapedjoules:1;  // not used
/* 00000004 */	unsigned int smart_ph:1;
/* 00000008 */	unsigned int swap_mp:1;
/* 00000010 */	unsigned int digclk2:1;
/* 00000020 */	unsigned int splash0:1;         // 0-00 off; 1-01 on; 2-10 on with box name //11
/* 00000040 */	unsigned int splash1:1;
/* 00000080 */	unsigned int bybatts:1;     //batteries voltage for bypass mode, 0 - 1batt, 1 - all batts

/* 00000100 */	unsigned int vwrezlock:1;  //lock res in VW like in TC
/* 00000200 */	unsigned int reztype:1;     //RezType 1 - TC, 0 - not TC wire
/* 00000400 */	unsigned int vapedelay:1;     //flag 
/* 00000800 */	unsigned int dfRezLockedNI:1;
/* 00001000 */	unsigned int dfRezLockedTI:1;
/* 00002000 */	unsigned int dfRezLockedSS:1;
/* 00004000 */	unsigned int dfRezLockedTCR:1;       
/* 00008000 */	unsigned int replay:1;          //15 bit
/* 00010000 */	unsigned int session:1;
/* 00020000 */	unsigned int garbage:1;         //17
/* 00040000 */	unsigned int autoprofile:1;
/* 00080000 */	unsigned int pwrlow:1;          // 19 powerscale on-off
/* 00100000 */	unsigned int smart_ph2:1;
}
// Do not exceed 32 bits;
dfStatus2_t;

typedef struct
{
	uint16_t P;
	uint16_t I;
	uint16_t D;
}
dfPID_t;

typedef struct
{
	uint8_t	percent[9];
	uint8_t voltage[11];
	uint8_t	cutoff;
	uint8_t	maxamp;
}
dfBattery_t;

#define PWR_CURVE_PTS	20 //10

typedef struct
{
	uint8_t time;
	uint8_t power;
}
dfPCPoint_t;

//__attribute__((packed)) - big file, but no padding (+6 bytes)
typedef struct dfParams
{
/* 0000 */	uint32_t	PCRC;
/* 0004 */	uint32_t	HWVersion;
/* 0008 */	uint8_t		Magic;                  // = DFMagicNumber;
/* 0009 */	uint8_t		BootFlag;
/* 000A */	uint8_t		Mode;                   // 0 Ni, 1 Ti, 2 SS, 3 TCR, 4 pwr, 5 by, 6 smart
/* 000B */	uint8_t		Protec;			//	former 1-byte pad
/* 000C */	uint16_t	Power;                  //df VW power set
/* 000E */	uint16_t	Temp;                   //df TC temperature set
/* 0010 */	uint16_t	TCPower;                //df TC power set
/* 0012 */	uint16_t	AwakingTimer;           //was VWVolts
/* 0014 */	uint8_t		APT;
/* 0015 */	uint8_t		VWTempAlgo;             // 1 Ni, 2 Ti, 3 SS, 4 M1, 5 M2, 6 M3, 7 tcrNi, 8 tcrTi, 9 tcrSS
/* 0016 */	uint8_t		TempAlgo;               // same
/* 0017 */	uint8_t		CUDelay;                // was isCelsius
/* 0018 */	uint16_t	Resistance;
/* 001A */	uint16_t	RezTI;
/* 001C */	uint16_t	RezNI;
/* 001E */	uint16_t	ReplayRez;              // was uint8_t RezLockedTI uint8_t RezLockedNI
/* 0020 */	uint8_t		MaxBoardTemp;	
/* 0021 */	uint8_t		StealthOn;              // 0 1 2 (off on contrast)
/* 0022 */	uint16_t	VVLockedVolt;           // for vvlite: cold rez locked volt
/* 0024 */	dfBattery_t	Battery;                //custom
/* 003A */	dfPCPoint_t	PwrCurve[PWR_CURVE_PTS]; //20
/* 0062 */	uint16_t	VapeHoldTimer;          //dfVapeDelayTimer Status2.vapedelay
/* 0064 */      uint32_t	JoulesEnergy;                 // Joules for energy separated from Joules for vaped
/* 0068 */      uint32_t	JoulesDay;                 // save for mods without clock battery
/* 006C */	uint8_t		ThreeButtonsAct;        //action
/* 006D */	uint8_t		StealthPuffsCnt;        //stealth delay in puffs
/* 006E */	uint8_t		ScrChargeTime;          // timeout for charge screen
/* 006F 00 padding */
/* 0070 */      uint32_t	Joules;                 // save for mods without clock battery
/* 0074 */	uint8_t         CurveRepeatTimer;            // 0.1s = 1 1s = 10...    was TempCoefsTi
/* 0075 */	int8_t          AkkuTempCorr;  
/* 0076 */	uint16_t	LEDColor;		//	former 2-bytes pad
/* 0078 */	dfStatus_t	Status;                 //4 bytes
/* 007C */	uint8_t         BattLine;		//0- %+V,1- %+small,2-v+small,3-all_small	//was uint16_t	AtoRez;
/* 007D */	uint8_t         AutoPuffTimer;
/* 007E */	uint8_t		NewRezPerc;		//	was AtoStatus
/* 007F */	uint8_t		ShuntRez;		//	former 1-byte pad
/* 0080 */	uint16_t	RezSS;
/* 0082 */	uint16_t	ReplayPower;            //was RezLockedSS UIVersion
/* 0084 */	uint8_t		TCRIndex;               // 0 m1, 1 m2, 2 m3
/* 0085 */	uint8_t		ScrMainTime;            //	former 1-byte pad
/* 0086 */	uint16_t	TCRM[3];                // custom TCR M1 M2 M3
/* 008C */	uint16_t	RezTCR;
/* 008E */	uint8_t		ReplayMillis;           //    for ReplayRez 4 digits (1/2 byte need) //was RezLockedTCR
/* 008F */	uint8_t		ScreenSaver;
/* 0090 */	uint8_t		TCMode;                 // 0 1 2
/* 0091 */	uint8_t		ScreenProt;		//	former 1-byte pad
/* 0092 */	uint16_t	SavedCfgRez[10];
/* 00A6 */	uint16_t	SavedCfgPwr[10];
/* 00BA */	uint16_t	FBBest;
/* 00BC */	uint8_t		UIVersion;              // 0 1 now // was FBSpeed
/* 00BD */	uint8_t		ColdLockTemp;
/* 00BE */	uint8_t		Contrast;               //value
/* 00BF */	uint8_t		ModesSel;
/* 00C0 */	uint16_t	ClkRatio;
/* 00C2 */	uint16_t	VVRatio;                //calc vaped
/* 00C4 */	uint8_t		PHDelay;
/* 00C5 */	uint8_t		Clicks[4];
/* 00C9 */	uint8_t		BatCutOff;             //was DimTimeout
/* 00CA */	uint8_t		BatteryModel;
/* 00CB 00 padding */
/* 00CC */	uint16_t	USBMaxCharge; // 8 bit??? /10
/* 00CE */	uint8_t		PreheatTime;
/* 00CF 00 padding */
/* 00D0 */	uint16_t	PreheatPwr;
/* 00D2 */	uint16_t	BattVolt;  // to 8 bit +250 ?
/* 00D4 */	uint8_t		TCAlgo;
/* 00D5 */	uint8_t		TCBoost;
/* 00D6 */	uint16_t	TCRP[3];        //custom TCR for Ni Ti SS
/* 00DC */	dfPID_t		PID;
/* 00E2 */	uint16_t	Millis;         //store 0.00x Ohm for all TC res
/* 00E4 */	uint8_t		Profile;
/* 00E5 */	int8_t		BVOffset[4];
/* 00E9 00 padding */ 
/* 00EA */	uint16_t	TTBest;
/* 00EC */	uint8_t		TTSpeed; //todo del
/* 00ED */	uint8_t		APT3;
/* 00EE */	uint8_t		PuffsOff;
/* 00EF */	uint8_t		HideLogo;
/* 00F0 */	int8_t		BoardTempCorr;
/* 00F1 */	uint8_t		Contrast2;
/* 00F2 00 padding */
/* 00F3 00 padding */
/* 00F4 */	dfStatus2_t	Status2;
/* 00F8 */      uint8_t         FireScrDuration;
/* 00F9 */      uint8_t         dfObject3D;
/* 00FA */      uint16_t        MaxPower;
/* 00FC */      uint16_t        MaxVolts;
/* 00FE */      uint8_t         DimOffTimeout;          //   0-255 0-4:15  ( x \ 60 & ":" & (x Mod 60) )
/* 00FF */      uint8_t         DimOffMode;             // 0 normal sleep, 1 - mod off, 2 - lock
// stop on FF
}
dfParams_t;


//-------------------------------------------------------------------------
// Information part
// This part is stored in RAM but never written to the DataFlash.
// It contains usefull infos that can be retreived in plus of the
//  parameters part by using the HID_CMD_GETINFO usb command.
// Used by firwmare update software.
//-------------------------------------------------------------------------

typedef struct
// 0x100 in dataflash
{
/* 0000 */	uint32_t	FWVersion;
/* 0004 */	uint32_t	LDVersion;
/* 0008 */	uint32_t	fmcCID;
/* 000C */	uint32_t	fmcDID;
/* 0010 */	uint32_t	fmcPID;
/* 0014 */	uint32_t	fmcUID[3];
/* 0020 */	uint32_t	fmcUCID[4];
/* 0030 */	uint32_t	PuffCount;
/* 0034 */	uint32_t	TimeCount;
/* 0038 */	uint32_t	ProductID;
/* 003C */	uint32_t	MaxHWVersion;
/* 0040 */	uint16_t	Year;
/* 0042 */	uint8_t		Month;
/* 0043 */	uint8_t		Day;
/* 0044 */	uint8_t		Hour;
/* 0045 */	uint8_t		Minute;
/* 0046 */	uint8_t		Second;
/* 0047 */	uint8_t		Format;
/* 0048 */	//uint32_t	Build;
}
dfInfos_t;


//-------------------------------------------------------------------------
// Second params block
// 0x200 in dataflash
//-------------------------------------------------------------------------
//This part is stored in RAM only too/
//how to write this to DF? todo 

typedef struct
{
/* 0000 */      uint32_t	Build;
/* 0004 */	uint16_t	RezVW;
/* 0005 */      
}
dfParams2_t;


#define ALIGN256(x) (((x)&0xff)?((x)-((x)&0xff)+0x100):(x))

#define DATAFLASH_PARAMS_SIZE	ALIGN256(sizeof(dfParams_t))
#define DATAFLASH_INFOS_SIZE	ALIGN256(sizeof(dfInfos_t))
#define DATAFLASH_PARAMS2_SIZE	ALIGN256(sizeof(dfParams2_t))
#define DATAFLASH_FREE_SIZE	(FMC_FLASH_PAGE_SIZE-DATAFLASH_PARAMS_SIZE-DATAFLASH_INFOS_SIZE-DATAFLASH_PARAMS2_SIZE)

//-------------------------------------------------------------------------
// It's important for DATAFLASH_PARAMS_SIZE to be a divider of
//	DATAFLASH_PARAMS_SPACE for all the functions of cycling to work
//	properly. Unfortunatly, we can't check this in the preprocessor
//	due to the sizeof() operation.
// It won't be a problem as long as the size of the parameters do not
//	exceed 0x200 bytes. Just be aware.
// In any case, parameters size should never exceed 0x500 bytes.
//-------------------------------------------------------------------------


//=============================================================================

typedef union
{
	int8_t c[0x100];
	uint8_t uc[0x100];
	int16_t s[0x100/sizeof(int16_t)];
	uint16_t us[0x100/sizeof(uint16_t)];
	int32_t l[0x100/sizeof(int32_t)];
	uint32_t ul[0x100/sizeof(uint32_t)];
} Playfield_t;


typedef struct dfStruct
{
	uint32_t Checksum;
	union
	{
		dfParams_t	p;
		uint32_t	params[DATAFLASH_PARAMS_SIZE/4];
	};
	union
	{
		dfInfos_t	i;
		uint32_t	infos[DATAFLASH_INFOS_SIZE/4];
	};
	union
	{
		dfParams2_t	p2;
		uint32_t	params2[DATAFLASH_PARAMS2_SIZE/4];
	};
	union
	{
		uint32_t	free_pages[DATAFLASH_FREE_SIZE/4];
		Playfield_t	playfield;
	};
}
dfStruct_t;


//-------------------------------------------------------------------------

extern dfStruct_t DataFlash;

//-------------------------------------------------------------------------
// Accessor macros
// Used for better code readability
//-------------------------------------------------------------------------


#define DFP(x) DataFlash.p.x
#define DFI(x) DataFlash.i.x
#define DFP2(x) DataFlash.p2.x

#define dfChecksum		DataFlash.Checksum
#define dfCRC			DFP(PCRC)
#define dfHWVersion		DFP(HWVersion)
#define dfMagic			DFP(Magic)
#define dfBootFlag		DFP(BootFlag)
#define dfMode			DFP(Mode)
#define dfProtec		DFP(Protec)
#define dfPower			DFP(Power)
#define dfTemp			DFP(Temp)
#define dfTCPower		DFP(TCPower)
//#define dfVWVolts		DFP(VWVolts)
#define dfAwakeTimer            DFP(AwakingTimer)
#define dfAPT			DFP(APT)
#define dfAPT3			DFP(APT3)
#define dfVWTempAlgo            DFP(VWTempAlgo)
#define dfTempAlgo		DFP(TempAlgo)
#define dfCUDelay		DFP(CUDelay)
#define dfResistance            DFP(Resistance)
#define dfRezTI			DFP(RezTI)
#define dfRezNI			DFP(RezNI)
#define dfReplayRez             DFP(ReplayRez)
#define dfStealthOn		DFP(StealthOn)
#define dfVVLockedVolt          DFP(VVLockedVolt)
#define dfBattery		DFP(Battery)
#define dfPwrCurve		DFP(PwrCurve)
#define dfThreeButtonsAct	DFP(ThreeButtonsAct)
#define dfStealthPuffsCnt       DFP(StealthPuffsCnt)
#define dfScrChargeTime		DFP(ScrChargeTime)
#define dfJoules		DFP(Joules)
#define dfJoulesDay		DFP(JoulesDay)
#define dfJoulesEnergy		DFP(JoulesEnergy)
#define dfVapeDelayTimer	DFP(VapeHoldTimer)
#define dfCurveRepeatTimer	DFP(CurveRepeatTimer)
#define dfLEDColor		DFP(LEDColor)
#define dfStatus		DFP(Status)
#define dfStatus2		DFP(Status2)
#define dfBattLine		DFP(BattLine)  //was dfAtoRez
#define dfAutoPuffTimer		DFP(AutoPuffTimer)
#define dfNewRezPerc		DFP(NewRezPerc) //was dfAtoStatus
#define dfShuntRez		DFP(ShuntRez)
#define dfRezSS			DFP(RezSS)
#define dfReplayPower           DFP(ReplayPower)
#define dfUIVersion		DFP(UIVersion)
#define dfTCRIndex		DFP(TCRIndex)
#define dfScrMainTime           DFP(ScrMainTime)
#define dfTCRM			DFP(TCRM)
#define dfRezTCR		DFP(RezTCR)
#define dfReplayMillis          DFP(ReplayMillis)
#define dfScreenSaver           DFP(ScreenSaver)
#define dfTCMode		DFP(TCMode)
#define dfScreenProt            DFP(ScreenProt)
#define dfSavedCfgRez           DFP(SavedCfgRez)
#define dfSavedCfgPwr           DFP(SavedCfgPwr)
#define dfFBBest		DFP(FBBest)
//#define dfFBSpeed		DFP(FBSpeed)
#define dfTTBest		DFP(TTBest)
#define dfTTSpeed		DFP(TTSpeed)        //todo del
#define dfColdLockTemp		DFP(ColdLockTemp)
#define dfContrast		DFP(Contrast)
#define dfContrast2		DFP(Contrast2)
#define Object3D		DFP(dfObject3D)
#define dfModesSel		DFP(ModesSel)
#define dfClkRatio		DFP(ClkRatio)
#define dfVVRatio		DFP(VVRatio)
#define dfPHDelay		DFP(PHDelay)
#define dfClick			DFP(Clicks)
#define dfBatCutOff             DFP(BatCutOff)
#define dfBatteryModel          DFP(BatteryModel)
#define dfBVOffset		DFP(BVOffset)
#define dfPreheatTime           DFP(PreheatTime)
#define dfPreheatPwr            DFP(PreheatPwr)
#define dfBattVolt              DFP(BattVolt)
#define dfTCAlgo		DFP(TCAlgo)
#define dfTCBoost		DFP(TCBoost)
#define dfTCRP			DFP(TCRP)
#define dfPID			DFP(PID)
#define dfMillis		DFP(Millis)
#define dfProfile		DFP(Profile)
#define dfPuffsOff  		DFP(PuffsOff)
#define dfHideLogo  		DFP(HideLogo)
#define dfBoardTempCorr 	DFP(BoardTempCorr)
#define dfAkkuTempCorr          DFP(AkkuTempCorr)
#define dfFireScrDuration       DFP(FireScrDuration)
#define dfMaxPower              DFP(MaxPower)
#define dfMaxVolts              DFP(MaxVolts)
#define dfUSBMaxCharge          DFP(USBMaxCharge)
#define dfMaxBoardTemp          DFP(MaxBoardTemp)
#define dfDimOffTimeout         DFP(DimOffTimeout)
#define dfDimOffMode            DFP(DimOffMode)


#define dfFWVersion	DFI(FWVersion)
#define dffmcCID        DFI(fmcCID)
#define dffmcDID        DFI(fmcDID)
#define dffmcPID        DFI(fmcPID)
#define dffmcUID        DFI(fmcUID)
#define dffmcUCID       DFI(fmcUCID)
#define dfPuffCount     DFI(PuffCount)
#define dfTimeCount     DFI(TimeCount)
#define dfProductID     DFI(ProductID)
#define dfMaxHWVersion  DFI(MaxHWVersion)
#define dfYear			DFI(Year)   //for second method of time setup
#define dfMonth			DFI(Month)
#define dfDay			DFI(Day)
#define dfHour			DFI(Hour)
#define dfMinute		DFI(Minute)
#define dfSecond		DFI(Second)

#define dfBuild         DFP2(Build)
#define dfRezVW         DFP2(RezVW)

#define gPlayfield		DataFlash.playfield


//=========================================================================
// Some globals

extern uint8_t	UpdateDFTimer;
extern uint8_t	UpdatePTTimer;
extern uint8_t	DFMagicNumber;

extern uint8_t	ScrFlip;

//=========================================================================
// Functions

extern void SetProfile();
extern void InitDataFlash();
extern void InitVariables();
extern void ResetDataFlash();
//extern void InitVariables();
//extern void DFCheckValuesValidity();
extern uint32_t CalcPageCRC( uint32_t *pu32Addr );
extern void UpdateDataFlash();
extern void UpdatePTCounters();
extern void DataFlashUpdateTick();
extern uint16_t GetShuntRezValue();

extern void LoadProfile( int p );
extern void SaveProfile();
extern void EraseProfile( int p );
extern int  IsProfileValid( int p );
extern void ApplyParameters();

extern void ResetPowerCurve();

extern void ResetDFlashRes();
extern void RestartMod();

//-------------------------------------------------------------------------

extern void FMCWritePage( uint32_t u32Addr, uint32_t *pu32Data );
extern int  FMCEraseWritePage( uint32_t u32Addr, uint32_t *pu32Data );
//extern uint32_t FMCVerifyPage( uint32_t u32Addr, uint32_t *pu32Data );

extern void FMCRead256( uint32_t u32Addr, uint32_t *pu32Buf );

//=========================================================================

#endif /* __DATAFLASH_H__ */
