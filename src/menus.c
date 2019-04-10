#include "myevic.h"
#include "events.h"
#include "screens.h"
#include "display.h"
#include "myrtc.h"
#include "dataflash.h"
#include "miscs.h"
#include "flappy.h"
#include "tetris.h"
#include "meusbd.h"
#include "atomizer.h"
#include "battery.h"
#include "timers.h"

//=============================================================================
// MENUS
//-----------------------------------------------------------------------------

typedef struct menu_s menu_t;
typedef struct mitem_s mitem_t;
typedef struct mdata_s mdata_t;
typedef struct mbitdesc_s mbitdesc_t;
typedef struct mvaluedesc_s mvaluedesc_t;

enum {
        MACTION_0,
	MACTION_SUBMENU,
	MACTION_DATA
};

enum {
	MITYPE_BIT,
	MITYPE_BYTE,
	MITYPE_SBYTE,
	MITYPE_WORD
};

struct mbitdesc_s
{
	const uint8_t div;
	const uint8_t pos;
	const uint8_t* const on;
	const uint8_t* const off;
};

struct mvaluedesc_s
{
	const uint8_t div;
	const uint8_t posr;
	const uint8_t nd;
	const uint8_t dp;
	const int16_t min;
	const int16_t max;
	void (*draw)( int x, int y, int v, uint8_t dp, uint8_t z, uint8_t nd );
	const uint8_t* const unit_s;
	const uint8_t unit_c;
	const uint8_t z;
	const uint8_t inc;
	const int16_t def1;
	const int16_t def2;
};

struct mdata_s
{
	void* const ptr;
	const void* const desc;
	const uint8_t type;
	const uint8_t bitpos;
};

struct mitem_s
{
	const uint8_t* const caption;
	const void* const action;
	const uint8_t event;
	const uint8_t action_type;
};

struct menu_s
{
	const uint8_t* const caption;
	const struct menu_s *parent;
	void (*on_enter)();
	void (*on_drawitem)( int mi, int y, int sel );
	void (*on_selectitem)();
	void (*on_clickitem)();
	int  (*on_event)( int event );
	const short nitems;
	const mitem_t mitems[];
};

const menu_t *CurrentMenu;
uint8_t CurrentMenuItem;
uint8_t PrevMenuItem;
uint8_t CUSSaved = 0;
uint8_t AkkuTempFlag = 0;
const menu_t LogoMenu;

const uint8_t *strSplash[] = { String_Off, String_On, String_Box };
const uint8_t *modes[] = { String_NI, String_TI, String_SS, String_TC, String_PW, String_BY, String_SM };
const uint8_t *strDMY[] = { String_DMY1, String_MDY, String_DMY2, String_YMD };
const uint8_t strADM[4] = { 0x9C, 0x9F, 0xA8, 0x9D }; //A D M B
const uint8_t *strSmartPH[] = { String_Off, String_TIME_s, String_PWR_s };

const mbitdesc_t BitDesc =
{
	0, 63,
	String_On,
	String_Off
};

const mbitdesc_t InvBitDesc =
{
	0, 63,
	String_Off,
	String_On
};

//-----------------------------------------------------------------------------
// Forward declarations for parent menu pointers

const menu_t MainMenu;
const menu_t CoilsMenu;
const menu_t ClockMenu;
const menu_t ScreenMenu;
const menu_t MiscsMenu;
const menu_t IFMenu;
const menu_t CurveMenu;
const menu_t VapingMenu;
const menu_t ExpertMenu;
const menu_t ProfileMenu;

//-----------------------------------------------------------------------------

// first Event then Click

__myevic__ void ProfileMenuIDraw( int it, int line, int sel )
{
    
    	uint8_t mode;
	//uint16_t rez;
        uint16_t val;
        int img;
        int z;
        static int trig = 0;
        
	if ( it >= DATAFLASH_PROFILES_MAX )
		return;

	DrawImage( 4, line+2, 0x0C + it );
	if ( sel ) InvertRect( 0, line, 12, line+12 );

        
        ++trig;
	//if ( p->PCRC == 0xFFFF )
        if ( ( it != dfProfile ) && !IsProfileValid( it ) )
		return;
    
        dfParams_t *p = (dfParams_t*)( DATAFLASH_PROFILES_BASE + it * DATAFLASH_PARAMS_SIZE );
    
	//const uint8_t *modes[] =
	//	{ String_NI, String_TI, String_SS, String_TC, String_PW, String_BY, String_SM };


	//if ( it == dfProfile )
	//{
	//	mode = dfMode;
	//	rez  = dfResistance;
	//}
	//else
	//{

		mode = p->Mode;
		//rez  = p->Resistance;
	//}
    
    val  = p->Resistance;
    img = 0xC0;
    z = 2;
    if ( trig >= 200 ) trig = 0; 
                    
    if ( trig < 100 ) 
    {
        if ( mode < 5 )
        {
                    if ( mode < 4 )
                    {
                        val  = p->TCPower;
                    }
                    else if ( mode == 4 )
                    {
                        val  = p->Power;
                    }
            z = 0;
            val /= 10;
            img = 0x7E;
        }
    }
    
        DrawStringCentered( String_LongFire, 105 );
	DrawStringCentered( String_Save, 116 );
        
	if ( mode > 6 )
		return;

	DrawString( modes[mode], 16, line+2 );
	DrawValueRight( 55, line+2, val, z, 0x0B, 0 );
	DrawImage( 56, line+2, img );
        
        if ( it == dfProfile ) InvertRect( 14, line, 63, line+12 );
        
    gFlags.refresh_display = 1;
        
}


__myevic__ int ProfileMenuOnEvent( int event )
{
	if ( CurrentMenuItem >= DATAFLASH_PROFILES_MAX )
		return 0;

	switch ( event )
	{
		case 1: // Fire button press
			break;

		case 15: // Single Fire
			//if ( CurrentMenuItem != dfProfile ) 

                            if ( IsProfileValid( CurrentMenuItem ) )
                            {
				LoadProfile( CurrentMenuItem );
                                dfProfile = CurrentMenuItem;
                                Event = EVENT_EXIT_MENUS;
                            }
                            else
                            {
                                EraseProfile( CurrentMenuItem );
                                gFlags.refresh_display = 1;
                            }
			break;

		case EVENT_LONG_FIRE:
			//if ( CurrentMenuItem != dfProfile )
                    
				if ( !AtoRez && IsProfileValid( CurrentMenuItem ) )
				{
                                    EraseProfile( CurrentMenuItem );
                                    gFlags.refresh_display = 1;
                                    //UpdateDFTimer = 50;
				}
                                else {
                                    dfProfile = CurrentMenuItem;
                                    SaveProfile();
                                    //LoadProfile( CurrentMenuItem );   
                                    //UpdateDFTimer = 50;
                                    
                                    Event = EVENT_EXIT_MENUS;
                                }  
                        
			//gFlags.refresh_display = 1;
			UpdateDFTimer = 50;                        
			//Event = EVENT_EXIT_MENUS;
			break;

		default:
			return 0;
	}

	return 1;
}


//-----------------------------------------------------------------------------

__myevic__ void AlgoMenuIDraw( int it, int line, int sel )
{
	switch ( it )
	{
		case 0:	// Algo
			DrawFillRect( 30, line, 63, line+12, 0 );
			switch ( dfTCAlgo )
			{
				case TCALGO_JOY:
				default:
					DrawStringRight( String_Off, 63, line+2 );
					break;

				case TCALGO_SWEET:
					DrawStringRight( String_Sweet, 63, line+2 );
					break;

				case TCALGO_BOOST:
					DrawStringRight( String_Boost, 63, line+2 );
					break;

				case TCALGO_PID:
					DrawStringRight( String_PID, 63, line+2 );
					break;
			}
			break;
	}
}

__myevic__ void AlgoMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 0: // Algo
			if ( ++dfTCAlgo >= TCALGO_MAX ) dfTCAlgo = 0;
			break;
	}

	gFlags.refresh_display = 1;
}

//-----------------------------------------------------------------------------

__myevic__ void DrawVapingLine (int line, uint16_t val)
{
    //same for two
    DrawFillRect( 40, line-2, 63, line+10, 0 );
                        if ( val ) 
                        {
                            DrawValueRight( 48, line, val / 3600, 0, 0x0B, 1 );
                            DrawImage( 48, line, 0xD7 );
                            DrawValue( 51, line, val / 60 % 60, 0, 0x0B, 2 );
                        }
                        else 
                        {
                            DrawStringRight( String_Off, 63, line );
                        }
}

__myevic__ void VapingMenuIDraw( int it, int line, int sel )
{
	switch ( it )
	{
		case 5:	// Protec
			DrawFillRect( 34, line, 63, line+12, 0 );
			DrawImageRight( 63, line+2, 0x94 );
			DrawValueRight( 57, line+2, dfProtec, 1, 0x0B, 0 );
			//if ( sel && gFlags.edit_value )
			//	InvertRect( 0, line, 63, line+12 );
			break;
                        
		case 6: // puffs off mod
                        DrawFillRect( 40, line, 63, line+12, 0 );
                        if ( dfPuffsOff ) 
                        {
                            DrawValueRight( 63, line+2, dfPuffsOff, 0, 0x0B, 0 );
                        }
                        else 
                        {
                            DrawStringRight( String_Off, 63, line+2 );
                        }
			//if ( sel && gFlags.edit_value )
			//	InvertRect( 0, line, 63, line+12 );
			break;
                        
                case 7: // alive time off
                        //DrawFillRect( 40, line, 63, line+12, 0 );
                        
                        DrawVapingLine( line+2, dfAwakeTimer );
/*
                        if ( dfAwakeTimer ) 
                        {
                            DrawValueRight( 48, line+2, dfAwakeTimer / 3600, 0, 0x0B, 1 );
                            DrawImage( 48, line+2, 0xD7 );
                            DrawValue( 51, line+2, dfAwakeTimer / 60 % 60, 0, 0x0B, 2 );
                        }
                        else 
                        {
                            DrawStringRight( String_Off, 63, line+2 );
                        }
*/
                        
			//if ( sel && gFlags.edit_value )
			//	InvertRect( 0, line, 63, line+12 );
			break;
                        
                case 8: // vape delay timer 0h:00m
                    
                        DrawVapingLine( line+2, dfVapeDelayTimer );
/*                        
                        DrawFillRect( 40, line, 63, line+12, 0 );

                        if ( dfVapeDelayTimer ) 
                        {
                            DrawValueRight( 48, line+2, dfVapeDelayTimer / 3600, 0, 0x0B, 1 );
                            DrawImage( 48, line+2, 0xD7 );
                            DrawValue( 51, line+2, dfVapeDelayTimer / 60 % 60, 0, 0x0B, 2 );
                        }
                        else 
                        {
                            DrawStringRight( String_Off, 63, line+2 );
                        }
*/
                        
			//if ( sel && gFlags.edit_value )
			//	InvertRect( 0, line, 63, line+12 );
			break;
                        
                case 11: //AutoPuffTimer
                        DrawFillRect( 36, line, 63, line+12, 0 );
                        if ( dfStatus.endlessfire )
                        {
                            DrawImageRight( 63, line+2, 0xEC );
                        }
                        else
                        {
                            DrawImageRight( 63, line+2, 0x94 );
                            DrawValueRight( 57, line+2, dfAutoPuffTimer, 1, 0x0B, 0 );
                        }
                        
                	//if ( sel && gFlags.edit_value )
			//	InvertRect( 0, line, 63, line+12 );    
                        break;             
	}
        
        if ( sel && gFlags.edit_value )
		InvertRect( 0, line, 63, line+12 );
}


__myevic__ void VapingMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 5:	// Protec
		case 6: // puffs off
                case 7: // alive time off
                case 11: // autopufftimers
			gFlags.edit_value ^= 1;
			break;
                case 8: // vape delay
                        if ( !VapeDelayTimer )
                        {
                            gFlags.edit_value ^= 1;
                        }
			break;
 
        }

	gFlags.refresh_display = 1;
}


__myevic__ int VapingMenuOnEvent( int event )
{
	int vret = 0;
        int holdtimer = dfVapeDelayTimer / 60; // in minutes
        int vapetimer = dfAwakeTimer / 60;
        
	if ( !gFlags.edit_value && event != EVENT_LONG_FIRE )
		return vret;

	switch ( event )
	{              
		case 2:	// Plus
			switch ( CurrentMenuItem )
			{
				case 5: // Protec
					if ( ++dfProtec > FIRE_PROTEC_MAX )
					{
						if ( KeyTicks < 5 ) dfProtec = FIRE_PROTEC_MIN;
						else dfProtec = FIRE_PROTEC_MAX;
					}
					vret = 1;
					break;
                                        
				case 6: // Puffs off
					if ( ++dfPuffsOff > PUFFS_OFF_MAX )
					{
						if ( KeyTicks < 5 ) dfPuffsOff = PUFFS_OFF_MIN;
						else dfPuffsOff = PUFFS_OFF_MAX;
					}
                                        PuffsOffCount = 0;
					vret = 1;
					break;
                                        
				case 7: // alive time off
					if ( ++vapetimer > 60 )
					{
						if ( KeyTicks < 5 ) vapetimer = 0;
						else vapetimer = 60;
					}
                                        dfAwakeTimer = vapetimer * 60; 
                                        AwakeTimer = 0;
					vret = 1;
					break;   
                                        
				case 8: // vape delay
					if ( ++holdtimer > 60 )
					{
						if ( KeyTicks < 5 ) holdtimer = 0;
						else holdtimer = 60;
					}
                                        dfVapeDelayTimer = holdtimer * 60;   
					vret = 1;
					break;                                        
                                        
				case 11: // autopufftimers
					if ( ++dfAutoPuffTimer > 251 ) // 25 sec max
					{
						if ( KeyTicks < 5 ) dfAutoPuffTimer = 10;
						else dfAutoPuffTimer = 251;
					}
					vret = 1;
					break;                                          
			}
			break;

		case 3:	// Minus
			switch ( CurrentMenuItem )
			{
				case 5: // Protec
					if ( --dfProtec < FIRE_PROTEC_MIN )
					{
						if ( KeyTicks < 5 ) dfProtec = FIRE_PROTEC_MAX;
						else dfProtec = FIRE_PROTEC_MIN;
					}
					vret = 1;
					break;

				case 6: // Puffs off
					if ( --dfPuffsOff > PUFFS_OFF_MAX )
					{
						if ( KeyTicks < 5 ) dfPuffsOff = PUFFS_OFF_MAX;
						else dfPuffsOff = PUFFS_OFF_MIN;
					}
                                        PuffsOffCount = 0;
					vret = 1;
					break;
                                        
				case 7: // alive time off
					if ( --vapetimer < 0 )
					{
						if ( KeyTicks < 5 ) vapetimer = 60;
						else vapetimer = 0;
					}
                                        dfAwakeTimer = vapetimer * 60;
                                        AwakeTimer = 0;
					vret = 1;
					break;
                                        
				case 8: // vape delay
					if ( --holdtimer < 0 )
					{
						if ( KeyTicks < 5 ) holdtimer = 60;
						else holdtimer = 0;
					}
                                        dfVapeDelayTimer = holdtimer * 60;
					vret = 1;
					break;
                                        
				case 11: // autopufftimers
					if ( --dfAutoPuffTimer < 9 )
					{                                                
						if ( KeyTicks < 5 ) dfAutoPuffTimer = 250;
						else dfAutoPuffTimer = 9;
					}
					vret = 1;
					break; 
			}
			break;
                        
                case EVENT_LONG_FIRE:
                        switch ( CurrentMenuItem )
			{                    
                                case 5: //protec
                                        dfProtec = 100;
                                        vret = 1;
                                        break;

                                case 6: //puffs off        
                                        dfPuffsOff = 10;
                                        PuffsOffCount = 0;
                                        vret = 1;
                                        break;
                                        
                                case 7: // alive time off
                                        dfAwakeTimer = 0;
                                        AwakeTimer = 0;
                                        vret = 1;
                                        break;
                                        
                                case 8: //vape delay   
                                        dfVapeDelayTimer = 0;
                                        VapeDelayTimer = 0;
                                        vret = 1;
                                        break;
                                        
                                case 11: //autopufftimers     
                                        dfAutoPuffTimer = 20;  
                                        //gFlags.edit_value = 0;
                                        vret = 1;
                                        break;        
                        }
                        
                        gFlags.edit_value = 0;
                        break;                         
	}

	if ( vret )
	{
            dfStatus.endlessfire = 0;
            if ( dfAutoPuffTimer == 9 || dfAutoPuffTimer == 251 )
                dfStatus.endlessfire = 1;
            
            UpdateDFTimer = 50;
            gFlags.refresh_display = 1;
	}

	return vret;
}

