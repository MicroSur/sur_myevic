//#include "vcom.h"
#include "myevic.h"

//=========================================================================
// VCOM
//-------------------------------------------------------------------------

//#define RXBUFSIZE	512 /* RX buffer size */
volatile uint8_t comRbuf[512];
//volatile uint16_t comRbytes = 0;
//volatile uint16_t comRtail = 0;

//-------------------------------------------------------------------------
// need for USB in games but WHY? ///
__myevic__ void NeedHelp() //VCOM_Cout
{
	comRbuf[0] = 0;
}
