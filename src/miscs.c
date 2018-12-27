#include "myevic.h"
#include "dataflash.h"
#include "display.h"
#include "atomizer.h"
#include "miscs.h"
#include "timers.h"

#include "screens.h"

//=========================================================================
//----- (00004FB0) --------------------------------------------------------
__myevic__ void ModeChange()
{
	//dfResistance = AtoRez; //in 4
	//RezMillis = AtoMillis;
    
//dfTempAlgo 1 Ni, 2 Ti, 3 SS, 4 M1, 5 M2, 6 M3, 7 tcrNi, 8 tcrTi, 9 tcrSS
    
	switch ( dfMode )
	{
		case 0: // Temp Ni
			if ( dfTCRP[dfMode] == 0 ) dfTempAlgo = 1;
			else dfTempAlgo = 7; //or simple TCR

			if ( dfRezNI )
			{
				if (( dfStatus2.dfRezLockedNI ) || !( gFlags.new_rez_ni ))
				{
					dfResistance = dfRezNI;
					RezMillis = dfMillis & 0xf;
				}
			}
			break;

		case 1: // Temp Ti
			if ( dfTCRP[dfMode] == 0 ) dfTempAlgo = 2;
			else dfTempAlgo = 8;

			if ( dfRezTI )
			{
				if (( dfStatus2.dfRezLockedTI ) || !( gFlags.new_rez_ti ))
				{
					dfResistance = dfRezTI;
					RezMillis = ( dfMillis & 0xf0 ) >> 4;
				}
			}
			break;

		case 2:  // Temp SS
			if ( dfTCRP[dfMode] == 0 ) dfTempAlgo = 3;
			else dfTempAlgo = 9;

			if ( dfRezSS )
			{
				if (( dfStatus2.dfRezLockedSS ) || !( gFlags.new_rez_ss ))
				{
					dfResistance = dfRezSS;
					RezMillis = ( dfMillis & 0xf00 ) >> 8;
				}
			}
			break;

		case 3:  //TCR
                        //dfTCRM[dfTCRIndex]
                    
			dfTempAlgo = 4 + dfTCRIndex; //4 5 6
                        
			if ( dfRezTCR )
			{
				if (( dfStatus2.dfRezLockedTCR ) || !( gFlags.new_rez_tcr ))
				{
					dfResistance = dfRezTCR;
					RezMillis = ( dfMillis & 0xf000 ) >> 12;
				}
			}
			break;

		default: //VW
                    
                        if ( 
                               ( dfVWTempAlgo == 1 && dfTCRP[0] )
                            || ( dfVWTempAlgo == 2 && dfTCRP[1] )
                            || ( dfVWTempAlgo == 3 && dfTCRP[2] ) )
                        {
                                dfVWTempAlgo += 6; //7 8 9
                        }
                        else if ( 
                               ( dfVWTempAlgo == 7 && !dfTCRP[0] )
                            || ( dfVWTempAlgo == 8 && !dfTCRP[1] )
                            || ( dfVWTempAlgo == 9 && !dfTCRP[2] ) )
                        {
                                dfVWTempAlgo -= 6; //1 2 3
                        }
                        
                        if ( dfVWTempAlgo && dfVWTempAlgo < 10 )
                            dfTempAlgo = dfVWTempAlgo;
                        else
                            dfTempAlgo = 3;
                        
                    	if ( dfRezVW ) //not in DF for now
			{
				if ( dfStatus2.vwrezlock )
				{
					dfResistance = dfRezVW;
					RezMillis = 0; //lazy todo
				}
			}
                                        
                    	//dfResistance = AtoRez;
                        //RezMillis = AtoMillis;
			break;
	}

	UpdateDFTimer = 50;
}


//=========================================================================
//----- (00005AF0) --------------------------------------------------------
__myevic__ void NextMode()
{
	do
	{
		if ( dfMode < 3 )
		{
			dfMode = 3;
		}
		else if ( dfMode < 6 )
		{
			++dfMode;
		}
		else
		{
			dfMode = dfTCMode;

			if ( dfModesSel & ( 1 << dfMode ) )
			{
				if (( dfModesSel & 0x07 ) != 0x07 )
				{
					do
					{
						if ( ++dfMode > 2 ) dfMode = 0;
					}
					while ( dfModesSel & ( 1 << dfMode ) );
				}
			}
		}
	}
	while ( dfModesSel & ( 1 << dfMode ) );

	ModeChange();
}