__myevic__ void ScreenMenuIDraw( int it, int line, int sel )
{
	switch ( it )
	{
		case 5: //fire scr duration
			DrawFillRect( 44, line, 63, line+12, 0 );
			DrawImage( 58, line+2, 0x94 );
			DrawValueRight( 57, line+2, dfFireScrDuration, 0, 0x0B, 0 );
			if ( sel && gFlags.edit_value )
				InvertRect( 0, line, 63, line+12 );
			break;

	}
}

__myevic__ void ScreenMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 5:	//fire scr duration
			gFlags.edit_value ^= 1;
			break;     
	}

	gFlags.refresh_display = 1;
}

__myevic__ int ScreenMenuOnEvent( int event )
{
	int vret = 0;

	if ( !gFlags.edit_value )
		return vret;

	switch ( event )
	{
		case 2:	// Plus
			switch ( CurrentMenuItem )
                        {
				case 5: //fire scr duration
					if ( ++dfFireScrDuration > 9 )
					{
						if ( KeyTicks < 5 ) dfFireScrDuration = 0;
						else dfFireScrDuration = 9;
					}
					vret = 1;
					break;                                        
			}
			break;

		case 3:	// Minus
			switch ( CurrentMenuItem )
			{
				case 5: //fire scr duration
					if ( --dfFireScrDuration > 9 ) //< 1 )
					{
						if ( KeyTicks < 5 ) dfFireScrDuration = 9;
						else dfFireScrDuration = 0;
					}
					vret = 1;
					break;

                                 
			}
			break;
                        
                case EVENT_LONG_FIRE:
                        switch ( CurrentMenuItem )
			{                    
                                case 5: //fire scr duration
                                        dfFireScrDuration = 2;
                                        gFlags.edit_value = 0;
                                        vret = 1;
                                        break;      
                        }  
                        break;  
	}

	if ( vret )
	{
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}

	return vret;
}

__myevic__ void LogoMenuIDraw( int it, int line, int sel )
{
    	//if ( it != 2 )
	//	return;
        
	switch ( it )
	{
		case 2: //logo
                {
                    DrawFillRect( 40, line, 63, line+12, 0 );
                    
                    if ( dfStatus.nologo )
 			DrawStringRight( String_Off, 63, line+2 );
                    else if ( dfStatus.logomid )
                        DrawStringRight( String_Mid, 63, line+2 );
                    else
                        DrawStringRight( String_Top, 63, line+2 );
                    
                }   
                    break;
	}
}

__myevic__ void LogoMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 2:	//logo
                {
			int f = dfStatus.logomid | ( dfStatus.nologo << 1 ); // dfStatus.nologo dfStatus.logomid 
			if ( ++f > 2 ) f = 0; //00 01 10
			dfStatus.logomid = f & 1;
			dfStatus.nologo = f >> 1; //10			
                }
                    break;     
	}
        
        UpdateDFTimer = 50;
	gFlags.refresh_display = 1;
}

//-----------------------------------------------------------------------------

__myevic__ void ClicksMenuIDraw( int it, int line, int sel )
{
	if ( it > CurrentMenu->nitems - 2 )
		return;

	DrawFillRect( 20, line, 63, line+12, 0 );
        
        uint8_t num = (it == 4)? dfThreeButtonsAct : dfClick[it];
        
	switch ( num )
	{
		default:
		case CLICK_ACTION_NONE:
			DrawString( String_None, 26, line+2 );
			break;

		case CLICK_ACTION_EDIT:
			DrawString( String_Edit, 26, line+2 );
			break;

		case CLICK_ACTION_CLOCK:
			DrawString( String_Clock, 26, line+2 );
			break;

		case CLICK_ACTION_TDOM:
			DrawString( String_PPwr, 26, line+2 );
			break;

		case CLICK_ACTION_NEXT_MODE:
			DrawString( String_Modes, 26, line+2 );
			break;

		case CLICK_ACTION_ON_OFF:
			DrawString( String_OnOff, 26, line+2 );
			break;

		case CLICK_ACTION_PROFILE:
			DrawString( String_Profile, 26, line+2 );
			break;

		case CLICK_ACTION_GAME:
			DrawString( String_Game, 26, line+2 );
			break;

		case CLICK_ACTION_TETRIS:
			DrawString( String_Tetris, 26, line+2 );
			break;
                        
		case CLICK_ACTION_SAVER:
			DrawString( String_Saver, 26, line+2 );
			break;
                        
                case CLICK_ACTION_MENU:
			DrawString( String_Menus, 26, line+2 );
			break;
                        
                case CLICK_ACTION_BATTERIES:
			DrawString( String_Battery, 26, line+2 );
			break;
                        
                case CLICK_ACTION_REZRESET:
			DrawString( String_ResetCoil, 26, line+2 );
			break;                        
                        
	}
        
                
        if ( sel && gFlags.edit_value )
            InvertRect( 0, line, 63, line+12 );
        
}



__myevic__ void ClicksMenuOnClick()
{
    
	gFlags.edit_value ^= 1;
     
	gFlags.refresh_display = 1;
        
/*
	if ( CurrentMenuItem > CurrentMenu->nitems - 2 )
		return;

        if ( CurrentMenuItem == 4 )
        {
            if ( ++dfThreeButtonsAct >= CLICK_ACTION_MAX )
            {
		dfThreeButtonsAct = CLICK_ACTION_NONE;
            }
        }
        else if ( ++dfClick[CurrentMenuItem] >= CLICK_ACTION_MAX )
        {
		dfClick[CurrentMenuItem] = CLICK_ACTION_NONE;
        }
        
	UpdateDFTimer = 50;
	gFlags.refresh_display = 1;
*/
}


__myevic__ int ClicksMenuOnEvent( int event )
{
	int vret = 0;

	if ( !gFlags.edit_value )
		return vret;

	switch ( event )
	{
		case 2:	// Plus
			switch ( CurrentMenuItem )
                        {
				case 0: //
                                case 1:
                                case 2:
                                case 3:
                                        if ( ++dfClick[CurrentMenuItem] >= CLICK_ACTION_MAX )
                                        {
                                            dfClick[CurrentMenuItem] = CLICK_ACTION_NONE;
                                        }
					vret = 1;
					break;
                                case 4:
                                        if ( ++dfThreeButtonsAct >= CLICK_ACTION_MAX )
                                        {
                                            dfThreeButtonsAct = CLICK_ACTION_NONE;
                                        }   
                                        vret = 1;
					break;
			}
			break;

		case 3:	// Minus
                        switch ( CurrentMenuItem )
                        {
				case 0: //
                                case 1:
                                case 2:
                                case 3:
                                        if ( --dfClick[CurrentMenuItem] > CLICK_ACTION_MAX )
                                        {
                                            dfClick[CurrentMenuItem] = CLICK_ACTION_MAX - 1;
                                        }
					vret = 1;
					break;
                                case 4:
                                        if ( --dfThreeButtonsAct > CLICK_ACTION_MAX )
                                        {
                                            dfThreeButtonsAct = CLICK_ACTION_MAX - 1;
                                        }
                                        vret = 1;
					break;
			}
			break;
                        
/*
                case EVENT_LONG_FIRE:
                        switch ( CurrentMenuItem )
			{                    
                                case : //

                                        gFlags.edit_value = 0;
                                        vret = 1;
                                        break;      
                        }  
                        break;  
*/
	}

	if ( vret )
	{
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}

	return vret;
}

//-----------------------------------------------------------------------------

__myevic__ void ClockMenuIDraw( int it, int line, int sel )
{
	switch ( it )
	{
		case 3:	// Format
		{
			//const uint8_t *strDMY[] =
			//	{ String_DMY1, String_MDY, String_DMY2, String_YMD };
                        
			int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			//const uint8_t *s = strDMY[f];
			DrawFillRect( 28, line, 63, line+12, 0 );
			DrawStringRight( strDMY[f], 63, line+2 );
			break;
		}

		case 5:	// Dial
			DrawFillRect( 36, line, 63, line+12, 0 );
			//DrawImageRight( 63, line+2, dfStatus.digclk ? 0x9F : 0x9C ); // D/A
                        
                        //const uint8_t strADM[4] = { 0x9C, 0x9F, 0xA8, 0x9D }; //A D M B
                        
			int f = dfStatus.digclk | ( dfStatus2.digclk2 << 1 );  // 0 1 2 3
                        DrawImageRight( 63, line+2, strADM[f] ); 
			break;
	}
}


__myevic__ void ClockMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		//case 0:
		//	Event = EVENT_SET_TIME;
		//	break;

		//case 1:
		//	Event = EVENT_SET_DATE;
		//	break;

		case 3:	// Format
		{
			int f = dfStatus.dfmt1 | ( dfStatus.dfmt2 << 1 );
			if ( ++f > 3 ) f = 0;
			dfStatus.dfmt1 = f & 1;
			dfStatus.dfmt2 = f >> 1;
			break;
		}

		case 5:	// Dial
			//dfStatus.digclk ^= 1;
                {    
                    	int f = dfStatus.digclk | ( dfStatus2.digclk2 << 1 );
			if ( ++f > 3 ) f = 0;
			dfStatus.digclk = f & 1;
			dfStatus2.digclk2 = f >> 1;
			break;
                }
	}
    UpdateDFTimer = 50;
    gFlags.refresh_display = 1;        
}


//-----------------------------------------------------------------------------

__myevic__ void IFMenuIDraw( int it, int line, int sel )
{
    //Interface
    
	if ( it < 1 || it > CurrentMenu->nitems - 3 )
		return;
        
	DrawFillRect( 40, line, 63, line+12, 0 );

	switch ( it )
	{
		case 1:	// 1Watt
			DrawStringRight( dfStatus.onewatt ? String_On : String_Off, 63, line+2 );
			break;

		case 2:	// 1C5F
			DrawStringRight( dfStatus.onedegree ? String_On : String_Off, 63, line+2 );
			break;

		case 3:	// Wake -+
			DrawStringRight( dfStatus.wakeonpm ? String_On : String_Off, 63, line+2 );
			break;

		case 4:	// Temp
			DrawImageRight( 63, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
			break;

		case 5:	// TDom
			DrawStringRight( dfStatus.priopwr ? String_On : String_Off, 63, line+2 );
			break;
                        
		case 6:	// UI
			DrawValueRight( 63, line+2, dfUIVersion + 1, 0, 0x0B, 0 );
			break;
                        
                case 7: //splash 00 01 10
                        {
                            int f = dfStatus2.splash0 | ( dfStatus2.splash1 << 1 );  // 0 1 2 3
                            //const uint8_t *strSplash[] = { String_Off, String_On, String_Box };
                            //const uint8_t *s = strSplash[f];
                            DrawStringRight( strSplash[f], 63, line+2 );
                        }
			break;
                        
		default:
			break;
	}
}


__myevic__ void IFMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 1:	// 1Watt
			dfStatus.onewatt ^= 1;
			if ( dfStatus.onewatt )
			{
				WattsInc = 10;
				RoundPowers();
			}
			else
			{
				WattsInc = 1;
			}
			break;

		case 2:	// 1C5F
			dfStatus.onedegree ^= 1;
			if ( !dfStatus.onedegree )
			{
				dfTemp -= dfTemp % ( dfStatus.IsCelsius ? 5 : 10 );
			}
			break;

		case 3:	// Wake -+
			dfStatus.wakeonpm ^= 1;
			break;

		case 4:	// Temp
			dfStatus.IsCelsius ^= 1;
                        dfBoardTempCorr = 0;
                        dfAkkuTempCorr = 0;
                        
			if ( dfStatus.IsCelsius )
			{
				dfTemp = FarenheitToC( dfTemp );
                                if ( !dfStatus.onedegree )
                                {
                                int rem = dfTemp % 5;
				dfTemp -= rem;
				if ( rem >= 3 ) dfTemp += 5;
                                }
				//if ( dfTemp < 100 ) dfTemp = 100;
				//if ( dfTemp > 315 ) dfTemp = 315;
                                if ( dfTemp < 5 ) dfTemp = 5;
				if ( dfTemp > 995 ) dfTemp = 995;
                                
			}
			else
			{
				dfTemp = CelsiusToF( dfTemp );
				int rem = dfTemp % 5;
				dfTemp -= rem;
				if ( rem >= 3 ) dfTemp += 5;
				//if ( dfTemp < 200 ) dfTemp = 200;
				//if ( dfTemp > 600 ) dfTemp = 600;
      				if ( dfTemp < 35 ) dfTemp = 35;
				if ( dfTemp > 995 ) dfTemp = 995;

			}
			break;

		case 5:	// TDom
			dfStatus.priopwr ^= 1;
			break;
                        
		case 6:	// UI
			if ( ++dfUIVersion > 1 )
                            dfUIVersion = 0;
			break;
                        
                case 7: //splash
                        {
                            int f = dfStatus2.splash0 | ( dfStatus2.splash1 << 1 );
                            if ( ++f > 2 ) f = 0;
                            
                            dfStatus2.splash0 = f & 1;
                            dfStatus2.splash1 = f >> 1;
                        }       
                        break;
                        
		//default: // Exit
		//	UpdateDataFlash();
		//	return;
	}

	UpdateDFTimer = 50;
	gFlags.refresh_display = 1;
}


//-----------------------------------------------------------------------------

__myevic__ void SetMinMaxPreheatPower( int min, int max )
{
    if ( dfPreheatPwr > max ) dfPreheatPwr = max;
    if ( dfPreheatPwr < min ) dfPreheatPwr = min;
}

//-----------------------------------------------------------------------------

