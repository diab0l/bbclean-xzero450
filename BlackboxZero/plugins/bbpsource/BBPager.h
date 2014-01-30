/*
 ============================================================================
 Blackbox for Windows: BBPager
 ============================================================================
 Copyright © 2003 nc-17@ratednc-17.com
 http://www.ratednc-17.com
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

 ============================================================================
*/

#ifndef __BBPAGER_H
#define __BBPAGER_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include "../../blackbox/BBApi.h"
#include "resource.h"
#include <string>
#include <vector>
#include <aggressiveoptimize.h>

using namespace std;

//===========================================================================

//const long magicDWord = 0x49474541;

extern HINSTANCE hInstance;

// data structures
struct FRAME 
{
	int width;
	int height;
	int rows;
	int columns;
	int bevelWidth;
	int borderWidth;
	int hideWidth;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	StyleItem *style;
};

struct DESKTOP 
{
	int width;
	int height;
	
	int fontSize;

	char fontFace[256];

	bool numbers;
	bool windows;

	COLORREF fontColor;
	COLORREF color;
	COLORREF colorTo;

	StyleItem *style;
};

struct ACTIVEDESKTOP 
{
	char styleType[MAX_LINE_LENGTH];

	bool useDesktopStyle;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	StyleItem *style;
};

struct WINDOW 
{
	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	StyleItem *style;
};

struct FOCUSEDWINDOW 
{
	char styleType[MAX_LINE_LENGTH];

	bool useWindowStyle;

	COLORREF borderColor;
	COLORREF color;
	COLORREF colorTo;

	StyleItem *style;
};

struct POSITION 
{
	int x;
	int y;
	int ox;
	int oy;
	int hx;
	int hy;
	int side;

	bool vertical;
	bool horizontal;
	bool raised;
	bool snapWindow;
	bool snapWindowOld;
	bool unix;
	bool autohide;
	bool autohideOld;
	bool hidden;
};

extern struct POSITION position;
extern struct FRAME frame;
extern struct DESKTOP desktop;
extern struct ACTIVEDESKTOP activeDesktop;
extern struct WINDOW window;
extern struct FOCUSEDWINDOW focusedWindow;

// Window information
extern int screenWidth, screenHeight;
extern double ratioX, ratioY;

typedef struct winStruct
{
	HWND window;
	RECT r;
	BOOL active;
	int desk;
} winStruct;

extern vector<winStruct> winList;

extern int winCount;

extern bool winMoving;
extern winStruct moveWin;
extern int mx, yx;
extern HWND hwndBBPager;

extern int leftMargin, topMargin;

extern bool drawBorder;

// File paths
extern char rcpath[MAX_PATH];
extern char bspath[MAX_PATH];
extern char stylepath[MAX_PATH];

extern char editor[MAX_LINE_LENGTH];

// Desktop information
extern int desktops;
extern int currentDesktop;
//extern RECT desktopRect[64];
extern vector<RECT> desktopRect;
extern int desktopChangeButton;
extern int focusButton;
extern int moveButton;

// Transparency
extern bool usingWin2kXP;
extern bool transparency;
extern int transparencyAlpha;

// Slit
extern bool inSlit, useSlit;
extern int xpos, ypos;

//===========================================================================
// function declarations

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CheckTaskEnumProc(HWND hwnd, LPARAM lParam);

extern void GetStyleSettings();

extern void InitRC();
extern void ReadRCSettings();
extern void WriteRCSettings();

extern void DrawBBPager(HWND hwnd);
extern void DrawBorder(HDC hdc, RECT rect, COLORREF borderColour, int borderWidth);
extern void DrawActiveDesktop(HDC buf, RECT r, int i);
extern void DrawActiveWindow(HDC buf, RECT r);
extern void DrawInactiveWindow(HDC buf, RECT r);

void GetPos(bool snap);
void SetPos(int place);

bool IsValidWindow(HWND hWnd);
int getDesktop(HWND h);

void UpdatePosition();

void ClickMouse();
void TrackMouse();
bool CursorOutside();

void DeskSwitch();

void FocusWindow();
void GrabWindow();
void DropWindow();

void HidePager();

void DisplayMenu();

void ToggleSlit();

//BOOL WINAPI BBSetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

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