//=========================================================================
// Median filter
//-------------------------------------------------------------------------

__myevic__ void InitFilter( filter_t *filter )
{
	filter->count = 0;
	filter->oldest = 0;
}

__myevic__ uint16_t FilterMedian( filter_t *filter, uint16_t data )
{
	filter->data[filter->oldest] = data;
	filter->oldest = ( filter->oldest + 1 ) % FILTER_SIZE;

	if ( filter->count < FILTER_SIZE )
	{
		++filter->count;
		return data;
	}

	filter->order[0] = 0;

	for ( int i = 0 ; i <= FILTER_SIZE / 2 ; ++i )
	{
		for ( int j = i + 1 ; j < FILTER_SIZE ; ++j )
		{
			if ( i == 0 ) filter->order[j] = j;

			if ( filter->data[filter->order[i]] > filter->data[filter->order[j]] )
			{
				uint8_t k = filter->order[i];
				filter->order[i] = filter->order[j];
				filter->order[j] = k;
			}
		}
	}
	
	return filter->data[filter->order[FILTER_SIZE / 2]];
}

/* not used yet
__myevic__ uint16_t FilterTriangle( filter_t *filter, uint16_t data )
{
	const char coefs[FILTER_SIZE] = { 1, 2, 3, 2, 1 };

	filter->data[filter->oldest] = data;
	filter->oldest = ( filter->oldest + 1 ) % FILTER_SIZE;

	if ( filter->count < FILTER_SIZE )
	{
		++filter->count;
		return data;
	}

	data = 0;

	int i,j;
	for ( i = 0, j = filter->oldest ; i < FILTER_SIZE ; ++i )
	{
		data += coefs[i] * filter->data[j];
		if ( ++j >= FILTER_SIZE ) j = 0;
	}

	return ( data / 9 );
}
*/

__myevic__ uint16_t FilterWMean( filter_t *filter, uint16_t data )
{
	filter->data[filter->oldest] = data;
	filter->oldest = ( filter->oldest + 1 ) % FILTER_SIZE;

	if ( filter->count < FILTER_SIZE )
	{
		++filter->count;
		return data;
	}

	data = 0;

	for ( int i = 0 ; i < filter->count ; ++i )
	{
		data += filter->data[i];
	}

	return ( data / filter->count );
}


//=========================================================================
// Bhaskara I's approximation of sine
// returns sin(x) in 16.16 fixed point format
__myevic__ int32_t BhaskaraI( int32_t x )
{
	int32_t a = x * ( 180 - x );
	int32_t b = 40500 - a;
	return ( ( a << 18 ) / b );
}


//=========================================================================
// sine approx given for x in degrees
// result in 16.16 fixed point format
__myevic__ int32_t sine( int32_t x )
{
	if ( x < 0 )
	{
		x = 180 - x;
	}

	x %= 360;

	if ( x < 180 )
	{
		return BhaskaraI( x );
	}
	else
	{
		return ( - BhaskaraI( x - 180 ) );
	}
}


//=========================================================================
// cosine approx given for x in degrees
// result in 16.16 fixed point format
__myevic__ int32_t cosine( int32_t x )
{
	return sine( x + 90 );
}

//=========================================================================
// scale of coordinate
__myevic__ int map( int x, int in_min, int in_max, int out_min, int out_max )
{
	return ( x - in_min ) * ( out_max - out_min )
                / ( in_max - in_min ) + out_min;
}


//=========================================================================

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} angles_t;

typedef struct {
	int8_t x;
	int8_t y;
	int8_t z;
} pt3d_t;

typedef struct {
	int8_t pt1;
	int8_t pt2;
} line3d_t;

typedef struct {
	const uint8_t	scale;
	const uint8_t	scalediv;
	const uint8_t	npoints;
	const uint8_t	nlines;
	const pt3d_t	*points;
	const line3d_t	*lines;
} obj3d_t;

const pt3d_t cube_points[] = 
	{
		{  1,  1,  1 }, // 0  000
		{ -1,  1,  1 }, // 1  X00
		{  1, -1,  1 }, // 2  0Y0
		{ -1, -1,  1 }, // 3  XY0
		{  1,  1, -1 }, // 4  00Z
		{ -1,  1, -1 }, // 5  X0Z
		{  1, -1, -1 }, // 6  0YZ
		{ -1, -1, -1 }  // 7  XYZ
	};