__myevic__ void PreheatIDraw( int it, int line, int sel )
{
	if ( it > 4 ) return;

	int v, dp, img;

	switch ( it )
	{       
                case 0:
                        DrawFillRect( 40, line, 63, line+12, 0 );
                        DrawStringRight( dfStatus.preheat ? String_On : String_Off, 63, line+2 );
                        return;
                
		case 1:	// Unit
			DrawFillRect( 30, line, 63, line+12, 0 );
			DrawImageRight( 63, line+2, dfStatus.phpct ? 0xC2 : 0xB2 );
			return;

		case 2:	// Power
			v = dfPreheatPwr;
			if ( dfStatus.phpct )
			{
				int p = GetPreheatPwrFromPerc(); //dfPower * dfPreheatPwr / 100;
				if ( p > AtoMaxPower ) p = AtoMaxPower;
				if ( p < 10 ) p = 10;
				if ( p < 1000 )
				{
					dp = 1;
				}
				else
				{
					p = p / 10;
					dp = 0;
				}
				DrawImage(  10, 118, 0xAB ); //calc power
                                DrawImage(  18, 118, 0xFB );
				DrawValueRight( 45, 118, p, dp, 0x0B, 0 );
				DrawImage( 47, 118, 0x98 );
                                
				dp = 0;
				img = 0xC2;
			}
			else
			{
                                if ( v < 1000 && WattsInc == 1 )
				//dp = ( v < 1000 ) ? 1 : 0;
				//v  = ( v < 1000 ) ? v : v / 10;
                                {
                                    dp = 1;
                                }
                                else
                                {
                                    dp = 0;
                                    v = v / 10;
                                }
				img = 0x98;
			}                       
			break;
                       
		case 3:	// Smart
                        DrawFillRect( 39, line, 63, line+12, 0 );
			v = GetSmartPreheat();
                      
                        DrawStringRight( strSmartPH[v], 63, line+2 );
                        return;
                        
		case 4:	// Time
			v = dfPreheatTime / 10;
			dp = 1;
			img = 0x94;                                             
			break;
                        
		default:
			return;
	}

	DrawFillRect( 30, line, 63, line+12, 0 );
       
        DrawValueRight( 54, line+2, v, dp, 0x0B, 0 );
        DrawImageRight( 63, line+2, img );
        
       
	if ( sel && gFlags.edit_value )
		InvertRect( 0, line, 63, line+12 );
}


__myevic__ void DrawPreheatDelay( int x, int y, int v, uint8_t dp, uint8_t z, uint8_t nd )
{
	if ( v == 0 )
	{
		DrawStringRight( String_Off, 63, y+2 );
	}
	else
	{
		DrawValue( x, y+2, v / 60, 0, 0x0B, 1 ); //x+1
		DrawImage( x+6, y+2, 0xD7 );
		DrawValue( x+9, y+2, v % 60, 0, 0x0B, 2 );
	}
}

__myevic__ int PreheatMEvent( int event )
{
	int vret = 0;
                                      
	if ( CurrentMenuItem > 4 )
		return vret;

	switch ( event )
	{                       
		case 1:	// Fire
                    if ( CurrentMenuItem == 0 )
                    {
                                dfStatus.preheat ^= 1;   
                    }
                    else if ( CurrentMenuItem == 1 )
                    {
				dfStatus.phpct ^= 1;
				if ( dfStatus.phpct )
				{
                                        //dfPreheatPwr = SetPreheatPwrPerc();
					dfPreheatPwr = 100 * dfPreheatPwr / dfPower; //from real 2 perc
                                        SetMinMaxPreheatPower(50, 300);
					//if ( dfPreheatPwr > 300 ) dfPreheatPwr = 300;
					//if ( dfPreheatPwr < 50 ) dfPreheatPwr = 50;
				}
				else
				{
					dfPreheatPwr = GetPreheatPwrFromPerc(); //dfPower * dfPreheatPwr / 100;
                                        SetMinMaxPreheatPower(10, MaxPower);
					//if ( dfPreheatPwr > MaxPower ) dfPreheatPwr = MaxPower;
					//if ( dfPreheatPwr < 10 ) dfPreheatPwr = 10;
				}
                    }
                    else if ( CurrentMenuItem == 3 )     //smart    
                    {
                            int f = GetSmartPreheat();
                            if ( ++f > 2 ) f = 0;
                            
                            dfStatus2.smart_ph = f & 1;
                            dfStatus2.smart_ph2 = f >> 1;
                    }
                    else
                    {
                            gFlags.edit_value ^= 1;
                    }
                    
                    vret = 1;
                    break;

		case 2:	// Plus
			if ( gFlags.edit_value )
			{
				if ( CurrentMenuItem == 2 ) //pwr
				{
					if ( dfStatus.onewatt )
					{
						dfPreheatPwr -= dfPreheatPwr % 10; //correct to int
					}
					if ( dfStatus.phpct )
					{
						PowerPlus( &dfPreheatPwr, 50, 300 );
					}
					else
					{
						PowerPlus( &dfPreheatPwr, AtoMinPower, AtoMaxPower );
					}
				}
				else if ( CurrentMenuItem == 4 ) //time
				{
					if ( dfPreheatTime < 250 ) dfPreheatTime += 10;
					else if ( KeyTicks < 5 ) dfPreheatTime = 10;
				}

				vret = 1;
			}
			break;

		case 3:	// Minus
			if ( gFlags.edit_value )
			{
				if ( CurrentMenuItem == 2 ) //pwr
				{
					if ( dfStatus.onewatt )
					{
						dfPreheatPwr -= dfPreheatPwr % 10;
					}
					if ( dfStatus.phpct )
					{
						PowerMinus( &dfPreheatPwr, 50, 300 );
					}
					else
					{                                      
						PowerMinus( &dfPreheatPwr, AtoMinPower, AtoMaxPower );
					}
				}
				else if ( CurrentMenuItem == 4 ) //time
				{
					if ( dfPreheatTime >= 20 ) dfPreheatTime -= 10;
					else if ( KeyTicks < 5 ) dfPreheatTime = 250;
				}

				vret = 1;
			}
			break;
                        
                case EVENT_LONG_FIRE:
                    
			if ( CurrentMenuItem == 4 ) //time
                        { 
                            dfPreheatTime = 10;
                        }   
                        else if ( CurrentMenuItem == 5 ) //delay
                        { //preheat delay off
                            dfPHDelay = 0;
                        }     
                        
                        gFlags.edit_value = 0;
                        vret = 1;
                        
                        break;
	}
        
        if ( vret )
        {
            NextPreheatPower = GetPreheatPwr();  
            UpdateDFTimer = 50;
            gFlags.refresh_display = 1;                                
        }
	return vret;
}


//-----------------------------------------------------------------------------

__myevic__ void BVOMenuIDraw( int it, int line, int sel )
{
	if ( !it )
	{
		for ( int i = 0 ; i < NumBatteries ; ++i )
		{
			DrawValue(  2 + 32 * ( i & 1 ), 104 + ( i >> 1 ) * 12, BattVolts[i], 2, 0x0B, 3 );
			DrawImage( 23 + 32 * ( i & 1 ), 104 + ( i >> 1 ) * 12, 0x7D );
		}
		ScreenRefreshTimer = 10;
	}

	if ( it >= CurrentMenu->nitems - 2 )
		return;

	DrawFillRect( 22, line, 63, line+12, 0 );

	uint16_t bvo = ( dfBVOffset[it] >= 0 ) ? dfBVOffset[it] : -dfBVOffset[it];
	DrawImage( 26, line+2, ( dfBVOffset[it] >= 0 ) ? 0xFC : 0xFD );
	DrawValue( 34, line+2, bvo, 2, 0x0B, 3 );
	DrawImage( 55, line+2, 0x7D );
        
	if ( gFlags.edit_value && sel )
		InvertRect( 0, line, 63, line+12 );
}

__myevic__ void BVOMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 4: //batteries screen with graph
                    SetScreen( 54, 30 ); //need both for prevent auto-close
                    FireClicksEvent = 34;
                    break;

                default:
                    break;               
	}
}

__myevic__ void DrawCUS( int x, int y, int v, uint8_t dp, uint8_t z, uint8_t nd )
{    
            
	DrawValue( x, y + 2, v, dp, z, nd ); 
        
        if ( CurrentMenuItem > 21 )
            return;
        
        if ( CurrentMenuItem % 2 ) DrawImage( 37, 3, 0xBE ); 
        else DrawImage( 37, 3, 0xC2 );    
        
        DrawValue( 49, 3, (CurrentMenuItem + 2) / 2, 0, 0x0B, 2 );
        
}

__myevic__ void CUSMenuIDraw( int it, int line, int sel )
{
        //DrawValue( 0, 0, CUSSaved, 0, 0x0B, 2 );
        //DrawValue( 15, 0, line, 0, 0x0B, 1 );
        //DrawValue( 30, 0, CurrentMenuItem, 0, 0x0B, 2 );
    
	if ( it >= CurrentMenu->nitems - 1 )
		return;

        if ( it == 0 ) 
        {
            DrawFillRect( 30, line, 63, line+12, 0 );
            DrawImage( 40, line+2, 0x0B );  
        } 
        else if ( it == 20 ) 
        {
            DrawFillRect( 30, line, 63, line+12, 0 );
            DrawValue( 40, line+2, 100, 0, 0x0B, 3 );  
        }        
        else if ( it == 22 ) 
        {
            switch ( CUSSaved )
            {
                    default:
                    case 0:
                        break;
                    case 1:
                        DrawStringInv( String_Ok, 40, line+2 );  
                        break;
                    case 2:
                        DrawStringInv( String_Error, 35, line+2 ); 
                        break;
            }  
            gFlags.refresh_display = 1;
        }
}

/*
__myevic__ void CUSMenuOnClick()
{
            	if ( CurrentMenuItem == 22 ) //save 2 DF
                {
		    if ( CheckCustomBattery() )
                    {
                        SaveCustomBattery ( &CustomBattery );   
                        CUSSaved = 1;
                    }
                    else
                    { 
                        CUSSaved = 2;
                    }
                }
}
*/

__myevic__ int CUSMenuOnEvent( int event )
{   
    
    	switch ( event )
	{       
            case 1:
                if ( CurrentMenuItem == 22 ) //save 2 DF
                {
		    if ( CheckCustomBattery() )
                    {
                        SaveCustomBattery ( &CustomBattery );   
                        CUSSaved = 1;
                    }
                    else
                    { 
                        CUSSaved = 2;  //error
                    }
                }
                break;               
            case 2:
            case 3:
                CUSSaved = 0;  
                break;
        }
        
        return 0; //no capture
}

