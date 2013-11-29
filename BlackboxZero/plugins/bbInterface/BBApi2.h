/*===================================================

	BLACKBOX API AND MISC DEFINITIONS

===================================================*/

#ifndef __BBAPI2_H_
#define __BBAPI2_H_

#define WINVER 0x0500

#define WS_EX_LAYERED   0x00080000

#if (_MSC_VER >= 1400) 

// overload strcpy and the like to new "secure" functions 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1 
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1 
#pragma warning(disable:4996)
#endif


#include <BBApi.h>

// Utility
void dbg_printf(const char *fmt, ...);

// these are already in BBApi.h. keep it for a while and delete if safe
//#define BB_DESKCLICK 10884
//#define BB_DRAGTODESKTOP 10510
//#define VALID_TEXTCOLOR (1<<3)

// these are already in BBApi.h. keep it for a while and delete if safe
// wParam values for BB_WORKSPACE
//#define BBWS_DESKLEFT           0
//#define BBWS_DESKRIGHT          1
//#define BBWS_GATHERWINDOWS      5
//#define BBWS_MOVEWINDOWLEFT     6
//#define BBWS_MOVEWINDOWRIGHT    7
//#define BBWS_PREVWINDOW         8
//#define BBWS_NEXTWINDOW         9


// these are new, moved to BBApi.h
//#define BBI_MAX_LINE_LENGTH     4000
//#define BBI_POSTCOMMAND         (WM_USER+10)
//#define VALID_SHADOWCOLOR   (1<<13)
//-------------------------------------------------
#endif /* __BBAPI2_H_ */
