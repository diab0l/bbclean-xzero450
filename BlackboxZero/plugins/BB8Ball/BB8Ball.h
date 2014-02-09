/*
 ============================================================================
 Blackbox for Windows: Plugin BBAnalog by Brad Bartolucci made using SDK example
 ============================================================================
 Copyright (c) 2001-2002 The Blackbox for Windows Development Team
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

  For additional license information, please read the included license.html

  You will need the gdi+ sdk availible in the Windows core sdk at MS.

  Redistributable gdi+ runtime driver for all Windows OS's
  http://download.microsoft.com/download/platformsdk/redist/3097/W98NT42KMeXP/EN-US/gdiplus_dnld.exe

  Wav sounds can be found at http://www.a1freesoundeffects.com/radio.html

 ============================================================================
*/



#ifndef __BB8BALL_H
#define __BB8BALL_H

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

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#endif // ndef WS_EX_LAYERED


#include <windows.h>
#include <math.h>
//#include <time.h>
#include <shlwapi.h>
#include "BBApi.h"
#include "MessageBox.h"
//#include "AggressiveOptimize.h"


//#pragma comment(lib, "Blackbox.lib")
//#pragma comment(lib, "winmm.lib")
//#pragma comment(lib, "Shlwapi.lib")
//#pragma comment(lib, "MessageBox.lib")
//#pragma comment(lib, "comdlg32.lib")
//#pragma comment(lib, "msimg32.lib")


//typedef float REAL;

//===========================================================================

#define ADVICE_COUNT 20
char advice[ADVICE_COUNT][30] = {
	"Signs point to yes.",
	"Yes.",
	"Reply hazy, try again.",
	"Without a doubt.",
	"My sources say no.",
	"As I see it, yes.",
	"You may rely on it.",
	"Concentrate and ask again.",
	"Outlook not so good.",
	"It is decidedly so.",
	"Better not tell you now.",
	"Very doubtful.",
	"Yes - definitely.",
	"It is certain.",
	"Cannot predict now.",
	"Most likely.",
	"Ask again later.",
	"My reply is no.",
	"Outlook good.",
	"Don't count on it."
};



//OS info storage
DWORD      dwId;
DWORD      dwMajorVer;
DWORD      dwMinorVer;

RECT rect,r1;
//int fontsize;
//temp storage
static char szTemp[MAX_LINE_LENGTH];

//window instances
HINSTANCE hInstance;
HWND hwndBB8Ball, hwndBlackbox, hwndMessage;
bool inSlit = false;	//Are we loaded in the slit? (default of no)
HWND hSlit;				//The Window Handle to the Slit (for if we are loaded)

// Blackbox messages we want to "subscribe" to:
// BB_RECONFIGURE -> Sent when changing style and on reconfigure
// BB_BROADCAST -> Broadcast message (bro@m)
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

//file path storage
char rcpath[MAX_PATH];
char fpath[MAX_PATH];
char stylepath[MAX_PATH];
char resmsg[MAX_LINE_LENGTH];
//start up positioning
int ScreenWidth;
int ScreenHeight;




//.rc file settings
int xpos, ypos;
int mxpos,mypos;
int width, height;

bool wantInSlit;
bool alwaysOnTop;
bool snapWindow;
bool pluginToggle;
bool transparency;
bool fullTrans;
bool hide;
int alpha;

char windowStyle[24];
char dcaption[256];

//style setting storage
COLORREF backColor, backColor2;
COLORREF backColorTo, backColorTo2;

//bool labelIsPR;

StyleItem *myStyleItem, *myStyleItem2;

StyleItem *button, *buttonpr, *toolbar, *label, *winlabel, *clock;
COLORREF buttonColor, buttonColorTo;
COLORREF buttonprColor, buttonprColorTo;
COLORREF labelColor, labelColorTo;
COLORREF winlabelColor, winlabelColorTo;
COLORREF clockColor, clockColorTo;
COLORREF toolbarColor, toolbarColorTo;

COLORREF fontColor;

COLORREF buttonfontColor;
COLORREF buttonprfontColor;
COLORREF toolbarfontColor;
COLORREF labelfontColor;
COLORREF winlabelfontColor;
COLORREF clockfontColor;


int bevelWidth;
int borderWidth;
COLORREF borderColor;

//menu items
Menu *myMenu, *windowStyleSubmenu, *configSubmenu, *settingsSubmenu, *regionSubmenu, *colorSubmenu, *sizeSubmenu;// *placementSubmenu;
Menu *calendarConfigSubmenu, *alarmConfigSubmenu, *setAlarmsConfigSubmenu, *generalConfigSubmenu;

//===========================================================================

//gets OS version
int WINAPI _GetPlatformId(DWORD *pdwId, DWORD *pdwMajorVer, DWORD *pdwMinorVer);
//window process
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MesProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//special function for alpha transparency
//so there you just use BBSLWA like normal SLWA
//(c)grischka
BOOL WINAPI BBSetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

//Plugin functions
void GetStyleSettings();
void ReadRCSettings();
void WriteRCSettings();
void InitBB8Ball();
void setStatus();
void showResult();
void showFortune();
int CreateMessageBox();
void DestroyMessageBox();

RECT  okRect,omRect;
bool pressed = false;
bool ompressed = false;
bool fortunes;
bool apossition = true;
char fontFace[256];


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