__myevic__ void MaxMenuIDraw( int it, int line, int sel )
{  
        int t;
    
	if ( it >= CurrentMenu->nitems - 1 )
		return;

	DrawFillRect( 30, line, 63, line+12, 0 );

	switch ( it )
	{
		case 0:	// W  wv   
                    if (!dfMaxPower) dfMaxPower = MaxPower;
                    
                    if ( dfMaxPower < 1000 )
                    {
                    	DrawValueRight( 53, line+2, dfMaxPower, 1, 0x0B, 0 );
                    } else {
                        DrawValueRight( 53, line+2, dfMaxPower / 10, 0, 0x0B, 0);  
                    }
			DrawImageRight( 63, line+2, 0x7E ); //W-
                        break;                       
                    
		case 1:	// v
                        if (!dfMaxVolts) dfMaxVolts = MaxVolts;
                        
                    	DrawValueRight( 53, line+2, dfMaxVolts, 2, 0x0B, 0 );
			DrawImageRight( 63, line+2, 0x7D ); //V-
			break;

		case 2:	// t
                        t = dfStatus.IsCelsius ? dfMaxBoardTemp : CelsiusToF( dfMaxBoardTemp );
			DrawValueRight( 53, line+2, t, 0, 0x0B, t>99?3:2 );
			DrawImageRight( 63, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
			break;     
	                 
		case 3:	// ch
                    if ( !dfUSBMaxCharge || !gFlags.soft_charge )
                    {
                        DrawImageRight( 63, line+2 , 0xF6 ); // N/A
                    } else {
                    	DrawValueRight( 53, line+2, dfUSBMaxCharge / 10, 2, 0x0B, 3 );
			DrawImageRight( 63, line+2, 0x68 ); //A-
                    }
			break;
                
                case 4:	// BY batts num
                        DrawStringRight( dfStatus2.bybatts ? String_All : String_B1, 63, line+2 );
			break;
                        
		case 5:	// bat cut off
                    	DrawValueRight( 53, line+2, BatteryCutOff, 2, 0x0B, 0 );
			DrawImageRight( 63, line+2, 0x7D );
			break;                        
                        
		//case 4:	// bat amps
                //    
                //        DrawValueRight( 53, line+2, dfBattMaxAmps, 0, 0x0B, 0 );
		//	DrawImageRight( 63, line+2, 0x68 );
		//	break;
                        
		default:
			break;
	}
        
        if ( gFlags.edit_value && sel )
	{
		InvertRect( 0, line, 63, line+12 );
	}
}

__myevic__ void MaxMenuOnClick()
{
    	if ( CurrentMenuItem > CurrentMenu->nitems - 1 )
		return;
        
        if ( CurrentMenuItem == 3 && !gFlags.soft_charge )
                return;
        
        if ( CurrentMenuItem == 4 )
        {
            dfStatus2.bybatts ^= 1;
        }
        else
        {
        gFlags.edit_value ^= 1;         
        }
        
        gFlags.refresh_display = 1;
}

__myevic__ int MaxMenuOnEvent( int event )
{   
	int vret = 0;

	switch ( event )
	{              
		case 2:	// +
			if ( !gFlags.edit_value ) break;
                        
			switch ( CurrentMenuItem )
			{
				case 0:	// W
                                    if ( dfMaxPower + WattsInc > 5000 )
                                    {
					dfMaxPower = ( KeyTicks == 0 ) ? 10 : 5000;
                                    } 
                                    else
                                    {
                                        if ( dfMaxPower < 1000 )
                                        {
                                            dfMaxPower += WattsInc;
                                        } else
                                        {
                                            dfMaxPower += 10; 
                                        }
                                    
                                    }
                                    break;
                                        
                                case 1: //v
                                    
                                        if ( ++dfMaxVolts > 999 )
                                        {
                                            dfMaxVolts = ( KeyTicks == 0 ) ? 50 : 999;
                                        }
                                        break;
                                        
                                case 2: //board temp 
                                        if ( ++dfMaxBoardTemp > 99 )
                                        {
                                            dfMaxBoardTemp = ( KeyTicks == 0 ) ? 20 : 99;
                                        }
                                        break;
                                        
                                case 3: //ch
                                        if ( dfUSBMaxCharge + 10 > 2000 )
                                        {
                                            dfUSBMaxCharge = ( KeyTicks == 0 ) ? 100 : 2000;
                                        } else {
                                         dfUSBMaxCharge += 10;   
                                        }
					break;

                                case 5: //batt cutoff
                                    
                                        if ( ++dfBatCutOff > 80 )
                                        {
                                            dfBatCutOff = ( KeyTicks == 0 ) ? 20 : 80;
                                        }
                                        break;
                                        
                                //case 4: //max amps
                                //        if ( dfBattMaxAmps + 1 > 50 )
                                //        {
                                //            dfBattMaxAmps = ( KeyTicks == 0 ) ? 1 : 50;
                                //        } 
                                //        else 
                                //        {
                                //            ++dfBattMaxAmps;   
                                //        }
				//	break;                                         
			}
                        vret = 1;
			break;

		case 3:	// -
			if ( !gFlags.edit_value ) break;
                        
			switch ( CurrentMenuItem )
			{
				case 0:	// W
                                    if ( dfMaxPower - WattsInc < 10 )
                                    {
					dfMaxPower = ( KeyTicks == 0 ) ? 5000 : 10;
                                    }
                                    else
                                    {
                                        if ( dfMaxPower < 1000 )
                                        {
                                            dfMaxPower -= WattsInc;
                                        } else
                                        {
                                            dfMaxPower -= 10; 
                                        }
                                    }
					break;
                                        
                                case 1: //v
                                        if ( --dfMaxVolts < 50 )
                                        {
                                            dfMaxVolts = ( KeyTicks == 0 ) ? 999 : 50;
                                        }
					break;  
                                        
                                case 2: //board temp
                                        if ( --dfMaxBoardTemp < 20 )
                                        {
                                            dfMaxBoardTemp = ( KeyTicks == 0 ) ? 99 : 20;
                                        }
					break;
                                        
                                case 3: //ch
                                        if ( dfUSBMaxCharge - 10 < 100 )
                                        {
                                            dfUSBMaxCharge = ( KeyTicks == 0 ) ? 2000 : 100;
                                        } else {
                                         dfUSBMaxCharge -= 10;   
                                        }
					break;    

                                case 5: //batt cutoff
                                    
                                        if ( --dfBatCutOff < 20 )
                                        {
                                            dfBatCutOff = ( KeyTicks == 0 ) ? 80 : 20;
                                        }
                                        break;
                                        
                                //case 4: //max amps
                                //        if ( dfBattMaxAmps - 1 < 1 )
                                //        {
                                //            dfBattMaxAmps = ( KeyTicks == 0 ) ? 50 : 1;
                                //        } 
                                //        else 
                                //        {
                                //            --dfBattMaxAmps;   
                                //        }
				//	break;                                         
			}
                        vret = 1;
			break;

		case EVENT_LONG_FIRE:

			switch ( CurrentMenuItem )
			{
				case 0:	// w
					dfMaxPower = 800;
					break;

				case 1:	// v
					dfMaxVolts = 600;
					break;
                                        
				case 2:	// board temp
					dfMaxBoardTemp = 70;
					break;   
                                
                                case 3:	// charge
					dfUSBMaxCharge = 1000;
					break;  

                                case 5:	// bat cutoff
					dfBatCutOff = 30; //2.80
					break; 
                                        
                                //case 4:	// max amps
				//	dfBattMaxAmps = 20;
				//	break;                                          
			}
                        vret = 1;
                        gFlags.edit_value = 0;
			break;
	}

	if ( vret )
	{
            
            MaxPower = dfMaxPower;
            MaxTCPower = dfMaxPower;
            MaxVolts = dfMaxVolts;
            SetAtoLimits();
            
            SetBatteryModel();
            
            UpdateDFTimer = 50;
            gFlags.refresh_display = 1;
	}

	return vret;
}

//-----------------------------------------------------------------------------

__myevic__ void ExpertMenuIDraw( int it, int line, int sel )
{
    int t;
        
	//if ( it >= CurrentMenu->nitems - 1 )
	//	return;
	if ( it < 1 || it > 6 )
		return;
    
	DrawFillRect( 32, line, 63, line+12, 0 );

	switch ( it )
	{
                        
		case 1:	// X32
			if ( dfStatus.x32off )
				DrawStringRight( String_Off, 63, line+2 );
			else
				DrawStringRight( String_On, 63, line+2 );
			break;
                    
		case 2:	// LSL
			if ( dfStatus.lsloff )
				DrawStringRight( String_Off, 63, line+2 );
			else
				DrawStringRight( String_On, 63, line+2 );
			break;

		case 3:	// SHR
			//DrawValue( 40, line+2, AtoShuntRez, 0, 0x0B, 3 );
                        //ProbeAtomizer();
                        DrawValueRight( 63, line+2, AtoRezMilli, 3, 0x0B, 4 );
			//if ( gFlags.edit_value && sel )
			//	InvertRect( 0, line, 63, line+12 );
			break;                    

		case 4:	// UCH
                        if ( !gFlags.soft_charge )
                        {
                        DrawImageRight( 63, line+2 , 0xF6 ); // N/A
                        }
                        else
                        {
                            if ( dfStatus.usbchgoff )
				DrawStringRight( String_Off, 63, line+2 );
                            else
				DrawStringRight( String_On, 63, line+2 );
                        }
			break;                   

		case 5:	// BAT profile
			DrawStringRight( GetBatteryName(), 63, line+2 );
                        //if ( gFlags.edit_value && sel )
                        //        InvertRect( 0, line, 63, line+12 );
			break;

/*
		case 6:	// USB
			if ( dfStatus.vcom )
				DrawStringRight( String_COM, 63, line+2 );
			//else if ( dfStatus.storage )
			//	DrawString( String_DSK, 40, line+2 );
			else
				DrawStringRight( String_HID, 63, line+2 );
			break;
*/

/*
		case 7:	// DBG
			if ( dfStatus.dbgena )
				DrawStringRight( String_ON, 63, line+2 );
			else
				DrawStringRight( String_OFF, 63, line+2 );
			break;
*/

		//case 8:	// PC Tools
		//	if ( dfStatus.nfe )
		//		DrawString( String_ON, 40, line+2 );
		//	else
		//		DrawString( String_OFF, 40, line+2 );
		//	break;
                        
                case 6:	// mod Temp
                    if ( AkkuTempFlag )
                    {
                        t = dfStatus.IsCelsius ? AkkuTemp : CelsiusToF( AkkuTemp );
                    }
                    else
                    {
                        t = dfStatus.IsCelsius ? BoardTemp : CelsiusToF( BoardTemp );
                    }
                        
			DrawValueRight( 54, line+2, t, 0, 0x0B, 0 ); //t>99?3:2
			DrawImageRight( 63, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
			//if ( gFlags.edit_value && sel )
			//	InvertRect( 0, line, 63, line+12 );
			break; 
                        
		default:
			break;
	}
        
        if ( gFlags.edit_value && sel )
		InvertRect( 0, line, 63, line+12 );
        
        ProbeAtoSeries();
        gFlags.refresh_display = 1;
}

__myevic__ void ExpertMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		//case 0:	// BVO
		//	break;
                    
		case 1:	// X32
                        if ( gFlags.has_x32 )
                        {
                            dfStatus.x32off ^= 1;
                        }
			break;
                   
		case 2:	// LSL
			dfStatus.lsloff ^= 1;
			break;
                    
		case 3:	// SHR
			if ( !(gFlags.edit_value ^= 1) )
				dfShuntRez = AtoShuntRez;
			break;
                    
		case 4:	// UCH
                        if ( gFlags.soft_charge )
                            dfStatus.usbchgoff ^= 1;
			break;
                   
		case 5:	// BAT
                //      //BATTERY_CUSTOM 255
		//	if ( ++dfBatteryModel >= GetNBatteries() )
		//		dfBatteryModel = 0;
		//	SetBatteryModel();
                        gFlags.edit_value ^= 1;
			break;
/*                        
		case 6:	// USB

			if ( dfStatus.vcom )
			{
				dfStatus.vcom = 0;
				//dfStatus.storage = 1;
			}
			else if ( dfStatus.storage )
			{
				dfStatus.storage = 0;
			}
			else
			{
				dfStatus.vcom = 1;
			}

                        dfStatus.vcom ^= 1;
			InitUSB();
			break;
*/                    
		//case 7:	// DBG
		//	dfStatus.dbgena ^= 1;
		//	if ( ! dfStatus.dbgena ) gFlags.debug = 0;
		//	break;
                    
		//case 8:	// PCT
		//	dfStatus.nfe ^= 1;
		//	break;
                        
                case 6: //boart temp corr
                    
                    if ( ( ISSINFJ200 || ISIKU200 ) && !AkkuTempFlag && gFlags.edit_value )
                    {
                        AkkuTempFlag = 1;
                    }
                    else
                    { 
                        gFlags.edit_value ^= 1;
                        AkkuTempFlag = 0;
                    }   
                        break;
                        
                //case 10:	// Back
		//	UpdateDataFlash();
		//	break;
	}

	gFlags.refresh_display = 1;
}

__myevic__ int ExpertMenuOnEvent( int event )
{
	int vret = 0;

	switch ( event )
	{              
		case 2:	// +
			if ( !gFlags.edit_value )
				break;
			switch ( CurrentMenuItem )
			{                                       
				case 3:	// Shunt Rez
					if ( ++AtoShuntRez > SHUNT_MAX_VALUE )
					{
						AtoShuntRez = ( KeyTicks == 0 ) ? SHUNT_MIN_VALUE : SHUNT_MAX_VALUE;
					}
					vret = 1;
					break;
                                        
                                case 5:	// BAT
                                    if ( dfBatteryModel == BATTERY_CUSTOM )
                                    {
                                        dfBatteryModel = 0; 
                                    }
                                    else
                                    {
                                        dfBatteryModel = BATTERY_CUSTOM;
                                        //++dfBatteryModel;
                                    	//if ( dfBatteryModel >= GetNBatteries() )  
                                        //    dfBatteryModel = BATTERY_CUSTOM;  
                                    }
                                    SetBatteryModel();
                                    vret = 1;
                                    break;
                                         
                                case 6: //board temp corr
                                    if ( AkkuTempFlag )
                                    {
                                        if ( ++AkkuTemp > 70 )
                                        {
                                            AkkuTemp = 70;
                                        }
                                        else
                                        {
                                        ++dfAkkuTempCorr;    
                                        }                                        
                                    }
                                    else
                                    {
                                        if ( ++BoardTemp > 99 )
                                        {
                                            BoardTemp = 99;
                                        }
                                        else
                                        {
                                        ++dfBoardTempCorr;    
                                        }
                                    }
                                    vret = 1;
                                    break;
			}
			break;

		case 3:	// -
			if ( !gFlags.edit_value )
				break;
			switch ( CurrentMenuItem )
			{
				case 3:	// Shunt Rez
					if ( --AtoShuntRez < SHUNT_MIN_VALUE )
					{
						AtoShuntRez = ( KeyTicks == 0 ) ? SHUNT_MAX_VALUE : SHUNT_MIN_VALUE;
					}
					vret = 1;
					break;
                                        
                                case 5:	// BAT
                                    if ( dfBatteryModel == 0 )
                                    {
                                            dfBatteryModel = BATTERY_CUSTOM; 
                                    }
                                    else
                                    {
                                        dfBatteryModel = 0;
                                        //if ( dfBatteryModel == BATTERY_CUSTOM )
                                        //    dfBatteryModel = GetNBatteries() - 1;   
                                        //else
                                        //    --dfBatteryModel;
                                    }
                                    SetBatteryModel();
                                    vret = 1;
                                    break;
                                          
                                case 6: //board temp corr
                                    if ( AkkuTempFlag )
                                    {
                                        if ( --AkkuTemp > 70 )
                                        {
                                            AkkuTemp = 0;
                                        }
                                        else
                                        {
                                        --dfAkkuTempCorr;    
                                        }                                        
                                    }
                                    else
                                    {
                                        if ( --BoardTemp > 99 )
                                        {
                                            BoardTemp = 0;
                                        }
                                        else
                                        {
                                        --dfBoardTempCorr;    
                                        }
                                    }
                                    vret = 1;
                                    break;                                        
			}
			break;

		case EVENT_LONG_FIRE:
			switch ( CurrentMenuItem )
			{
				case 3:	// Shunt Rez
					AtoShuntRez = GetShuntRezValue();
					//dfShuntRez = 0;
					gFlags.edit_value = 0;
					vret = 1;
					break;

				//case 5:	// Battery model
				//	dfBatteryModel = BATTERY_CUSTOM;
				//	SetBatteryModel();
                                //        gFlags.edit_value = 0;
				//	vret = 1;
				//	break;
                                        
				case 6:	// board temp corr
                                    
                                        dfAkkuTempCorr = 0;
                                        gFlags.sample_atemp = 1;
                                        ReadAkkuTemp();

                                        dfBoardTempCorr = 0;
                                        gFlags.sample_btemp = 1;
                                        ReadBoardTemp();
                                        gFlags.edit_value = 0;
                                        AkkuTempFlag = 0;
                                        vret = 1;
                                        break;                                        
			}
			break;
	}

	if ( vret )
	{
                if (CurrentMenuItem == 3 )
                {
                    Set_NewRez_dfRez = 2;
                    AtoProbeCount = 0;
                    AtoRezMilli = 0;
                    dfShuntRez = AtoShuntRez;
                }
                
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}

	return vret;
}


//-----------------------------------------------------------------------------

__myevic__ void ScreenSaveOnEnter()
{
	CurrentMenuItem = dfScreenSaver;
}


__myevic__ void ScreenSaveOnSelect()
{
	dfScreenSaver = CurrentMenuItem;
	UpdateDFTimer = 50;
}

//-----------------------------------------------------------------------------

__myevic__ void CoilsMenuIDraw( int it, int line, int sel )
{
        int t;
	if ( it < 4 || it > 6 )
		return;
        
	DrawFillRect( 32, line, 63, line+12, 0 );

	switch ( it )
	{
		case 4:	// Cold
                        t = dfStatus.IsCelsius ? dfColdLockTemp : CelsiusToF( (uint16_t)dfColdLockTemp );
			DrawValueRight( 52, line+2, t, 0, 0x0B, 0 ); //t>99?3:2
                        DrawImageRight( 63, line+2, dfStatus.IsCelsius ? 0xC9 : 0xC8 );
			//DrawImage( 55, line+2, dfIsCelsius ? 0xC9 : 0xC8 );
			break;
		case 5:	// New
			DrawValueRight( 52, line+2, dfNewRezPerc, 0, 0x0B, 0 );
			DrawImageRight( 63, line+2, 0xC2 );
			break;
		case 6:	// Check
		{
			const uint8_t *s;
			DrawFillRect( 36, line, 63, line+12, 0 );
			s = ( dfStatus.chkmodeoff ) ? String_Off : String_On; //String_No : String_Yes;
			DrawStringRight( s, 63, line+2 );
			break;
		}                        
		default:
			break;
	}

	if ( gFlags.edit_value && sel )
	{
		InvertRect( 0, line, 63, line+12 );
	}
}


__myevic__ void CoilsMenuOnClick()
{
	switch ( CurrentMenuItem )
	{           
		case 4:	// Cold
                case 5: // New
			gFlags.edit_value ^= 1;
			break;
		case 6:	// Check
			//if ( ISMODETC(dfMode) )
			//{
				dfStatus.chkmodeoff ^= 1;
			//}
			break;
		default:
			break;
	}
        gFlags.refresh_display = 1;
}


__myevic__ int CoilsMenuOnEvent( int event )
{
	int vret = 0;
        
        if ( CurrentMenuItem == 2 && event == 1 ) //&& event == 15 )
        {
            CurrentMenu = &ProfileMenu;
            CurrentMenuItem = dfProfile;
            SetScreen( 102, 15 );
            FireClicksEvent = EVENT_PROFILE_MENU;
            //Event = EVENT_PROFILE_MENU;
            
            
            //CurrentMenu = &ProfileMenu;
            //CurrentMenuItem = dfProfile;
            //SetScreen( 102, 15 );  
            return 1;
        }

	if ( !gFlags.edit_value )
		return vret;

	switch ( event )
	{
		case 2:
			switch ( CurrentMenuItem )
			{
				case 4:	// Cold
					if ( ++dfColdLockTemp > 40 )
						dfColdLockTemp = 1;
					vret = 1;
					break;
				case 5:	// New
					if ( ++dfNewRezPerc > 50 )
						dfNewRezPerc = 1;
					vret = 1;
					break;                                        
			}
			break;

		case 3:
			switch ( CurrentMenuItem )
			{
				case 4:	// Cold
					if ( --dfColdLockTemp < 1 )
						dfColdLockTemp = 40;
					vret = 1;
					break;
				case 5:	// Cold
					if ( --dfNewRezPerc < 1 )
						dfNewRezPerc = 50;
					vret = 1;
					break;                                        
			}
			break;
                        
		case EVENT_LONG_FIRE:
			switch ( CurrentMenuItem )
			{
				case 4:	// Cold
					dfColdLockTemp = 20;
                                        gFlags.edit_value = 0;
					vret = 1;
					break;
				case 5:	// Cold
					dfNewRezPerc = 5;
                                        gFlags.edit_value = 0;
					vret = 1;
					break;                                        
			}
			break;                        
	}

	if ( vret )
	{
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}

	return vret;
}