const line3d_t cube_lines[] = 
	{
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 4 },
		{ 1, 3 },
		{ 1, 5 },
		{ 2, 3 },
		{ 2, 6 },
		{ 3, 7 },
		{ 4, 5 },
		{ 4, 6 },
		{ 5, 7 },
		{ 6, 7 }
	};
	
const obj3d_t cube =
{
	16, 1,
	8,
	12,
	cube_points,
	cube_lines
};

const pt3d_t quartz_points[] = 
	{
		{  0,  0,  2 }, 
		{  0,  1,  1 }, 
		{  1,  0,  1 }, 
		{  0, -1,  1 }, 
		{ -1,  0,  1 }, 
		{  0,  1,  0 }, 
		{  1,  0,  0 }, 
                {  0, -1,  0 }, 
                { -1,  0,  0 }, 
		{  0,  0, -1 }  
	};

const line3d_t quartz_lines[] = 
	{
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 0, 4 },
		{ 1, 2 },
		{ 1, 4 },
		{ 1, 5 },
		{ 2, 3 },
		{ 2, 6 },
		{ 3, 4 },
		{ 3, 7 },
                { 4, 8 },
                { 5, 6 },
                { 5, 8 },
                { 5, 9 },
                { 6, 7 },
                { 6, 9 },
                { 7, 8 },
                { 7, 9 },
		{ 8, 9 }
	};
	
const obj3d_t quartz =
{
	15, 1,
	10,
	20,
	quartz_points,
	quartz_lines
};

const pt3d_t spinner_points[] = 
	{
{0, 0, 25},
{0, 0, -25},
{0, 42, 0},
{-36, -22, 0},
{36, -22, 0},
{-105, -64, 0},
{-95, -8, 0},
{-52, -82, 0},
{0, 123, 0},
{-42, 88, 0},
{42, 88, 0},
{105, -64, 0},
{95, -8, 0},
{52, -82, 0},
{0, 88, 25},
{0, 88, -25},
{-72, -44, 25},
{-72, -44, -25},
{72, -44, 25},
{72, -44, -25}
	};

const line3d_t spinner_lines[] = 
	{
{2, 9},
{9, 8},
{8, 10},
{10, 2},
{6, 5},
{5, 7},
{13, 11},
{11, 12},
{2, 3},
{3, 4},
{4, 2},
{6, 3},
{3, 7},
{4, 13},
{4, 12},
{0, 3},
{0, 4},
{0, 2},
{2, 1},
{4, 1},
{3, 1},
{16, 6},
{16, 3},
{16, 5},
{16, 7},
{3, 17},
{7, 17},
{5, 17},
{17, 6},
{14, 2},
{14, 10},
{14, 8},
{14, 9},
{15, 8},
{15, 9},
{15, 10},
{15, 2},
{4, 18},
{13, 18},
{18, 11},
{18, 12},
{4, 19},
{12, 19},
{13, 19},
{11, 19},
	};
	
const obj3d_t spinner =
{
	8, 33,
	20,
	45,
	spinner_points,
	spinner_lines
};

const pt3d_t tie_points[] = 
	{
{0, 0, 15},
{0, 0, -15},
{-15, 0, 0},
{-27, 25, 15},
{-27, 25, -15},
{27, -25, -15},
{27, 0, -19},
{27, -25, 15},
{27, 0, 19},
{27, 25, -15},
{27, 25, 15},
{-27, 0, 0},
{0, 15, 0},
{-27, 0, -19},
{-27, -25, 15},
{-27, -25, -15},
{15, 0, 0},
{-27, 0, 19},
{0, -15, 0},
{27, 0, 0}
	};

const line3d_t tie_lines[] = 
	{
{14, 17},
{17, 3},
{3, 4},
{4, 13},
{13, 15},
{15, 14},
{2, 11},
{10, 9},
{8, 10},
{8, 7},
{7, 5},
{5, 6},
{6, 9},
{2, 1},
{2, 0},
{0, 16},
{16, 1},
{0, 12},
{12, 1},
{0, 18},
{18, 1},
{12, 2},
{12, 16},
{16, 18},
{2, 18},
{4, 14},
{3, 15},
{7, 9},
{5, 10},
{19, 16},
{11, 2},
{2, 12}
	};
	
