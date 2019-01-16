#ifndef __BATTERY_H__
#define __BATTERY_H__

//=========================================================================

#define BBC_PWMCH_CHARGER	5

#define BVO_MIN -100
#define BVO_MAX  100

#define BATTERY_CUSTOM	255

//-------------------------------------------------------------------------

typedef struct
{
	uint16_t percent;
	uint16_t voltage;
}
BatV2P_t;

typedef struct
{
	const uint8_t	*name;
	BatV2P_t	V2P[11];
	uint16_t	cutoff;
	uint16_t	intrez;
	uint16_t	maxamp;
}
Battery_t;

//enum { //BatteryStatus
//	BtSt_OK = 0,
//        BtSt_OOE_ERR,       //1 imbalanced
//	BtSt_NOBAT_ERR,     //2
//	BtSt_USBHI_ERR,     //3 high usb volts
//        BtSt_LOWCURR_ERR,   //4 charge err low output current?
//
//};
//
//enum { //ChargeStatus
//	ChSt_NOR = 0,    //0 Battery charging
//        ChSt_BAL,        //1 ballancing charge from min_ch
//        ChSt_LOW,        //2 low batt charging, < 2.8v
//	ChSt_MAX,        //3 max current charging
//	ChSt_MIN,        //4 min current charging
//        ChSt_FUL,        //5 charge full
//	ChSt_ERR,        //6 stopped charge
//};

//-------------------------------------------------------------------------

extern uint16_t RTBattVolts;
extern uint16_t	RTBVolts[4];
extern uint16_t	RTBVTotal;
extern uint16_t LowBatVolts;
extern uint32_t	PowerScale; //optional reduce pwr on low battery
extern uint16_t	BatteryVoltage;
extern uint16_t	BattVoltsTotal;
extern uint16_t	BattVolts[4];
extern uint16_t	BatteryCutOff;
extern uint16_t	BatteryIntRez;
extern uint16_t	BatteryMaxAmp;
extern uint16_t	BatteryMaxPwr;
extern uint8_t	BatteryPercent;
extern uint8_t	SavedBatPercent;
extern uint8_t	BatteryTenth;
extern uint8_t	BatteryTenthAll[4];
extern uint8_t	NoEventTimer;
extern uint16_t	LinePuffAwakeTimer;
extern uint16_t	VapedLineTimer;
extern uint8_t	BatReadTimer;
extern uint8_t	NumBatteries;
extern uint8_t	MaxBatteries;
extern uint16_t	ChargerDuty;
extern uint16_t	MaxChargerDuty;
extern uint16_t	ChBalTimer;

extern uint8_t	BattProbeCount;

extern uint8_t	USBMaxLoad;
extern uint8_t	ChargeStatus;
extern uint8_t	BatteryStatus; // 0- ok? / 1- ooe / 2- no batt / 3- bad high usb, 4- charge err low output current?
extern uint8_t	BBBits;
extern uint8_t	ChargeMode;
extern uint8_t	ChargeStep;

extern Battery_t CustomBattery;
extern const Battery_t *Battery;
extern uint32_t  USBVolts;
extern uint32_t  ChargeCurrent;

//-------------------------------------------------------------------------

extern void ReadBatteryVoltage();
extern void NewBatteryData();
extern void NewBatteryVoltage();
extern int CheckBattery();
extern int GetNBatteries();
extern void SetBatteryModel();
extern const uint8_t *GetBatteryName();
extern void ReadInternalResistance();
extern void SetBatMaxPower();
extern void SetMaxPower( const uint16_t p );
extern void SetMaxVolts( const uint16_t v );
extern void BatteryChargeDual();
//extern void BatteryChargeBuildIn();
extern void BatteryCharge();
extern void SaveCustomBattery( const Battery_t *b );
extern void LoadCustomBattery();
extern void ResetCustomBattery();
extern int  CheckCustomBattery();

//=========================================================================
#endif /* __BATTERY_H__ */