//-----------------------------------------------------------------------------

__myevic__ void ScreenProtMenuIDraw( int it, int line, int sel )
{
	int v;

	if ( it > 4 )
		return;

	DrawFillRect( ( ( it == 2 ) || ( it == 3 ) )? 39 : 34, line, 63, line+12, 0 );

	switch ( it )
	{
		case 0:	// Saver
			v = ScrSaveTimes[dfScreenProt];
			if ( v ) //&& v < 255 )
			{
				DrawValueRight( 54, line+2, v, 0, 0x0B, 0 ); //54 - char m > s
                                if ( dfScreenProt < 3 ) 
                                { //sec
                                    DrawImageRight( 63, line+2, 0x94 );
                                }
				else 
                                { //min
                                    DrawImageRight( 63, line+2, 0x8E );
                                }
			}
                        //else if ( v == 255 )
                        //{
                        //        DrawString( String_On, 44, line+2 );
                        //}
			else
			{
				DrawStringRight( String_Off, 63, line+2 );
			}
			break;
                                     
		case 1:	// Main
			v = ScrMainTimes[dfScrMainTime];
			DrawValueRight( 54, line+2, v, 0, 0x0B, 0 );
			DrawImageRight( 63, line+2, 0x94 );
			break;
                        
		case 2:	// charge
			v = ScrChargeTimes[dfScrChargeTime];
			if ( v )
			{
				DrawValueRight( 54, line+2, v, 0, 0x0B, 0 );
                                DrawImageRight( 63, line+2, 0x94 ); //S
			}
			else
			{
				DrawStringRight( String_On, 63, line+2 ); //always show scr
			}
			break;
                        
		case 3:	// stealth
                    
			v = dfStealthPuffsCnt;
			if ( v )
			{
				DrawValueRight( 54, line+2, v, 0, 0x0B, 0 );
                                DrawImageRight( 63, line+2, 0x91 ); //p uff
			}
			else
			{
				DrawStringRight( String_Off, 63, line+2 );
			}
			break;
                        
		case 4:	// ModOff mode
                    
                        if ( dfDimOffMode == 2 )
                        {
                            DrawStringRight( String_Lock, 63, line+2 );
                        }
                        else
                        {
                            DrawStringRight( dfDimOffMode ? String_OFF : String_Sleep, 63, line+2 );
                        }
                    
			break;
                        
		default:
			break;
	}

	if ( gFlags.edit_value && CurrentMenuItem == it )
	{
		InvertRect( 0, line, 63, line+12 );
	}
}


__myevic__ void ScreenProtMenuOnClick()
{
	switch ( CurrentMenuItem )
	{
		case 0:	// Saver
		case 1:	// Main
		case 2:	// Charge  
                case 3: //stealth
                    
			gFlags.edit_value ^= 1;
			break;                                             
                        
		case 4:
                    
                        if ( ++dfDimOffMode > 2 )
                        {
                            dfDimOffMode = 0;
                        }                   
                        break;
                    
		default:
			break;
	}
        
        gFlags.refresh_display = 1;
}


__myevic__ int ScreenProtMenuOnEvent( int event )
{
	int vret = 0;

	if ( !gFlags.edit_value )
		return vret;

	switch ( event )
	{
		case 2: // +
			switch ( CurrentMenuItem )
			{
				case 0:	// Saver
					if ( ++dfScreenProt > 7 )
						dfScreenProt = 0;
                                        
					vret = 1;
					break;

				case 1:	// Main
					if ( ++dfScrMainTime > 5 )
						dfScrMainTime = 0;
                                        
					//dfDimTimeout = ScrMainTimes[dfScrMainTime];
					vret = 1;
					break;
                                        
				case 2:	// Charge
					if ( ++dfScrChargeTime > 3 )
						dfScrChargeTime = 0;
                                        
					vret = 1;
					break;
                                        
				case 3:	// stealth
					if ( ++dfStealthPuffsCnt > 20 )
						dfStealthPuffsCnt = 0;
                                        
					vret = 1;
					break;                                         
			}
			break;

		case 3: // -
			switch ( CurrentMenuItem )
			{
				case 0:	// Saver
					if ( !dfScreenProt-- )
						dfScreenProt = 7;
					vret = 1;
					break;

				case 1:	// Main
					if ( !dfScrMainTime-- )
						dfScrMainTime = 5;
                                        
					//dfDimTimeout = ScrMainTimes[dfScrMainTime];
					vret = 1;
					break;
                                        
				case 2:	// Charge
					if ( !dfScrChargeTime-- )
						dfScrChargeTime = 3;
					vret = 1;
					break;
                                        
				case 3:	// Stealth
					if ( !dfStealthPuffsCnt-- )
						dfStealthPuffsCnt = 20;
					vret = 1;
					break;                                        
			}
			break;
	}

	if ( vret )
	{
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}

	return vret;
}


//-----------------------------------------------------------------------------

__myevic__ void Object3DOnEnter()
{
	//if ( dfStatus2.anim3d )
		CurrentMenuItem = Object3D - 1;
	//else
	//	CurrentMenuItem = 0;
}

__myevic__ void Object3DOnClick()
{
	Object3D = CurrentMenuItem + 1; // ? : Object3D;
	//if ( CurrentMenuItem ) dfStatus2.anim3d = 1;
	//else dfStatus2.anim3d = 0;
        
        UpdateDFTimer = 50;
}


//-----------------------------------------------------------------------------

__myevic__ void GameMEnter()
{
    CurrentMenuItem = FBSpeed;
}

__myevic__ void GameISelect()
{
    if ( CurrentMenuItem < 3 )
    {
	FBSpeed = CurrentMenuItem;
	UpdateDFTimer = 50;
    }
}

__myevic__ void GameIClick()
{
    if ( CurrentMenuItem < 3 )
    {
	if ( FBSpeed > 2 ) FBSpeed = 1;
        fbStartGame();
    }
}

__myevic__ void GameTtMEnter()
{
	CurrentMenuItem = dfTTSpeed;
}

__myevic__ void GameTtISelect()
{
    if ( CurrentMenuItem < 3 )
    {
	dfTTSpeed = CurrentMenuItem;
	UpdateDFTimer = 50;
    }
}

__myevic__ void GameTtIClick()
{
    if ( CurrentMenuItem < 3 )
    {
        if ( dfTTSpeed > 2 ) dfTTSpeed = 1;
        ttStartGame();
    }
}

//-----------------------------------------------------------------------------

__myevic__ void ModesMEnter()
{
	dfModesSel &= ~( 1 << dfMode );
	UpdateDFTimer = 50;
}

__myevic__ void ModesIDraw( int it, int line, int sel )
{
	if ( it > 6 ) return;

	(sel?DrawImageInv+1:DrawImage+1)( 53, line+2,
			(dfMode==it) ? 0x9C : (dfModesSel&(1<<it)) ? 0xA9 : 0xB4 );
}

__myevic__ void ModesIClick()
{
	if (( CurrentMenuItem < 7 ) && ( CurrentMenuItem != dfMode ))
	{
		dfModesSel ^= ( 1 << CurrentMenuItem );
		if ( dfModesSel == 0x7f ) dfModesSel ^= ( 1 << 4 );
		UpdateDFTimer = 50;
		gFlags.refresh_display = 1;
	}
	//else
	//{
	//	UpdateDataFlash();
	//}
}

//-----------------------------------------------------------------------------

uint8_t CoilShift;
uint8_t CoilSelectedLock[4];
uint16_t *CoilSelectedRez;

__myevic__ void CoilsSelectRez( uint8_t mode )
{
	switch ( mode )
	{
		case 0 : CoilSelectedRez = &dfRezNI ; CoilSelectedLock[0] = dfStatus2.dfRezLockedNI ; break;
		case 1 : CoilSelectedRez = &dfRezTI ; CoilSelectedLock[1] = dfStatus2.dfRezLockedTI ; break;
		case 2 : CoilSelectedRez = &dfRezSS ; CoilSelectedLock[2] = dfStatus2.dfRezLockedSS ; break;
		case 3 : CoilSelectedRez = &dfRezTCR; CoilSelectedLock[3] = dfStatus2.dfRezLockedTCR; break;
		default:
			break;
	}
	CoilShift = mode << 2;
}


__myevic__ void CoilsISelect()
{
	if ( CurrentMenuItem > 3 )
	{
		gFlags.edit_value = 0;
		return;
	}
	CoilsSelectRez( CurrentMenuItem );
}


__myevic__ void CoilsMEnter()
{
	if ( dfMode < 4 ) CurrentMenuItem = dfMode;
	else CurrentMenuItem = 4;
	CoilsISelect( CurrentMenuItem );
}


__myevic__ void CoilsIDraw( int it, int line, int sel )
{
    //manage
	switch ( it )
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			int rez = 0;
			short img = 0xC0;
			CoilsSelectRez( it );
			rez = *CoilSelectedRez * 10;
			rez += ( dfMillis >> CoilShift ) & 0xf;
			if ( CoilSelectedLock[it] ) img = 0xC3;
			DrawFillRect( 26, line, 63, line+12, 0 );
			DrawValue( 28, line+2, rez, 3, 0x0B, 4 );
			DrawImage( 56, line+2, img );
			CoilsSelectRez( CurrentMenuItem );
			break;
		} 
	}

	if ( gFlags.edit_value && sel )
	{
		InvertRect( 0, line, 63, line+12 );
	}
        

        ProbeAtoSeries();
        DrawValue( 15, 117, AtoRezMilli, 3, 0x0B, 4 ); 
	DrawImage( 43, 117, 0xC0 );  
        gFlags.refresh_display = 1;
}

__myevic__ void CoilsIClick()
{
	switch ( CurrentMenuItem )
	{
		case 4 :	// Zero All
			dfRezNI  = 0; dfStatus2.dfRezLockedNI  = 0;
			dfRezTI  = 0; dfStatus2.dfRezLockedTI  = 0;
			dfRezSS  = 0; dfStatus2.dfRezLockedSS  = 0;
			dfRezTCR = 0; dfStatus2.dfRezLockedTCR = 0;
			dfMillis = 0;
                                        
/*
                        //zero smart
			for ( int i = 0 ; i < 10 ; ++i )
			{
				dfSavedCfgRez[i] = 0;
				dfSavedCfgPwr[i] = 0;
			}
*/
                        
			ResetResistance();
			if ( AtoStatus == 4 )
			{
				SwitchRezLock( 1 );
			}
			break;
	}
        
//	if ( CurrentMenuItem == CurrentMenu->nitems - 1 )
//	{
//		UpdateDataFlash();
//	}
//	else
//	{
//		UpdateDFTimer = 50; // in ResetResistance
		gFlags.refresh_display = 1;
//	}
}

__myevic__ int CoilsMEvent( int event )
{
	int vret = 0;
        
	static char rmodified = 0;

	if ( CurrentMenuItem > 3 )
		return vret;

	int millis = ( dfMillis >> CoilShift ) & 0xf;
	int rez = *CoilSelectedRez * 10 + millis;
        
	if ( event != 1 ) rmodified = 0;

	switch ( event )
	{
		case 1: //fire
			gFlags.edit_value ^= 1;
			if ( !gFlags.edit_value && !rmodified && *CoilSelectedRez > 0 )
			{
				CoilSelectedLock[CurrentMenuItem] ^= 1;
			}
			//gFlags.refresh_display = 1;
			rmodified = 0;
			vret = 1;
			break;

		case 2: //plus
			if ( gFlags.edit_value )
			{
				if ( rez == 0 )
				{
					rez = 50;
				}
				else if ( rez < 1500 )
				{
					++rez;
				}
				CoilSelectedLock[CurrentMenuItem] = 1;
				rmodified = 1;
				vret = 1;
			}
			break;

		case 3: //minus
			if ( gFlags.edit_value )
			{
				if ( rez == 50 )
				{
					rez = 0;
					CoilSelectedLock[CurrentMenuItem] = 0;
				}
				else if ( rez > 50 )
				{
					--rez;
					CoilSelectedLock[CurrentMenuItem] = 1;
				}
				rmodified = 1;
				vret = 1;
			}
			break;

		case EVENT_LONG_FIRE: //zero current
			rez = 0;
			CoilSelectedLock[CurrentMenuItem] = 0;
			if ( CurrentMenuItem == dfMode )
			{
				ResetResistance();
				if ( AtoStatus == 4 )
				{
					rez = 10 * AtoRez + AtoMillis;
					CoilSelectedLock[CurrentMenuItem] = 1;
				}
			}
			rmodified = 1;
			gFlags.edit_value = 0;
			vret = 1;
			break;

	}

	if ( rmodified )
	{
                *CoilSelectedRez = rez / 10;
		millis = rez % 10;
		dfMillis &= ~( 0xf << CoilShift );
		dfMillis |= millis << CoilShift;

		UpdateDFTimer = 50;
		//gFlags.refresh_display = 1;

		if ( CurrentMenuItem == dfMode )
		{
			AtoRez = *CoilSelectedRez;
			AtoMillis = millis;

			dfResistance = *CoilSelectedRez;
			RezMillis = millis;

			SetAtoLimits();
		}
	}
        
        switch ( CurrentMenuItem )
        {
            case 0 : dfStatus2.dfRezLockedNI = CoilSelectedLock[0]; break;
            case 1 : dfStatus2.dfRezLockedTI = CoilSelectedLock[1]; break;
            case 2 : dfStatus2.dfRezLockedSS = CoilSelectedLock[2]; break;
            case 3 : dfStatus2.dfRezLockedTCR = CoilSelectedLock[3]; break;
            default:
		break;
                }
        
        gFlags.refresh_display = 1;
        
	return vret;
}


//-----------------------------------------------------------------------------

__myevic__ void DrawTCRP( int x, int y, int v, uint8_t dp, uint8_t z, uint8_t nd )
{
	if ( v == 0 )
	{
		DrawStringRight( String_DEF, x, y + 2 );
	}
	else
	{
		DrawValueRight( x, y + 2, v, dp, z, nd );
	}
}

//-----------------------------------------------------------------------------

__myevic__ void TCRSetIDraw( int it, int line, int sel )
{

	if ( it != 6 )
		return;

	DrawFillRect( 31, line, 63, line+12, 0 );

	//switch ( it )
	//{
	//	case 0:
        
        switch ( dfVWTempAlgo )
        {
            case 1:
            case 7:
                DrawStringRight( String_NI, 63, line+2 );
                break;
            case 2:
            case 8:    
                DrawStringRight( String_TI, 63, line+2 );
                break;
            case 3:
            case 9:    
                DrawStringRight( String_SS, 63, line+2 );
                break;
            case 4:
                DrawStringRight( String_M1, 63, line+2 );
                break;
            case 5:
                DrawStringRight( String_M2, 63, line+2 );
                break;
            case 6:
                DrawStringRight( String_M3, 63, line+2 );
                break;
                
            default:
		break;
	}

	if ( gFlags.edit_value && CurrentMenuItem == it )
	{
		InvertRect( 0, line, 63, line+12 );
	}
}

