/*         
 ============================================================================
 Blackbox for Windows: Plugin BBMagnify 1.0 by Miroslav Petrasko [Theo] 
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



#ifndef __BBMAGNIFY_H
#define __BBMAGNIFY_H

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#endif 

#include <windows.h>
#include <shlwapi.h>
#include "BBApi.h"
#include "MessageBox.h"
//#include "AggressiveOptimize.h"

#define IDT_TIMER 1

//OS info storage
DWORD      dwId;
DWORD      dwMajorVer;
DWORD      dwMinorVer;


RECT rec,updateRect;

//temp storage
static char szTemp[MAX_LINE_LENGTH];

//window instances
HINSTANCE hInstance;
HWND hwndBBMagnify, hwndBlackbox;
bool inSlit = false;	//Are we loaded in the slit? (default of no)
HWND hSlit;				//The Window Handle to the Slit (for if we are loaded)

// Blackbox messages we want to "subscribe" to:
// BB_RECONFIGURE -> Sent when changing style and on reconfigure
// BB_BROADCAST -> Broadcast message (bro@m)
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

//file path storage
char rcpath[MAX_PATH];
char stylepath[MAX_PATH];

//start up positioning
int ScreenWidth;
int ScreenHeight;

//.rc file settings
int xpos, ypos;
int width, height;

bool wantInSlit;
bool alwaysOnTop;
bool snapWindow;
bool pluginToggle;
bool transparency;
bool fullTrans;	
bool drawBorder;
bool drawCross;
bool drawPos;
bool drawColor;
int alpha;
bool noBitmap;
char windowStyle[24];
char bitmapFile[MAX_PATH];
int ratio;
bool magStart;
bool parentstyle;
int copied = 0;
//style setting storage
COLORREF backColor, backColor2;
COLORREF backColorTo, backColorTo2;
COLORREF fontColor;
COLORREF actColor;
char fontFace[256];
int fontSize = 13;

bool mag = false;

StyleItem *myStyleItem, *myStyleItem2;
int bevelWidth;
int borderWidth;
COLORREF borderColor;

//menu items
Menu *myMenu, *windowStyleSubmenu, *configSubmenu, *settingsSubmenu, *otherSubmenu, *bitmapSubmenu, *styleSubmenu, *browseSubmenu;
Menu *ratioSubmenu, *generalConfigSubmenu;

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
void InitBBMagnify();
void mySetTimer();
void setStatus();
void createMenu();
void ColorToClipboard(char* color);

//===========================================================================

extern "C"
{
	__declspec(dllexport) int beginPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) void endPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) LPCSTR pluginInfo(int field);
	// This is the function BBSlit uses to load your plugin into the Slit
	__declspec(dllexport) int beginSlitPlugin(HINSTANCE hMainInstance, HWND hBBSlit);
	__declspec(dllexport) int beginPluginEx(HINSTANCE hMainInstance, HWND hBBSlit); 

}

//===========================================================================

#endif

// the end ....