const obj3d_t tie =
{
	8, 11,
	20,
	32,
	tie_points,
	tie_lines
};

const pt3d_t tetra_points[] = 
	{
		{  1,  1,  1 }, // 0
		{  1, -1, -1 }, // 1
		{ -1,  1, -1 }, // 2
		{ -1, -1,  1 }  // 3
	};

const line3d_t tetra_lines[] = 
	{
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 1, 2 },
		{ 1, 3 },
		{ 2, 3 }
	};
	
const obj3d_t tetra =
{
	16, 1,
	4,
	6,
	tetra_points,
	tetra_lines
};

const pt3d_t square_points[] = 
	{
		{  1,  1,  1 }, // 0
		{  1,  1, -1 }, // 1
		{ -1, -1,  1 }, // 2
		{ -1, -1, -1 }  // 3
	};

const line3d_t square_lines[] = 
	{
		{ 0, 1 },
		{ 0, 2 },
		{ 1, 3 },
		{ 2, 3 }
	};
	
const obj3d_t square =
{
	16, 1,
	4,
	4,
	square_points,
	square_lines
};

const pt3d_t octa_points[] = 
	{
		{  0,  1,  0 }, // 0
		{  1,  0,  0 }, // 1
		{  0,  0,  1 }, // 2
		{ -1,  0,  0 }, // 3
		{  0,  0, -1 }, // 4
		{  0, -1,  0 }  // 5
	};

const line3d_t octa_lines[] = 
	{
		{ 0, 1 },
		{ 0, 2 },
		{ 0, 3 },
		{ 0, 4 },
		{ 5, 1 },
		{ 5, 2 },
		{ 5, 3 },
		{ 5, 4 },
		{ 1, 2 },
		{ 2, 3 },
		{ 3, 4 },
		{ 4, 1 }
	};
	
const obj3d_t octa =
{
	28, 1,
	6,
	12,
	octa_points,
	octa_lines
};

/*
const pt3d_t dodeca_points[] = 
	{
		{  50,  50,  50 }, //  0
		{  50,  50, -50 }, //  1
		{  50, -50,  50 }, //  2
		{  50, -50, -50 }, //  3
		{ -50,  50,  50 }, //  4
		{ -50,  50, -50 }, //  5
		{ -50, -50,  50 }, //  6
		{ -50, -50, -50 }, //  7
		{   0,  31,  81 }, //  8
		{   0,  31, -81 }, //  9
		{   0, -31,  81 }, // 10
		{   0, -31, -81 }, // 11
		{  81,   0,  31 }, // 12
		{  81,   0, -31 }, // 13
		{ -81,   0,  31 }, // 14
		{ -81,   0, -31 }, // 15
		{  31,  81,   0 }, // 16
		{  31, -81,   0 }, // 17
		{ -31,  81,   0 }, // 18
		{ -31, -81,   0 }  // 19
	};

const line3d_t dodeca_lines[] = 
	{
		{  0,  8 },
		{  0, 12 },
		{  0, 16 },
		{  1,  9 },
		{  1, 13 },
		{  1, 16 },
		{  2, 10 },
		{  2, 12 },
		{  2, 17 },
		{  3, 11 },
		{  3, 13 },
		{  3, 17 },
		{  4,  8 },
		{  4, 14 },
		{  4, 18 },
		{  5,  9 },
		{  5, 15 },
		{  5, 18 },
		{  6, 10 },
		{  6, 14 },
		{  6, 19 },
		{  7, 11 },
		{  7, 15 },
		{  7, 19 },
		{  8, 10 },
		{  9, 11 },
		{ 12, 13 },
		{ 14, 15 },
		{ 16, 18 },
		{ 17, 19 }
	};

const obj3d_t dodeca =
{
	16, 50,
	20,
	30,
	dodeca_points,
	dodeca_lines
};

const pt3d_t isoca_points[] = 
	{
		{   0,  10,  16 }, //  0
		{   0, -10,  16 }, //  1
		{   0,  10, -16 }, //  2
		{   0, -10, -16 }, //  3
		{  10,  16,   0 }, //  4
		{ -10,  16,   0 }, //  5
		{  10, -16,   0 }, //  6
		{ -10, -16,   0 }, //  7
		{  16,   0,  10 }, //  8
		{ -16,   0,  10 }, //  9
		{  16,   0, -10 }, // 10
		{ -16,   0, -10 }  // 11
	};

const line3d_t isoca_lines[] = 
	{
		{  0,  1 },
		{  0,  4 },
		{  0,  5 },
		{  0,  8 },
		{  0,  9 },
		{  1,  6 },
		{  1,  7 },
		{  1,  8 },
		{  1,  9 },
		{  2,  3 },
		{  2,  4 },
		{  2,  5 },
		{  2, 10 },
		{  2, 11 },
		{  3,  6 },
		{  3,  7 },
		{  3, 10 },
		{  3, 11 },
		{  4,  5 },
		{  4,  8 },
		{  4, 10 },
		{  5,  9 },
		{  5, 11 },
		{  6,  7 },
		{  6,  8 },
		{  6, 10 },
		{  7,  9 },
		{  7, 11 },
		{  8, 10 },
		{  9, 11 }
	};

const obj3d_t isoca =
{
	16, 10,
	12,
	30,
	isoca_points,
	isoca_lines
};
*/