__myevic__ int TCRSetOnEvent( int event )
{
        int vret = 0;
    
	if ( CurrentMenuItem != 6 )
        {
            Event = EVENT_MODE_CHANGE;
            return 0;  //no capture others events
        }


	switch ( event )
	{                       
		case 1:	// Fire
			gFlags.edit_value ^= 1;  
                        vret = 1;
                        break;

		case 2:	// Plus
			if ( gFlags.edit_value )
			{
				if ( dfVWTempAlgo < 6 ) dfVWTempAlgo += 1;
				else if ( KeyTicks < 5 ) dfVWTempAlgo = 1;
                                
                                gFlags.refresh_display = 1;
                                return 1;
				//vret = 1; //not call EVENT_MODE_CHANGE

			}
			break;

		case 3:	// Minus
			if ( gFlags.edit_value )
			{
				if ( dfVWTempAlgo > 1 ) dfVWTempAlgo -= 1;
				else if ( KeyTicks < 5 ) dfVWTempAlgo = 6;
 
                                gFlags.refresh_display = 1;
                                return 1;
				//vret = 1;
			}
			break;
                        
                case EVENT_LONG_FIRE:
                            dfVWTempAlgo = 3;
                            gFlags.edit_value = 0;
                            vret = 1;   
                        break;
	}
        
        if ( vret )
        {
                UpdateDFTimer = 50;                     
                Event = EVENT_MODE_CHANGE;
        }
        
        gFlags.refresh_display = 1;
        
	return vret;
        
	//Event = EVENT_MODE_CHANGE;
	//return 0;
}


//-----------------------------------------------------------------------------

__myevic__ void DrawLedColor( int x, int y, int v, uint8_t dp, uint8_t z, uint8_t nd )
{
	DrawValueRight( x, y + 2, v * 4, dp, z, nd );
	DrawImage( x + 1, y + 2, 0xC2 );
}

__myevic__ void LedMenuEnter()
{
	LEDGetColor();
        
}

__myevic__ int LedMenuEvent( int event )
{
	LEDSetColor();
	return 0;
}

__myevic__ void MiscMenuOnClick()
{
    	switch ( CurrentMenuItem )
	{
            case 6:
                ResetAllCounters();
                break;
        }
	
}

__myevic__ int MiscMenuOnEvent( int event )
{
	int vret = 0;
                            
	if ( CurrentMenuItem != 8 )
		return vret;  //no capture others events

	switch ( event )
	{                       
                case EVENT_LONG_FIRE:
			//if ( CurrentMenuItem == 8 ) //reset dataflash
                        { 
                                ResetDFlashRes();			
                                RestartMod();
                                
                                //vret = 1;
                                //Event = EVENT_EXIT_MENUS;
                        }   
                        break;
	}
        
        //if ( vret )
        //{
        //        //UpdateDFTimer = 50;
        //        gFlags.refresh_display = 1;                                
        //}
	return vret;
}

//-----------------------------------------------------------------------------

/*
__myevic__ void CurveMenuOnClick()
{
	if ( CurrentMenuItem == 2 )
	{
		ResetPowerCurve();
	}
}
*/

__myevic__ void CurveMenuIDraw( int it, int line, int sel )
{
	//if ( it != 4 ) return;
        
	int v, dp, img;

	switch ( it )
	{       
		case 4:	// Time
			v = dfCurveRepeatTimer;
			dp = 1;
			img = 0x94;                                             
			break;

		default:
			return;
	}

	DrawFillRect( 38, line, 63, line+12, 0 );
        
        if ( v == 0 )
        {
            DrawStringRight( String_Off, 63, line+2 );
        }
        else
        {
            DrawValueRight( 56, line+2, v, dp, 0x0B, 0 );
            DrawImageRight( 63, line+2, img );
        }
        
	if ( sel && gFlags.edit_value )
		InvertRect( 0, line, 63, line+12 );
}

__myevic__ int CurveMenuOnEvent( int event )
{

	int vret = 0;

        
        if (event == EVENT_LONG_FIRE && CurrentMenuItem == 2 )
        { 
            ResetPowerCurve();
            vret = 1;
            Event = EVENT_POWER_CURVE;
        }
                            
	if ( CurrentMenuItem != 4 )
		return vret;  //no capture others events

	switch ( event )
	{                       
		case 1:	// Fire
                    //if ( CurrentMenuItem == 4 )
                    //{
			gFlags.edit_value ^= 1;
			//UpdateDFTimer = 50;
			//gFlags.refresh_display = 1;
                    //}
                    vret = 1;
                    break;

		case 2:	// Plus
			if ( gFlags.edit_value )
			{
                                //if ( CurrentMenuItem == 4 )
				//{
					if ( dfCurveRepeatTimer < 50 ) dfCurveRepeatTimer += 1;
					else if ( KeyTicks < 5 ) dfCurveRepeatTimer = 0;
				//}
				//UpdateDFTimer = 50;
				//gFlags.refresh_display = 1;
				vret = 1;
			}
			break;

		case 3:	// Minus
			if ( gFlags.edit_value )
			{
                                //if ( CurrentMenuItem == 4 )
				//{
					if ( dfCurveRepeatTimer > 0 ) dfCurveRepeatTimer -= 1;
					else if ( KeyTicks < 5 ) dfCurveRepeatTimer = 50;
				//}
				//UpdateDFTimer = 50;
				//gFlags.refresh_display = 1;
				vret = 1;
			}
			break;
                        
                case EVENT_LONG_FIRE:
			//if ( CurrentMenuItem == 4 )
                        //{ 
                            dfCurveRepeatTimer = 0;
                            //UpdateDFTimer = 50;
                            //gFlags.refresh_display = 1;
                            gFlags.edit_value = 0;
                            vret = 1;
                        //}   
                        break;
	}
        
        if ( vret )
        {
                UpdateDFTimer = 50;
                gFlags.refresh_display = 1;                                
        }
	return vret;
}


