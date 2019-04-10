#ifndef __ATOMIZER_H__
#define __ATOMIZER_H__

//=============================================================================

/* DC/DC converters PWM channels : BBC Buck-Boost Converter */
#define BBC_PWMCH_BUCK		0
#define BBC_PWMCH_BOOST		2

#define BBC_PWMCH_BUCK1		0
#define BBC_PWMCH_BUCK2		2


#define SHUNT_MIN_VALUE	75
#define SHUNT_MAX_VALUE	150

#define VVEL_DEF_RATIO	 300 //360
#define VVEL_MIN_RATIO	  50
#define VVEL_MAX_RATIO	1000

#define FIRE_PROTEC_MIN  10
#define FIRE_PROTEC_DEF 100
#define FIRE_PROTEC_MAX 250

#define PUFFS_OFF_MIN  0
#define PUFFS_OFF_DEF  0
#define PUFFS_OFF_MAX  99

enum
{
	TCALGO_JOY	= 0,
	TCALGO_SWEET,
	TCALGO_BOOST,
	TCALGO_PID,
	TCALGO_MAX
};

#define TCALGO_DEF TCALGO_JOY

#define PID_P_DEF	600
#define PID_P_MIN	1
#define PID_P_MAX	6000

#define PID_I_DEF	850
#define PID_I_MIN	0
#define PID_I_MAX	10000

#define PID_D_DEF	0
#define PID_D_MIN	0
#define PID_D_MAX	5000


//-------------------------------------------------------------------------

extern uint32_t	AtoVolts;
extern uint32_t	AtoVoltsMax;
extern uint32_t	TargetVolts;
extern uint32_t	AtoRezMilli;
extern uint32_t	ReplayRez;
extern uint32_t	AtoRezMilliMin; //min while puff
extern uint32_t	AtoMinVolts;
extern uint32_t	AtoMaxVolts;
extern uint32_t	AtoMinPower;
extern uint32_t	AtoMaxPower;
extern uint32_t	MaxTCPower;
extern uint32_t	MaxVolts;
extern uint32_t	MaxPower;
extern uint32_t	MaxCurrent;
extern uint16_t	TCR;
extern uint16_t	FireDuration;
extern uint8_t  CurveRepeatTimerDuration;
extern uint16_t	AtoTemp;
extern uint16_t VWVolts;
extern uint16_t	AtoCurrent;
extern uint16_t	AtoCurrentMax;
extern uint16_t	AtoRez;
extern uint8_t	AtoMillis;
extern uint8_t	AtoProbeCount;
extern uint8_t	AtoShuntRez;
extern uint8_t	AtoError;
extern uint8_t	AtoStatus;
extern uint8_t	BoardTemp;
extern uint8_t	AkkuTemp;
extern uint8_t	ConfigIndex;
extern uint8_t	PreheatTimer;
extern uint16_t	PreheatPower;
extern uint16_t	PreheatDelay;
extern uint16_t	CurveDelay;
extern uint16_t NextPreheatTimer;
extern uint16_t NextPreheatPower;
extern uint32_t	MilliJoules;
extern uint32_t	MilliJoulesDay;
extern uint32_t	MilliJoulesEnergy;
extern uint32_t	MilliJoulesVapedOn;
extern uint8_t  SessionPuffs;

extern uint8_t	RezMillis;
extern uint16_t	BypassVolts;

extern uint8_t	Set_NewRez_dfRez;
extern uint16_t	word_200000B8;
extern uint16_t	word_200000BA;
extern uint16_t	word_200000BC;
extern uint16_t	word_200000BE;

extern const uint8_t TempCoefsNI[];
extern const uint8_t TempCoefsTI[];

extern uint8_t  PuffsOffCount;

//-------------------------------------------------------------------------

extern void InitPWM();
extern void SetPWMClock();
extern void BBC_Configure( uint32_t chan, uint32_t mode );

extern void StopFire();

extern uint16_t LowestRezMeasure();
extern uint16_t ClampPower( uint16_t pwr, int clampmax );
extern uint16_t AtoPowerLimit( uint16_t pwr );
extern uint16_t AtoPower( uint16_t volts );
extern uint16_t GetVoltsForPower( uint16_t pwr );
extern uint16_t GetAtoVWVolts( uint16_t pwr, uint16_t r );
extern void ClampAtoPowers();
extern void ClampAtoVolts();
extern void SetMinMaxPower();
extern void SetMinMaxVolts();
extern void SetAtoLimits();

extern void RegulateBuckBoost();
extern void AtoWarmUp();
extern void TweakTargetVoltsVW();
extern void TweakTargetVoltsReplay();
extern void TweakTargetVoltsTC();
extern void ProbeAtomizer();
extern void ReadAtoCurrent();
extern void ReadAtoTemp();
extern void ReadAtomizer();
extern void GetTempCoef( const uint8_t tc[] );
extern void CheckMode();
extern void ReadBoardTemp();
extern void ReadAkkuTemp();
extern void Overtemp();
extern void ResetResistance();
extern void ProbeAtoSeries();

extern void SwitchRezLock( int lock );
extern uint16_t RereadRez();
//extern void SetRez();

//extern int SearchSMARTRez( uint16_t rez );
//extern void SetAtoSMARTParams();
extern void RoundPowers();

extern uint16_t CelsiusToF( uint16_t tc );
extern uint16_t FarenheitToC( uint16_t tf );
//extern uint16_t GetRegionalTemp( uint16_t atemp );

extern const uint16_t SMARTRezValues[];
extern const uint16_t SMARTPowers[];

extern void InitTCAlgo();
extern void TweakTargetVoltsSegments();
extern void TweakTargetVoltsAlgo();

extern int GetLockState();

//extern uint16_t	BuckDuty; //for onscreen debug only
//extern uint16_t	BoostDuty;
//extern uint16_t	PWMCycles; //for onscreen debug only

//=============================================================================

#endif /* __ATOMIZER_H__ */