const obj3d_t *objects3d[] =
	{
                &square,
		&tetra,
		&cube,
		&octa,
		//&dodeca,
		//&isoca,
                &tie,   
                &quartz,
                &spinner        
	};

#define N3DOBJECTS (sizeof(objects3d)/sizeof(obj3d_t*))

typedef int32_t matrix3d_t[3][3];


static angles_t angles = { 0, 0, 0 };
static angles_t speeds = { 3, 2, 1 }; //{ 3, 5, 1 };
static angles_t speeds_spin = { 1, 0, 10 }; //{ 3, 5, 1 };
static matrix3d_t rot_matrix;
static pt3d_t points[20]; //biggest one

//uint8_t Object3D = 0;

//	  SSAVER_NONE = 0,
//     SSAVER_3D,
//	SSAVER_QIX,     //
//	SSAVER_SNOW,
//      SSAVER_SF,              
//	SSAVER_CLOCK,   
//	SSAVER_LOGO,
//	SSAVER_SPLASH,  // rotate too with +-
//	  SSAVER_MAX
                
__myevic__ void NextSSaver()
{
    if ( dfScreenSaver == SSAVER_3D )
    {
        if ( ++Object3D > N3DOBJECTS ) 
        {
            dfScreenSaver = SSAVER_QIX;
            Object3D = 1;
        }
    }
    else
    {
	if ( ++dfScreenSaver == SSAVER_MAX ) 
        {
            dfScreenSaver = SSAVER_3D;   
            Object3D = 1;
        }
    }
    
    if ( !ISANIMSAVER(dfScreenSaver) ) SetScreen( 60, ScreenDuration );
}


__myevic__ void PreviousSSaver()
{
    if ( dfScreenSaver == SSAVER_3D )
    {
        if ( --Object3D < 1 ) 
        {
            dfScreenSaver = SSAVER_SPLASH;
            Object3D = N3DOBJECTS;
        }
    }
    else
    {
        --dfScreenSaver;
        Object3D = N3DOBJECTS;
    }
    
    if ( !ISANIMSAVER(dfScreenSaver) ) SetScreen( 60, ScreenDuration );
}

/*
__myevic__ void Next3DObject()
{
        //if ( Screen == 60 ) we here from if Screen == 60 
        //ClearScreenBuffer(); move once in call //DrawFillRect( 0, 0, 63, 127, 0 );
	if ( ++Object3D > N3DOBJECTS ) Object3D = 1;
}


__myevic__ void Previous3DObject()
{
        //if ( Screen == 60 ) 
        //ClearScreenBuffer(); //DrawFillRect( 0, 0, 63, 127, 0 );
	if ( --Object3D < 1 ) Object3D = N3DOBJECTS;
}

__myevic__ void NextSSaver()
{
	if ( ++dfScreenSaver == SSAVER_MAX ) dfScreenSaver = 2; //SSAVER_CLOCK,...
        
        //if ( !ISANIMSAVER(dfScreenSaver) ) 
            SetScreen( 60, ScreenDuration );
}


__myevic__ void PreviousSSaver()
{
	if ( --dfScreenSaver < 2 ) dfScreenSaver = SSAVER_MAX - 1; //SSAVER_SF,
        
        //if ( !ISANIMSAVER(dfScreenSaver) ) 
            SetScreen( 60, ScreenDuration );
}
*/