const menu_t GameMenu =
{
	String_Game,
	&MiscsMenu,
	GameMEnter+1,
	0,
	GameISelect+1,
	GameIClick+1,
	0,
	4,
	{
		{ String_Easy, 0, 0, 0 },
		{ String_Normal, 0, 0, 0 },
		{ String_Hard, 0, 0, 0 },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t Garbage =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	17
};

const menu_t GameTtMenu =
{
	String_Tetris,
	&MiscsMenu,
	GameTtMEnter+1,
	0,
	GameTtISelect+1,
	GameTtIClick+1,
	0,
	5,
	{
		{ String_Easy, 0, 0, 0 },
		{ String_Normal, 0, 0, 0 },
		{ String_Survival, 0, 0, 0 },
                { String_Garbage, &Garbage, 0, MACTION_DATA },    
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};
const menu_t ModesMenu =
{
	String_Modes,
	&VapingMenu,
	ModesMEnter+1,
	ModesIDraw+1,
	0,
	ModesIClick+1,
	0,
	8,
	{
		{ String_NI, 0, 0, 0 },
		{ String_TI, 0, 0, 0 },
		{ String_SS, 0, 0, 0 },
		{ String_TC, 0, 0, 0 },
		{ String_PW, 0, 0, 0 },
		{ String_BY, 0, 0, 0 },
		{ String_SM, 0, 0, 0 },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mvaluedesc_t PreheatDelayDesc =
{
	34, 42,
	3, 2,
	0, 255, // 180 0 - 3:00
	&DrawPreheatDelay+1,
	0,
	0,
	0x0B,
	1,
	-1, 0
};

const mdata_t PreheatDelayData =
{
	&dfPHDelay,
	&PreheatDelayDesc,
	MITYPE_BYTE,
	0
};

const mdata_t CurveDelayData =
{
	&dfCUDelay,
	&PreheatDelayDesc,
	MITYPE_BYTE,
	0
};

/*
const mdata_t PreheatSmartData =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	2
};
*/

const menu_t PreheatMenu =
{
	String_Preheat,
	&VapingMenu,
	0,
	PreheatIDraw+1,
	0,
	0,
	PreheatMEvent+1,
	7,
	{       
                { String_Enable, 0, 0, 0 },
		{ String_Unit, 0, 0, 0 },
		{ String_Pwr, 0, 0, 0 },
                { String_SMART, 0, 0, 0 },                                
		{ String_Time, 0, 0, 0 },
		{ String_Delay, &PreheatDelayData, 0, MACTION_DATA },
                //{ String_SMART, &PreheatSmartData, 0, MACTION_DATA },        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mvaluedesc_t TCRNIDesc =
{
	32, 63,
	0, 0,
	0, 999,     // 0 = DEF
	&DrawTCRP+1,
	0,
	0,
	0x0B,
	1,
	0, 600
};

const mvaluedesc_t TCRTIDesc =
{
	32, 63,
	0, 0,
	0, 999,
	&DrawTCRP+1,
	0,
	0,
	0x0B,
	1,
	0, 366
};

const mvaluedesc_t TCRSSDesc =
{
	32, 63,
	0, 0,
	0, 999,
	&DrawTCRP+1,
	0,
	0,
	0x0B,
	1,
	0, 100 //120
};

const mvaluedesc_t TCRMDesc =
{
	32, 63,
	0, 0,
	1, 999,
	0,
	0,
	0,
	0x0B,
	1,
	0, 100
};

const mdata_t TCRNIData =
{
	&dfTCRP[0],
	&TCRNIDesc,
	MITYPE_WORD,
	0
};

const mdata_t TCRTIData =
{
	&dfTCRP[1],
	&TCRTIDesc,
	MITYPE_WORD,
	0
};

const mdata_t TCRSSData =
{
	&dfTCRP[2],
	&TCRSSDesc,
	MITYPE_WORD,
	0
};

const mdata_t TCRM1Data =
{
	&dfTCRM[0],
	&TCRMDesc,
	MITYPE_WORD,
	0
};

const mdata_t TCRM2Data =
{
	&dfTCRM[1],
	&TCRMDesc,
	MITYPE_WORD,
	0
};

const mdata_t TCRM3Data =
{
	&dfTCRM[2],
	&TCRMDesc,
	MITYPE_WORD,
	0
};

const menu_t TCRSetMenu =
{
	String_TCR,
	&CoilsMenu,
	0,
	TCRSetIDraw+1, //0,
	0,
	0,
	&TCRSetOnEvent+1,
	8,
	{
		{ String_M1, &TCRM1Data, 0, MACTION_DATA },
		{ String_M2, &TCRM2Data, 0, MACTION_DATA },
		{ String_M3, &TCRM3Data, 0, MACTION_DATA },
		{ String_NI, &TCRNIData, 0, MACTION_DATA },
		{ String_TI, &TCRTIData, 0, MACTION_DATA },
		{ String_SS, &TCRSSData, 0, MACTION_DATA },
                { String_PW, 0, 0, 0 },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t CoilsMgmtMenu =
{
	String_Manage,
	&CoilsMenu,
	CoilsMEnter+1,
	CoilsIDraw+1,
	CoilsISelect+1,
	CoilsIClick+1,
	CoilsMEvent+1,
	6,
	{
		{ String_NI, 0, 0, 0 },
		{ String_TI, 0, 0, 0 },
		{ String_SS, 0, 0, 0 },
		{ String_TCR, 0, 0, 0 },
		{ String_Zero_All, 0, 0, 0 },                       
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t AutoProfileData =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	18
};

const menu_t CoilsMenu =
{
	String_Coils,
	&MainMenu,
	0,
	CoilsMenuIDraw+1,
	0,
	CoilsMenuOnClick+1,
	CoilsMenuOnEvent+1,
	8,
	{
		{ String_Manage, &CoilsMgmtMenu, 0, MACTION_SUBMENU },
		{ String_TCR, &TCRSetMenu, 0, MACTION_SUBMENU },
                { String_Profile, 0, 0, MACTION_SUBMENU }, //EVENT_PROFILE_MENU, MACTION_SUBMENU },
                { String_SMART, &AutoProfileData, 0, MACTION_DATA },
                { String_Cold, 0, 0, 0 },
                { String_New, 0, 0, 0 },
		{ String_Check, 0, 0, 0 },                         
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t Object3DMenu =
{
	String_3D,
	&ScreenMenu,
	Object3DOnEnter+1,
	0,
	0,
	Object3DOnClick+1,
	0,
	7,
	{
		//{ String_Off, 0, EVENT_PARENT_MENU, 0 }, //none
                { String_Square, 0, EVENT_PARENT_MENU, 0 },
		{ String_Tetra, 0, EVENT_PARENT_MENU, 0 },
		{ String_Box, 0, EVENT_PARENT_MENU, 0 },
		{ String_Octa, 0, EVENT_PARENT_MENU, 0 },
		//{ String_Dodeca, 0, EVENT_PARENT_MENU, 0 },
		//{ String_Isoca, 0, EVENT_PARENT_MENU, 0 },
                { String_TIE, 0, EVENT_PARENT_MENU, 0 },
                { String_Quartz, 0, EVENT_PARENT_MENU, 0 },     
                { String_Spinner, 0, EVENT_PARENT_MENU, 0 }  
	}
};

const mvaluedesc_t LedDesc =
{
	34, 53,
	0, 0,
	0, 25,
	DrawLedColor+1,
	0,
	0xC2,
	0x0B,
	1,
	26, 26
};

const mdata_t LedRedData =
{
	&LEDRed,
	&LedDesc,
	MITYPE_BYTE,
	0
};

const mdata_t LedGreenData =
{
	&LEDGreen,
	&LedDesc,
	MITYPE_BYTE,
	0
};

const mdata_t LedBlueData =
{
	&LEDBlue,
	&LedDesc,
	MITYPE_BYTE,
	0
};

const menu_t LedMenu =
{
	String_Led,
	&MiscsMenu,
	LedMenuEnter+1,
	0,
	0,
	0,
	LedMenuEvent+1,
	4,
	{
		{ String_Red, &LedRedData, 0, MACTION_DATA },
		{ String_Green, &LedGreenData, 0, MACTION_DATA },
		{ String_Blue, &LedBlueData, 0, MACTION_DATA },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t FireFlip =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	30
};
const mdata_t SwapMP =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	3
};
const mdata_t NBZC =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	12
};      
const mdata_t Puff24 =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	19 //dfStatus.puffday reset puffs counters
};
const menu_t MiscsMenu =
{
	String_Miscs,
	&MainMenu,
	0,
	0,
	0,
	MiscMenuOnClick+1,
	MiscMenuOnEvent+1,
	10,
	{
		{ String_Game, &GameMenu, 0, MACTION_SUBMENU },
                { String_Tetris, &GameTtMenu, 0, MACTION_SUBMENU },                        
		{ String_Led, &LedMenu, 0, MACTION_SUBMENU },
		//{ String_3D, &Object3DMenu, 0, MACTION_SUBMENU },
                //{ String_FiFlip, &FireFlip, 0, MACTION_DATA },     
                { String_SwapMP, &SwapMP, 0, MACTION_DATA },  
                { String_NewZC, &NBZC, 0, MACTION_DATA },
                { String_Puff24, &Puff24, 0, MACTION_DATA },        
                { String_ZeroCnts, 0, EVENT_EXIT_MENUS, 0 },                        
                { String_Version, 0, 29, 0 },
                { String_Reset, 0, 0, 0 },        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mbitdesc_t HmsDesc =
{
	36, 63,
	String_HM,
	String_hms
};

const mdata_t ClkSizeData =
{
	&dfStatus,
	&HmsDesc,
	MITYPE_BIT,
	24
};

const mdata_t OffModClock =
{
	&dfStatus,
	&InvBitDesc,
	MITYPE_BIT,
	29
};
const mdata_t OnModClock =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	4
};
const menu_t ClockMenu =
{
	String_Clock,
	&MainMenu,
	0,
	ClockMenuIDraw+1,
	0,
	ClockMenuOnClick+1,
	0,
	8,
	{
		{ String_Set, 0, EVENT_SET_TIME_DATE, MACTION_SUBMENU }, //0
		//{ String_Date, 0, EVENT_SET_DATE, MACTION_SUBMENU }, //0
		{ String_ClkAdjust, 0, EVENT_CLK_ADJUST, MACTION_SUBMENU }, //0
		{ String_ClkSpeed, 0, EVENT_CLK_SPEED, MACTION_SUBMENU }, //0
		{ String_Fmt, 0, 0, 0 },
		{ String_Size, &ClkSizeData, 0, MACTION_DATA },
		{ String_Dial, 0, 0, 0 },               
                { String_OffMod, &OffModClock, 0, MACTION_DATA },    
                //{ String_ON, &OnModClock, 0, MACTION_DATA },         
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t DimOffDelayData =
{
	&dfDimOffTimeout,
	&PreheatDelayDesc,
	MITYPE_BYTE,
	0
};

const menu_t ScreenProtMenu =
{
	String_Protection,
	&ScreenMenu,
	0,
	ScreenProtMenuIDraw+1,
	0,
	ScreenProtMenuOnClick+1,
	ScreenProtMenuOnEvent+1,
	7,
	{
		{ String_Saver, 0, 0, 0 },
		{ String_Main, 0, 0, 0 },
                { String_Charge, 0, 0, 0 },
                { String_Stealth, 0, 0, 0 },        
                { String_Mode, 0, 0, 0 },                        
                { String_Sleep, &DimOffDelayData, 0, MACTION_DATA }, 
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mvaluedesc_t BVODesc =
{
	32, 56,
	2, 0,
	BVO_MIN, BVO_MAX,
	0,
	0,
	0,
	0x0B,
	1,
	127, 0
};

const mdata_t BVO1Data =
{
	&dfBVOffset[0],
	&BVODesc,
	MITYPE_SBYTE,
	0
};

const mdata_t BVO2Data =
{
	&dfBVOffset[1],
	&BVODesc,
	MITYPE_SBYTE,
	0
};

const mdata_t BVO3Data =
{
	&dfBVOffset[2],
	&BVODesc,
	MITYPE_SBYTE,
	0
};

const mdata_t BVO4Data =
{
	&dfBVOffset[3],
	&BVODesc,
	MITYPE_SBYTE,
	0
};

const menu_t BVOMenu =
{
	String_Batteries,
	&ExpertMenu,
	0,
	BVOMenuIDraw+1,
	0,
	BVOMenuOnClick+1, //0,
	0,
	6,
	{
		{ String_B1, &BVO1Data, 0, MACTION_DATA },
		{ String_B2, &BVO2Data, 0, MACTION_DATA },
		{ String_B3, &BVO3Data, 0, MACTION_DATA },
		{ String_B4, &BVO4Data, 0, MACTION_DATA },
                { String_AMP_s, 0, 0, MACTION_SUBMENU },//show batts volts                            
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mvaluedesc_t CUSVoltDesc =
{
	30, 40,
	3, 2,
	270, 423,
	&DrawCUS+1,
	0,
	0,
	0x0B,
	1,
	424, 0
};
const mvaluedesc_t CUSPercDesc =
{
	30, 40,
	2, 0,
	1, 99,
	&DrawCUS+1,
	0,
	0,
	0x0B,
	1,
	100, 0
};
const mdata_t CUS1Voltage =
{
	&CustomBattery.V2P[0].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS2Voltage =
{
	&CustomBattery.V2P[1].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS3Voltage =
{
	&CustomBattery.V2P[2].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS4Voltage =
{
	&CustomBattery.V2P[3].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS5Voltage =
{
	&CustomBattery.V2P[4].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS6Voltage =
{
	&CustomBattery.V2P[5].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS7Voltage =
{
	&CustomBattery.V2P[6].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS8Voltage =
{
	&CustomBattery.V2P[7].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS9Voltage =
{
	&CustomBattery.V2P[8].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS10Voltage =
{
	&CustomBattery.V2P[9].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS11Voltage =
{
	&CustomBattery.V2P[10].voltage,
	&CUSVoltDesc,
	MITYPE_WORD,
	0
};

//
const mdata_t CUS2Percent =
{
	&CustomBattery.V2P[1].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS3Percent =
{
	&CustomBattery.V2P[2].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS4Percent =
{
	&CustomBattery.V2P[3].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS5Percent =
{
	&CustomBattery.V2P[4].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS6Percent =
{
	&CustomBattery.V2P[5].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS7Percent =
{
	&CustomBattery.V2P[6].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS8Percent =
{
	&CustomBattery.V2P[7].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS9Percent =
{
	&CustomBattery.V2P[8].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};
const mdata_t CUS10Percent =
{
	&CustomBattery.V2P[9].percent,
	&CUSPercDesc,
	MITYPE_WORD,
	0
};

const menu_t CUSMenu =
{
	String_CUS,
	&ExpertMenu,
	0,
	CUSMenuIDraw+1,
	0,
	0, //CUSMenuOnClick+1,
	CUSMenuOnEvent+1,
	24,
	{
		{ String_Percent, 0, 0, 0 },	
                { String_V, &CUS1Voltage, 0, MACTION_DATA },
		{ String_Percent, &CUS2Percent, 0, MACTION_DATA },
                { String_V, &CUS2Voltage, 0, MACTION_DATA },        
                { String_Percent, &CUS3Percent, 0, MACTION_DATA },
                { String_V, &CUS3Voltage, 0, MACTION_DATA },        
		{ String_Percent, &CUS4Percent, 0, MACTION_DATA },        
                { String_V, &CUS4Voltage, 0, MACTION_DATA },                          
                { String_Percent, &CUS5Percent, 0, MACTION_DATA },        
                { String_V, &CUS5Voltage, 0, MACTION_DATA },  
		{ String_Percent, &CUS6Percent, 0, MACTION_DATA },        
                { String_V, &CUS6Voltage, 0, MACTION_DATA },  
		{ String_Percent, &CUS7Percent, 0, MACTION_DATA },        
                { String_V, &CUS7Voltage, 0, MACTION_DATA },  
		{ String_Percent, &CUS8Percent, 0, MACTION_DATA },        
                { String_V, &CUS8Voltage, 0, MACTION_DATA },  
		{ String_Percent, &CUS9Percent, 0, MACTION_DATA },        
                { String_V, &CUS9Voltage, 0, MACTION_DATA },  
		{ String_Percent, &CUS10Percent, 0, MACTION_DATA },        
                { String_V, &CUS10Voltage, 0, MACTION_DATA },  
		{ String_Percent, 0, 0, 0 },        
                { String_V, &CUS11Voltage, 0, MACTION_DATA },    
                { String_Save, 0, 0, 0 }, 
                { String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t MAXMenu =
{
	String_MAX_s,
	&ExpertMenu,
	0,
	MaxMenuIDraw+1,
	0,
	MaxMenuOnClick+1,
	MaxMenuOnEvent+1,
	7,
	{
		{ String_PWR_s, 0, 0, 0 },	       
                { String_VOLT_s, 0, 0, 0 },
		{ String_TEMP_s, 0, 0, 0 },
                { String_UCH_s, 0, 0, 0 },
                { String_BY, 0, 0, 0 },
                { String_BATT_s, 0, 0, 0 },                        
                { String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t ExpertMenu =
{
	String_Expert,
	&MainMenu,
	0,
	ExpertMenuIDraw+1,
	0,
	ExpertMenuOnClick+1,
	ExpertMenuOnEvent+1,
	10,
	{
                { String_Batteries, &BVOMenu, 0, MACTION_SUBMENU },
                { String_X32, 0, 0, 0 },
                { String_LSL, 0, 0, 0 }, 
                { String_SHR, 0, 0, 0 }, 
                { String_UCH_s, 0, 0, 0 }, 
                { String_BATT_s, 0, 0, 0 },        
		//{ String_USB, 0, 0, 0 },
		//{ String_DBG, 0, 0, 0 },
		//{ String_PCT, 0, 0, 0 },
                { String_TEMP_s, 0, 0, 0 },  //6
                { String_CUS, &CUSMenu, 0, MACTION_SUBMENU },  //7
                { String_MAX_s, &MAXMenu, 0, MACTION_SUBMENU },   //8      
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t ScreenSaveMenu =
{
	String_Saver,
	&ScreenMenu,
	ScreenSaveOnEnter+1,
	0,
	ScreenSaveOnSelect+1,
	0,
	0,
	8,
	{
		{ String_Off, 0, EVENT_PARENT_MENU, 0 }, //none
		{ String_3D, 0, EVENT_PARENT_MENU, 0 },    
		{ String_Qix, 0, EVENT_PARENT_MENU, 0 },
		{ String_Snow, 0, EVENT_PARENT_MENU, 0 },                        
                { String_Vaped, 0, EVENT_PARENT_MENU, 0 },                        
		{ String_Clock, 0, EVENT_PARENT_MENU, 0 },
		{ String_Logo, 0, EVENT_PARENT_MENU, 0 },
		{ String_Splash, 0, EVENT_PARENT_MENU, 0 }
	}
};

/*
const mbitdesc_t TopBotDesc =
{
	0, 0,
	String_Mid,
	String_Top
};
*/

const mdata_t Show3DData =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	0 //dfStatus2.anim3d on/off
};

/*
const mdata_t LogoShowData =
{
	&dfStatus,
	&InvBitDesc,
	MITYPE_BIT,
	3
};

const mdata_t LogoWhereData =
{
	&dfStatus,
	&TopBotDesc,
	MITYPE_BIT,
	23
};
*/
const mvaluedesc_t HideLogoDesc =
{
	40, 55,
	0, 0,
	0, 15,
	0,
	0,
	0x94,
	0x0B,
	1,
	16, 3
};
const mdata_t HideLogoData =
{
	&dfHideLogo,
	&HideLogoDesc,
	MITYPE_BYTE,
	0
};
const menu_t LogoMenu =
{
	String_Logo,
	&ScreenMenu,
	0,
	LogoMenuIDraw+1, //0,
	0,
	LogoMenuOnClick+1, //0,
	0,
	5,
	{
	        //{ String_3D, &Object3DMenu, 0, MACTION_SUBMENU },
                { String_3D, &Show3DData, 0, MACTION_DATA },
                { String_Clock, &OnModClock, 0, MACTION_DATA },  
                { String_Logo, 0, 0, 0 },
		//{ String_Where, &LogoWhereData, 0, MACTION_DATA },
                { String_Hide, &HideLogoData, 0, MACTION_DATA },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t ScreenInvData =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	18
};

const menu_t ScreenMenu =
{
	String_Screen,
	&MainMenu,
	0,
	ScreenMenuIDraw+1,
	0,
	ScreenMenuOnClick+1,
	ScreenMenuOnEvent+1,
	9,
	{
		{ String_Contrast, 0, EVENT_EDIT_CONTRAST, MACTION_SUBMENU }, //0
		{ String_Protection, &ScreenProtMenu, 0, MACTION_SUBMENU },
		{ String_Saver, &ScreenSaveMenu, 0, MACTION_SUBMENU },
		{ String_Logo, &LogoMenu, 0, MACTION_SUBMENU },
                { String_3D, &Object3DMenu, 0, MACTION_SUBMENU },
                { String_FireScrDur, 0, 0, 0 }, //5                        
		{ String_Invert, &ScreenInvData, EVENT_INVERT_SCREEN, MACTION_DATA },
                { String_FiFlip, &FireFlip, 0, MACTION_DATA },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const menu_t ClicksMenu =
{
	String_Clicks,
	&IFMenu,
	0,
	ClicksMenuIDraw+1,
	0,
	ClicksMenuOnClick+1,
	ClicksMenuOnEvent+1,
	6,
	{
		{ String_2, 0, 0, 0 },
		{ String_3, 0, 0, 0 },
		{ String_4, 0, 0, 0 },
                { String_5, 0, 0, 0 },
                { String_FMP, 0, 0, 0 },                        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t Session =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	16
};

const menu_t IFMenu =
{
	String_Interface,
	&MainMenu,
	0,
	IFMenuIDraw+1,
	0,
	IFMenuOnClick+1,
	0,
	10,
	{
                { String_Clicks, &ClicksMenu, 0, MACTION_SUBMENU },
		{ String_1Watt, 0, 0, 0 },
		{ String_1C5F, 0, 0, 0 },
		{ String_WakeMP, 0, 0, 0 },
		{ String_Temp, 0, 0, 0 },
		{ String_PPwr, 0, 0, 0 },
                { String_UI, 0, 0, 0 },
                { String_Splash, 0, 0, 0 },
                { String_Session, &Session, 0, MACTION_DATA },        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};


const mvaluedesc_t BoostDesc =
{
	34, 52,
	0, 0,
	0, 100,
	0,
	0,
	0xC2,
	0x0B,
	1,
	101, 50
};

const mdata_t BoostData =
{
	&dfTCBoost,
	&BoostDesc,
	MITYPE_WORD,
	0
};

const mvaluedesc_t PIDPDesc =
{
	24, 63,
	0, 0,
	PID_P_MIN, PID_P_MAX,
	0,
	0,
	0,
	0x0B,
	1,
	PID_P_MAX+1, PID_P_DEF
};

const mdata_t PIDPData =
{
	&dfPID.P,
	&PIDPDesc,
	MITYPE_WORD,
	0
};

const mvaluedesc_t PIDIDesc =
{
	24, 63,
	0, 0,
	PID_I_MIN, PID_I_MAX,
	0,
	0,
	0,
	0x0B,
	1,
	PID_I_MAX+1, PID_I_DEF
};

const mdata_t PIDIData =
{
	&dfPID.I,
	&PIDIDesc,
	MITYPE_WORD,
	0
};

const mvaluedesc_t PIDDDesc =
{
	24, 63,
	0, 0,
	PID_D_MIN, PID_D_MAX,
	0,
	0,
	0,
	0x0B,
	1,
	PID_D_MAX+1, PID_D_DEF
};

const mdata_t PIDDData =
{
	&dfPID.D,
	&PIDDDesc,
	MITYPE_WORD,
	0
};

const menu_t AlgoMenu =
{
	String_Algo,
	&VapingMenu,
	0,
	AlgoMenuIDraw+1,
	0,
	AlgoMenuOnClick+1,
	0,
	6,
	{
		{ String_Algo, 0, 0, 0 },
		{ String_Boost, &BoostData, 0, MACTION_DATA },
		{ String_P, &PIDPData, 0, MACTION_DATA },
		{ String_I, &PIDIData, 0, MACTION_DATA },
		{ String_D, &PIDDData, 0, MACTION_DATA },
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};


const mbitdesc_t CurveEnaDesc =
{
	40, 63,
	String_On, //Yes,
	String_Off //No
};

const mdata_t CurveEnaData =
{
	&dfStatus,
	&CurveEnaDesc,
	MITYPE_BIT,
	28
};

const menu_t CurveMenu =
{
	String_Curve,
	&VapingMenu,
	0,
	CurveMenuIDraw+1,
	0,
	0, //CurveMenuOnClick+1,
	CurveMenuOnEvent+1,
	6,
	{
		{ String_Enable, &CurveEnaData, 0, MACTION_DATA },
                { String_Edit, 0, EVENT_POWER_CURVE, MACTION_SUBMENU }, //0
		{ String_Reset, 0, 0, 0 }, //EVENT_POWER_CURVE	
		{ String_Delay, &CurveDelayData, 0, MACTION_DATA },
                { String_Repeat, 0, 0, 0 },                        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};

const mdata_t VVLite =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	31
};
const mdata_t APuffTime =
{
	&dfStatus,
	&BitDesc,
	MITYPE_BIT,
	6
};
const mdata_t Replay =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	15
};
const mdata_t PwrLow =
{
	&dfStatus2,
	&BitDesc,
	MITYPE_BIT,
	19
};

const menu_t VapingMenu =
{
	String_Vaping,
	&MainMenu,
	0,
	VapingMenuIDraw+1,
	0,
	VapingMenuOnClick+1,
	VapingMenuOnEvent+1,
	15,
	{
		{ String_Preheat, &PreheatMenu, 0, MACTION_SUBMENU },
		{ String_Curve, &CurveMenu, 0, MACTION_SUBMENU },
                { String_Algo, &AlgoMenu, 0, MACTION_SUBMENU },
                { String_Modes, &ModesMenu, 0, MACTION_SUBMENU },
		{ String_Vaped, 0, EVENT_SET_JOULES, MACTION_SUBMENU }, //ShowSetJoules()
                { String_Prot, 0, 0, 0 },                        
                { String_PuffsOff, 0, 0, 0 }, //6
                { String_VapeTimeOff, 0, 0, 0 }, //7
                { String_HoldFi, 0, 0, 0 },     //8   
                { String_VVLite, &VVLite, 0, MACTION_DATA }, //9
                { String_AutoFi, &APuffTime, 0, MACTION_DATA }, //10 
                { String_ATime, 0, 0, MACTION_DATA },        //11
                { String_Repeat, &Replay, 0, MACTION_DATA },
                { String_PwrLo, &PwrLow, 0, MACTION_DATA },                        
		{ String_Back, 0, EVENT_PARENT_MENU, 0 }
	}
};


const menu_t ProfileMenu =
{
	String_Profile,
	0,
	0,
	ProfileMenuIDraw+1,
	0,
	0,
	ProfileMenuOnEvent+1,
	6,
	{
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ String_Exit, 0, EVENT_EXIT_MENUS, 0 }
	}
};


const menu_t MainMenu =
{
	String_Menus,
	0,
	0,
	0,
	0,
	0,
	0,
	8,
	{
		{ String_Screen, &ScreenMenu, 0, MACTION_SUBMENU },
		{ String_Coils, &CoilsMenu, 0, MACTION_SUBMENU },
		{ String_Vaping, &VapingMenu, 0, MACTION_SUBMENU },
		{ String_Clock, &ClockMenu, 0, MACTION_SUBMENU },
		{ String_Interface, &IFMenu, 0, MACTION_SUBMENU },
		{ String_Expert, &ExpertMenu, 0, MACTION_SUBMENU },
                { String_Miscs, &MiscsMenu, 0, MACTION_SUBMENU },       
		{ String_Exit, 0, EVENT_EXIT_MENUS, 0 }
	}
};

//-----------------------------------------------------------------------------

__myevic__ void DrawMenuData( int line, int sel, const mdata_t *data )
{
	uint32_t const *p = data->ptr;

	switch ( data->type )
	{
		case MITYPE_BIT:
		{
			uint32_t b;
			const mbitdesc_t *desc = data->desc;

			p += data->bitpos / 32;
			b = 1 << ( data->bitpos % 32 );
			b &= *p;

			if ( !desc )
			{
				DrawFillRect( 40, line, 63, line+12, 0 );
				DrawStringRight( b ? String_On : String_Off, 63, line+2 ); //44
			}
			else
			{
				DrawFillRect( desc->div ? : 40, line, 63, line+12, 0 );
				DrawStringRight( b ? desc->on : desc->off, desc->pos ? : 63, line+2 );
			}
			break;
		}

		case MITYPE_BYTE:
		case MITYPE_WORD:
		{
			uint16_t v;

			if ( data->type == MITYPE_BYTE )
			{
				v = *(const uint8_t*)p;
			}
			else
			{
				v = *(const uint16_t*)p;
			}

			const mvaluedesc_t *desc = data->desc;

			DrawFillRect( desc->div, line, 63, line+12, 0 );

			if ( desc->draw )
			{
				desc->draw( desc->posr, line, v, desc->dp, desc->z, desc-> nd );
			}
			else
			{
				DrawValueRight( desc->posr, line+2, v, desc->dp, desc->z, desc-> nd );
				//if ( desc->unit_s ) // not used 
				//{
				//	DrawStringRight( desc->unit_s, 63, line+2 ); //desc->posr + 1
				//}
				//else 
                                if ( desc->unit_c )
				{
					DrawImageRight( 63, line+2, desc->unit_c );
				}
			}

			if ( sel && gFlags.edit_value )
			{
				InvertRect( 0, line, 63, line+12 );
			}

			break;
		}
	}
}


//-----------------------------------------------------------------------------

__myevic__ void DrawMenu()
{
	unsigned int firstitem;

	if ( CurrentMenu == 0 )
	{
		CurrentMenu = &MainMenu;
		CurrentMenuItem = 0;
	}

	if ( !CurrentMenu->nitems )
	{
		MainView();
		return;
	}

	if ( CurrentMenuItem >= CurrentMenu->nitems ) CurrentMenuItem = 0;

	DrawString( CurrentMenu->caption, 4, 3 ); //4 5
	DrawHLine( 0, 14, 63, 1 ); //0 16

	if (( CurrentMenu->nitems > 8 ) && ( CurrentMenuItem > 3 ))
	{
		if ( CurrentMenuItem > CurrentMenu->nitems-4 )
		{
			firstitem = CurrentMenu->nitems - 8;
		}
		else
		{
			firstitem = CurrentMenuItem - 4;
		}
	}
	else
	{
		firstitem = 0;
	}

	for ( int i = 0; i < 8; ++i )
	{
		if ( firstitem + i >= CurrentMenu->nitems ) break;

		mitem_t const *mi = &CurrentMenu->mitems[firstitem+i];

		int line = 16 + 14 * i; //18 + 14 * i;

		if ( mi->caption )
		{
			if ( CurrentMenuItem == firstitem + i )
			{
				DrawFillRect( 0, line, 63, line+12, 1 );
                                DrawStringInv( mi->caption, 2, line+2 );
                                if ( mi->action_type == MACTION_SUBMENU )
                                    DrawImageInv( 58, line+2, 0xD4 ); // |>
			}
			else
			{
				DrawString( mi->caption, 2, line+2 );
                                if ( mi->action_type == MACTION_SUBMENU )
                                    DrawImage( 58, line+2, 0xD4 );
			}
		}

		if ( mi->action_type == MACTION_DATA )
		{
			DrawMenuData( line, CurrentMenuItem == firstitem + i, mi->action );
		}

		if ( CurrentMenu->on_drawitem )
		{
			CurrentMenu->on_drawitem( firstitem + i, line, CurrentMenuItem == firstitem + i ); //18+14*i
		}
	}
}

//-----------------------------------------------------------------------------

__myevic__ int MenuDataAction( int event, const mdata_t *data )
{
	int vret = 0;

	switch ( event )
	{
		case 1:	// Fire
		{
			switch ( data->type )
			{
				case MITYPE_BIT:
				{
					uint32_t *p = data->ptr;
					p += data->bitpos / 32;
					uint32_t mask = 1 << ( data->bitpos % 32 );
					*p ^= mask;
					vret = 1;
					break;
				}

				case MITYPE_BYTE:
				case MITYPE_SBYTE:
				case MITYPE_WORD:
					gFlags.edit_value ^= 1;
					vret = 1;
					break;
			}

			Event = CurrentMenu->mitems[CurrentMenuItem].event;

			break;
		}

		case 2:	// Plus
		{
			if ( !gFlags.edit_value )
				break;

			const mvaluedesc_t *desc = data->desc;

			switch ( data->type )
			{
				case MITYPE_BYTE:
				{
					uint8_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					*p += desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( *p > desc->max )
					{
						*p = ( KeyTicks < 5 ) ? desc->min : desc->max;
					}
					break;
				}

				case MITYPE_SBYTE:
				{
					int8_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					*p += desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( *p > desc->max )
					{
						*p = ( KeyTicks < 5 ) ? desc->min : desc->max;
					}
					break;
				}

				case MITYPE_WORD:
				{
					int16_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					*p += desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( *p > desc->max )
					{
						*p = ( KeyTicks < 5 ) ? desc->min : desc->max;
					}
					break;
				}
			}

			break;
		}

		case 3:	// Minus
		{
			if ( !gFlags.edit_value )
				break;

			const mvaluedesc_t *desc = data->desc;

			switch ( data->type )
			{
				case MITYPE_BYTE:
				{
					uint8_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					uint16_t inc = desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( ( *p < desc->min + inc ) || ( ( *p -= inc ) < desc->min ) )
					{
						*p = ( KeyTicks < 5 ) ? desc->max : desc->min;
					}
					break;
				}

				case MITYPE_SBYTE:
				{
					int8_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					uint16_t inc = desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( ( *p -= inc ) < desc->min )
					{
						*p = ( KeyTicks < 5 ) ? desc->max : desc->min;
					}
					break;
				}

				case MITYPE_WORD:
				{
					int16_t *p = data->ptr;

					vret = 1;

					if ( *p == desc->def1 )
						break;

					uint16_t inc = desc->inc && ( KeyTicks >= 105 ) ? 10 : 1;

					if ( ( *p -= inc ) < desc->min )
					{
						*p = ( KeyTicks < 5 ) ? desc->max : desc->min;
					}
					break;
				}
			}

			break;
		}

		case EVENT_LONG_FIRE:
		{
			const mvaluedesc_t *desc = data->desc;

			switch ( data->type )
			{
                                case MITYPE_BYTE:
				case MITYPE_WORD:
				{
					int16_t *p = data->ptr;
					if ( *p == desc->def1 )
					{
						*p = desc->def2;
						gFlags.edit_value = 1;
						vret = 1;
					}
					else if ( desc->def1 >= desc->min && desc->def1 <= desc->max )
					{
						*p = desc->def1;
						gFlags.edit_value = 0;
						vret = 1;
					}
					else if ( desc->def2 >= desc->min && desc->def2 <= desc->max )
					{
						*p = desc->def2;
						gFlags.edit_value = 0;
						vret = 1;
					}
					break;
				}
			}

			break;
		}

		default:
			// Other events to come
			break;
	}

	if ( vret )
	{
		UpdateDFTimer = 50;
		ScreenDuration = 30; //15;
		gFlags.refresh_display = 1;
	}

	return vret;
}


//-----------------------------------------------------------------------------

__myevic__ int MenuEvent( int event )
{
	int vret = 0;

	if ( CurrentMenu && CurrentMenu->on_event )
	{
		ScreenDuration = 15;
		vret = CurrentMenu->on_event( event );
		if ( vret ) return vret;
	}

	mitem_t const *mi = &CurrentMenu->mitems[CurrentMenuItem];

	if ( mi->action )
	{
		switch ( mi->action_type )
		{
			case MACTION_DATA:
			{
				vret = MenuDataAction( event, (const mdata_t*)mi->action );
				if ( vret ) return vret;
				break;
			}
		}
	}

	if ( ISEGRIPII )
	{
		if ( event == 2 ) event = 3;
		else if ( event == 3 ) event = 2;
	}

	switch ( event )
	{
		case 1: //fire
		{
			ScreenDuration = 15; // menu timeout
			if ( CurrentMenu->on_clickitem ) CurrentMenu->on_clickitem();

			if ( mi->action )
			{
				switch ( mi->action_type )
				{
					case MACTION_SUBMENU:
					{
						CurrentMenu = (const menu_t*)mi->action;
                                                
                                                PrevMenuItem = CurrentMenuItem;
						CurrentMenuItem = 0;
                                                
						if ( CurrentMenu->on_enter ) CurrentMenu->on_enter();

						gFlags.refresh_display = 1;
						break;
					}
				}
			}
			else if ( mi->event > 0 )
			{
				Event = mi->event;
			}

			vret = 1;

		}	break;

		case 2: //plus
		{
			if ( CurrentMenuItem < CurrentMenu->nitems - 1 )
			{
				++CurrentMenuItem;
			}
			else
			{
				CurrentMenuItem = 0;
			}
                        
			ScreenDuration = 15;
			gFlags.refresh_display = 1;

			if ( CurrentMenu->on_selectitem ) CurrentMenu->on_selectitem();

			vret = 1;

		}	break;

		case 3: //minus
		{
			if ( CurrentMenuItem )
			{
				--CurrentMenuItem;
			}
			else
			{
				CurrentMenuItem = CurrentMenu->nitems - 1;
			}
                        
			ScreenDuration = 15;
			gFlags.refresh_display = 1;

			if ( CurrentMenu->on_selectitem ) CurrentMenu->on_selectitem();

			vret = 1;

		}	break;

		case 15:  //short fire
			vret = 1;
			break;
                                              
		case 39:
			CurrentMenu = &TCRSetMenu;
			CurrentMenuItem = 0;
			SetScreen( 102, 15 );
			vret = 1;
			break;

		case EVENT_EXIT_MENUS:
			EditModeTimer = 0;
			gFlags.edit_capture_evt = 0;
			gFlags.edit_value = 0;
			//LEDOff();
                        if ( !CheckCustomBattery() ) ResetCustomBattery();
			UpdateDataFlash();
                         //               DisplaySetContrast( dfContrast );
			MainView();
			vret = 1;
			break;

		case EVENT_PARENT_MENU:
		{
			switch ( Screen )
			{
				case 101:
                                       // DisplaySetContrast( dfContrast );
                                       // gFlags.refresh_display = 1;
					CurrentMenu = &ScreenMenu;
					break;

				case 102:
					if ( CurrentMenu )
					{
						CurrentMenu = CurrentMenu->parent;
					}
					break;

				case 103:
				case 104:
				case 105:
				//case 106:
                                        PrevMenuItem = 0;
					CurrentMenu = &ClockMenu;
					break;

				case 107:
					CurrentMenu = &CurveMenu;
					break;
                                        
                                case EVENT_SET_JOULES:
                                        PrevMenuItem = 6;
					CurrentMenu = &VapingMenu;
					break;
                                    
				default:
					CurrentMenu = 0;
					break;
			}

			if ( !dfStatus.off && CurrentMenu )
			{
				//CurrentMenuItem = 0;
                                CurrentMenuItem = PrevMenuItem;
                                PrevMenuItem = 0;
				if ( CurrentMenu->on_enter ) CurrentMenu->on_enter();
				SetScreen( 102, 15 );
			}
			else
			{
				MainView();
			}

			EditModeTimer = 0;
			gFlags.edit_capture_evt = 0;
			gFlags.edit_value = 0;
			//LEDOff();
			vret = 1;
			break;
		}

		case EVENT_TOGGLE_TDOM:
			CurrentMenu = &PreheatMenu;
			CurrentMenuItem = 1;
			SetScreen( 102, 15 );
			vret = 1;
			break;

		case EVENT_PROFILE_MENU:
			CurrentMenu = &ProfileMenu;
			CurrentMenuItem = dfProfile;
			SetScreen( 102, 15 );       
			vret = 1;
			break;
                                              
		default:
			break;
	}

	return vret;
}


