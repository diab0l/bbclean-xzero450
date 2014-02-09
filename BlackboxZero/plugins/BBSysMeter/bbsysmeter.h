/*         
 ============================================================================
 Blackbox for Windows: Plugin BBSysMeter 1.0 by Miroslav Petrasko [Theo] 
 ============================================================================
 Copyright (c) 2001-2004 The Blackbox for Windows Development Team
 http://desktopian.org/bb/ - #bb4win on irc.freenode.net
 ============================================================================
  Blackbox for Windows is free software, released under the
  GNU General Public License (GPL version 2 or later), with an extension
  that allows linking of proprietary modules under a controlled interface.
  
  What this means is that plugins etc. are allowed to be released
  under any license the author wishes. Please note, however, that the
  original Blackbox gradient math code used in Blackbox for Windows
  is available under the BSD license.
  
  http://www.fsf.org/licenses/gpl.html
  http://www.fsf.org/licenses/gpl-faq.html#LinkingOverControlledInterface
  http://www.xfree86.org/3.3.6/COPYRIGHT2.html#5
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  For additional license information, please read the included license
 ============================================================================
*/



#ifndef __BBSYSMETER_H
#define __BBSYSMETER_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

//temporary until upgrade to .9x BBAPI
#ifndef SLIT_ADD
#define SLIT_ADD 11001
#endif

#ifndef SLIT_REMOVE
#define SLIT_REMOVE 11002
#endif
//end of temp fix

//#ifndef ULONG_PTR
//#define ULONG_PTR DWORD
//#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#endif // ndef WS_EX_LAYERED


#include <windows.h>
#include <winuser.h>
#include <math.h>
#include <time.h>
//#include "gdi/gdiplus.h"
#include <shlwapi.h>
#include "BBApi.h"
#include "MessageBox.h"
#include "getStats.h"
#include <gdiplus.h>
#include <Iphlpapi.h>

//#include "StdAfx.h"

#define IDT_TIMER 1

#define ELLIPSE 0
#define HAND 1
#define CHART 2
#define RECTLR 3
#define RECTBT 4

#define TOP 0
#define BOTTOM 1
#define LEFT 2
#define RIGHT 3

#define CPU 0
#define RAM 1
#define SWAP 2
#define HDD 3
#define NETT 4
#define NETIN 5
#define NETOUT 6
#define GDI 7


#define COMPUTER_NAME 15
#define	USER_NAME 16
#define WORK_AREA 17
#define SCREEN_SIZE 18
#define OS_VERSION 19
#define ADAPTER_DESCRIPTION 20

/*#define NET_MASK 16
#define IP_ADDRESS 17
#define GATEWAY_ADDRESS 18
#define HOST_NAME 19
#define DOMAIN_NAME 20
#define DNS_SERVER 21
*/
//typedef unsigned __int64 ULONG_PTR;
//typedef DWORD ULONG_PTR;
//typedef float REAL;
//typedef unsigned __int64* ULONG_PTR;

//===========================================================================

//gdiplus things
using namespace Gdiplus;
GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;

//OS info storage
DWORD      dwId;
DWORD      dwMajorVer;
DWORD      dwMinorVer;

//for circle calculations
double	PI = 3.14159265359;
double	theta;

RECT rect;
MIB_IFTABLE* c_Table = NULL;
int c_NumOfTables = 0;

//temp storage
static char szTemp[MAX_LINE_LENGTH];
static char sz[MAX_LINE_LENGTH];
static char command[MAX_LINE_LENGTH];



//window instances
HINSTANCE hInstance;
HWND hwndBBSysMeter, hwndBlackbox;
bool inSlit = false;	//Are we loaded in the slit? (default of no)
HWND hSlit;				//The Window Handle to the Slit (for if we are loaded)

// Blackbox messages we want to "subscribe" to:
// BB_RECONFIGURE -> Sent when changing style and on reconfigure
// BB_BROADCAST -> Broadcast message (bro@m)
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

//file path storage
char rcpath[MAX_PATH];
char stylepath[MAX_PATH];
char mpath[MAX_PATH];
char dllname[MAX_PATH];

//start up positioning
int ScreenWidth;
int ScreenHeight;

//LOGFONT lfont;
//HFONT hfont;
//border drawing


//font setting
int fontSizeC;

//.rc file settings
int xpos, ypos;
int width, height;

bool wantInSlit;
bool alwaysOnTop;
bool snapWindow;
bool pluginToggle;
bool transparency;
bool lockp;		//lock position
bool fullTrans;	//full transparent background
bool drawBorder;
int alpha;

bool noBitmap;
bool saveMessages;

int dotRadius;
int handLength;
bool fill;

int mon[300];
int c_cpu = 0;
int text_pos;
int text_width = 1;
char drive_letter[1];
char windowStyle[24];
char bitmapFile[MAX_PATH];

char new_line[]="\n";
//stats
ULARGE_INTEGER fba, tnob, tnofb;

enum opSys {OPSYS_WIN_9X,
            OPSYS_WIN_NT4,
            OPSYS_WIN_NT5};

CGetStats*  m_pStatsObj;


//enum opSys  m_eOpSys;

MEMORYSTATUS memstat;

UINT per_stat = 0;
UINT help_stat = 100;
UINT monitor = 0;
UINT draw_type = 0;

//style setting storage
COLORREF backColor, backColor2;
COLORREF backColorTo, backColorTo2;
COLORREF fontColor;
COLORREF drawColor;
char fontFace[256];
int fontSize = 13;
int temp = 0;
int refresh;
bool anti;
//bool labelIsPR;

StyleItem *myStyleItem, *myStyleItem2;
int bevelWidth;
int borderWidth;
COLORREF borderColor;

//menu items
Menu *myMenu, *windowStyleSubmenu, *configSubmenu, *settingsSubmenu, *drawSubmenu, *bitmapSubmenu, *styleSubmenu, *browseSubmenu;
Menu *monitorSubmenu, *refreshSubmenu, *generalConfigSubmenu, *messagesSubmenu, *textSubmenu, *otherSubmenu;

//===========================================================================

//gets OS version
int WINAPI _GetPlatformId(DWORD *pdwId, DWORD *pdwMajorVer, DWORD *pdwMinorVer);
//window process
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//special function for alpha transparency
//so there you just use BBSLWA like normal SLWA
//(c)grischka
BOOL WINAPI BBSetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

//Plugin functions
void GetStyleSettings();
void ReadRCSettings();
void WriteRCSettings();
void InitBBSysMeter();
void mySetTimer(int time);
void setStatus();
void createMenu();
void getStats(int stat);
void writeMessages(char message[MAX_LINE_LENGTH]);
LPCTSTR GetSysNfo(UINT id);
void NetTable();
DWORD GetNetOctets(UINT net);


//===========================================================================

extern "C"
{
	__declspec(dllexport) int beginPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) void endPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) LPCSTR pluginInfo(int field);
	// This is the function BBSlit uses to load your plugin into the Slit
	__declspec(dllexport) int beginSlitPlugin(HINSTANCE hMainInstance, HWND hBBSlit);
}

//===========================================================================

#endif

// the end ....