__myevic__ void compute_matrix( matrix3d_t mat, const angles_t *a )
{
	int32_t A = cosine( a->x );
	int32_t B = sine( a->x );
	int32_t C = cosine( a->y );
	int32_t D = sine( a->y );
	int32_t E = cosine( a->z );
	int32_t F = sine( a->z );

	int32_t AD = ( A >> 8 ) * ( D >> 8 );
	int32_t BD = ( B >> 8 ) * ( D >> 8 );

	mat[0][0] = (  C >> 8 ) * ( E >> 8 );
	mat[0][1] = ( -C >> 8 ) * ( F >> 8 );
	mat[0][2] =  D;
	mat[1][0] = (  BD >> 8 ) * ( E >> 8 ) + ( A >> 8 ) * ( F >> 8 );
	mat[1][1] = ( -BD >> 8 ) * ( F >> 8 ) + ( A >> 8 ) * ( E >> 8 );
	mat[1][2] = (  -B >> 8 ) * ( C >> 8 );
	mat[2][0] = ( -AD >> 8 ) * ( E >> 8 ) + ( B >> 8 ) * ( F >> 8 );
	mat[2][1] = (  AD >> 8 ) * ( F >> 8 ) + ( B >> 8 ) * ( E >> 8 );
	mat[2][2] = (   A >> 8 ) * ( C >> 8 );
}


__myevic__ void rotate_object( pt3d_t *dst, const obj3d_t *src, const matrix3d_t mat )
{
	int32_t s, d, x, y, z;

        if ( Screen == 60 )
        {
            s = 3 * src->scale / 2;
        } else {
            s = src->scale;
        }
        
	d = src->scalediv;

	for ( int i = 0 ; i < src->npoints ; ++i )
	{
		x = s * src->points[i].x / d;
		y = s * src->points[i].y / d;
		z = s * src->points[i].z / d;

		dst[i].x = ( mat[0][0] * x + mat[0][1] * y + mat[0][2] * z ) >> 16;
		dst[i].y = ( mat[1][0] * x + mat[1][1] * y + mat[1][2] * z ) >> 16;
		dst[i].z = ( mat[2][0] * x + mat[2][1] * y + mat[2][2] * z ) >> 16;
	}
}


__myevic__ void next_angle()
{
	angles.x += speeds.x; angles.x %= 360;
	angles.y += speeds.y; angles.y %= 360;
	angles.z += speeds.z; angles.z %= 360;
}
__myevic__ void next_angle_spin()
{
	angles.x += speeds_spin.x; angles.x %= 360;
	angles.y = 0;
	angles.z += speeds_spin.z; angles.z %= 360;
}

__myevic__ void anim3d() //( int redraw_last )
{
	static int tscaler = 0;
	const obj3d_t *object;
        int cY = 79;
        int thk = 1;
        
	if (( Object3D > 0 ) && ( Object3D <= N3DOBJECTS ))
	{
		object = objects3d[Object3D-1];
	}
	else
	{
		return;
	}

	//if ( !redraw_last )
	//{
            
                rotate_object( points, object, rot_matrix );
               
                if ( ++tscaler < 4 ) return; //4 speed
		tscaler = 0;
                      
                if ( Object3D == 7 ) next_angle_spin();
                else next_angle();

		compute_matrix( rot_matrix, &angles );
                
		int f = 256;
		for ( int i = 0 ; i < object->npoints ; ++i )
		{
			points[i].x = f * points[i].x / ( f + points[i].z );
			points[i].y = f * points[i].y / ( f + points[i].z );
		}

        if ( Screen == 60 )
        {
            cY = 64;
            thk = 2;
            ClearScreenBuffer(); //DrawFillRect( 0, 0, 63, 127, 0 );
        } 
        else
        {  
            int h = GetLogoHeight();
            int yh = 42;
            if ( h && !dfStatus.nologo && !dfStatus.logomid )
            {
                if ( h > 40 ) yh = 48;
            }
            DrawFillRect( 0, yh, 63, 112, 0 ); //clear space
        }
        
	for ( int i = 0 ; i < object->nlines ; ++i )
	{
		DrawLine(
			points[object->lines[i].pt1].x + 32,
			points[object->lines[i].pt1].y + cY,
			points[object->lines[i].pt2].x + 32,
			points[object->lines[i].pt2].y + cY,
			1,
			thk
		);
	}
         
	//if ( !redraw_last )
	//{
		DisplayRefresh();
	//}
}

//=========================================================================
// Pseudo-random number generator

uint32_t RNGSeed;

//----- (000001CC) --------------------------------------------------------
__myevic__ uint32_t Random()
{
  RNGSeed = 0x41c64e6d * RNGSeed + 12345;
  return RNGSeed >> 1;
}

//----- (000001E0) --------------------------------------------------------
__myevic__ void SetRandSeed( uint32_t s )
{
  RNGSeed = s;
}

//=========================================================================
// Qix
// derived from bounce.c Copyright (c) 1987 Bellcore
//

#define QIX_MAXX    63
#define QIX_MAXY    127
#define QIX_MINV    2
#define QIX_MAXV    10
#define QIX_LCT     12

typedef struct {
        int16_t x1, y1, x2, y2;
} qix_t;

static qix_t	qix_curr;
static qix_t	qix_move;
static qix_t	qix_prev[QIX_LCT];
static int16_t ptr;


__myevic__ void qix() // int refresh 
{
	static int tscaler = 0;

/*
	if ( refresh )
	{
		SetRandSeed( TMR1Counter );
		qix_move.x1 = qix_move.y1 = 5;
		qix_move.x2 = qix_move.y2 = -5;
		qix_curr.x1 = qix_curr.x2 = QIX_MAXX/2;
		qix_curr.y1 = qix_curr.y2 = QIX_MAXY/2;
		//qix_curr.y2 = QIX_MAXY/2+20;
		for ( ptr=0; ptr<QIX_LCT; ptr++ ) {
			qix_prev[ptr].x1 = qix_prev[ptr].y1 = qix_prev[ptr].x2 = qix_prev[ptr].y2 = -1;
		}
 	}
*/

	if ( ++tscaler < 4 ) return;
	tscaler = 0;

	ptr = (ptr + 1) % QIX_LCT;
	if (qix_prev[ptr].x1 >= 0) {
		DrawLine(qix_prev[ptr].x1, qix_prev[ptr].y1, qix_prev[ptr].x2, qix_prev[ptr].y2, 0, 1);
	}

	qix_mvpoint(&qix_curr.x1, &qix_curr.y1, &qix_move.x1, &qix_move.y1);
	qix_mvpoint(&qix_curr.x2, &qix_curr.y2, &qix_move.x2, &qix_move.y2);

	qix_prev[ptr].x1 = qix_curr.x1;
	qix_prev[ptr].y1 = qix_curr.y1;
	qix_prev[ptr].x2 = qix_curr.x2;
	qix_prev[ptr].y2 = qix_curr.y2;

	if (qix_prev[ptr].x1 >= 0) {
		DrawLine(qix_prev[ptr].x1, qix_prev[ptr].y1, qix_prev[ptr].x2, qix_prev[ptr].y2, 1, 1);
	}
        
	//if ( !refresh )
	//{
		DisplayRefresh();
	//}
}

__myevic__ void qix_mvpoint(int16_t *tx, int16_t *ty, int16_t *v_x, int16_t *v_y)
{
	*tx += *v_x; // move the point
	*ty += *v_y;

	if ( *tx >= QIX_MAXX)  // bounce
	{
		*v_x = (*v_x > 0) ? -(*v_x) : *v_x;
		qix_diddle(v_x);
	}
	if ( *ty >= QIX_MAXY)
	{
		*v_y = (*v_y > 0) ? -(*v_y) : *v_y;
		qix_diddle(v_y);
	}
	if ( *tx <= 0)
	{
		*v_x = (*v_x < 0) ? -(*v_x) : *v_x;
		qix_diddle(v_x);
	}
	if ( *ty <= 0)
	{
		*v_y = (*v_y < 0) ? -(*v_y) : *v_y;
		qix_diddle(v_y);
	}
}

__myevic__ void qix_diddle(int16_t *ptr)
{
	int16_t tmp;
	/* pick a number between QIX_MAXV and QIX_MINV */
	tmp = (int16_t) (Random() %(QIX_MAXV - QIX_MINV)) + QIX_MINV;
	/* get the sign right */
	if (*ptr < 0) *ptr = -tmp; else *ptr = tmp;
}

//=========================================================================
// Snow (screen burn-out recovery)
//-------------------------------------------------------------------------

__myevic__ void Snow() // int redraw 
{
	static int tscaler = 0;

	// Lets's say 50Hz
	if ( ++tscaler < 2 ) return;
	tscaler = 0;

	for ( int i = 0; i < SCREEN_BUFFER_SIZE ; ++i )
	{
		ScreenBuffer[i] = Random() % 0xFF;
	}
	//if ( !redraw )
	//{
		DisplayRefresh();
	//}
}

typedef struct {
        int x, y, z;
} stars_t;

__myevic__ void StarField()
{
    //Daniel Shiffman 
    //https://github.com/CodingTrain/website/tree/master/CodingChallenges/CC_01_StarField_p5.js
    
    static stars_t stars[50];
    int sx, sy, r;

    static int tscaler = 0;
    if ( ++tscaler < 5 ) return;
    tscaler = 0;
        
    //SetRandSeed( TMR1Counter );
    
    ClearScreenBuffer();
    
    for ( int i = 0; i < 50; i++ ) 
    {            
        stars[i].z -= 1;
        
        if ( stars[i].z < 1 ) 
        {
            stars[i].z = Random() % 32 + 32;
            stars[i].x = Random() % 64 - 32;
            stars[i].y = Random() % 128 - 64;
        }
    
        sx = map( (stars[i].x * 16) / stars[i].z, -32, 32, 0, 63 );
        sy = map( (stars[i].y * 16) / stars[i].z, -64, 64, 0, 127 );
        r = map( stars[i].z, 32, 0, 0, 6 );
    
        // DrawCircle( int x_centre, int y_centre, int r, int color, int fill )
        DrawCircle( sx, sy, r, 1, !r );
    }
 
    DisplayRefresh();

}

//=========================================================================
// LED Stuff
//-------------------------------------------------------------------------

uint8_t LEDRed;
uint8_t LEDGreen;
uint8_t LEDBlue;

volatile uint8_t LEDTimer;

__myevic__ void LEDGetColor()
{
	LEDBlue = dfLEDColor & 0x1F; // 0000000000011111 dfLEDColor
	LEDGreen = ( dfLEDColor & ( 0x1F << 5 ) ) >> 5; //000001111100000
	LEDRed = ( dfLEDColor & ( 0x1F << 10 ) ) >> 10; //111110000000000
}

__myevic__ void LEDSetColor()
{
	dfLEDColor = ( LEDRed << 10 ) | ( LEDGreen << 5 ) | LEDBlue;
        if ( ISEGRIPII || ISEVICAIO || ISSINFJ200 || ISSINP80 ) 
        {
            gFlags.led_on = 1;
            LEDTimer = 20;
        }
}

//__myevic__ void LEDWhite()
//{
//	LEDBlue = 25;
//	LEDGreen = 25;
//	LEDRed = 25;
//}

__myevic__ void LEDOff()
{
    gFlags.led_on = 0;

    if ( ISSINFJ200 )
    {
        PB7 = 0;
    }
    else if ( ISEGRIPII || ISEVICAIO )
    {
	PB->DOUT &= ~(7<<3); //~0x38; //low
    }
    else if ( ISSINP80 )
    {
	PD->DOUT &= ~3;
    }
}

// LED PWM
// Called by Timer 1 @ 5kHz
__myevic__ void LEDControl()
{
    if ( !gFlags.led_on ) 
    {
        LEDOff();
        return;
    }
        
    uint32_t phase = TMR1Counter % 25;
    
    if ( ISSINFJ200 )
    {
        //GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT ); 
        PB7 = ( phase < LEDGreen ); //1;
    }
    else if ( ISEGRIPII || ISEVICAIO )
    {	
	PB3 = ( phase < LEDBlue );
	PB4 = ( phase < LEDRed );
	PB5 = ( phase < LEDGreen );
    }
    else if ( ISSINP80 )
    { 
        // LEDBlue = Yellow
        // green 0 1; red 1 0;
        
        if ( LEDRed ) //R
        {
            PD0 = 1; PD1 = 0;
        }
        else if ( LEDGreen ) //G
        {
            PD0 = 0; PD1 = 1;
        }
        else if ( LEDBlue ) //Y
        {
            PD->DOUT |= 3; //PD0 = 1; PD1 = 1;
        }
        else //off
        {
            PD->DOUT &= ~3;
        }               
    }
    // no else
}

__myevic__ void LEDTimerTick()
{
    //10Hz
    
	if ( LEDTimer )
	{                   
		if ( !--LEDTimer )
		{
			LEDOff();
		}
	}
}

