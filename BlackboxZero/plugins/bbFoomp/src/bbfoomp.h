/*
 ============================================================================
 bbFoomp // The foobar2000 plugin for Blackbox for Windows!
 Copyright © 2004 freeb0rn@yahoo.com

 Credits and thanks:
 qwilk, without his help and code I would have never started!
 azathoth, nc-17, tres`ni and other channel regulars, coders or not,
 thanks for bearing with my (often stupid) questions and helping me out!
 ============================================================================

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

 ============================================================================
*/

#ifndef __SLITCLOCK_H
#define __SLITCLOCK_H

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#define WM_XBUTTONDOWN 0x020B
#define WM_XBUTTONUP 0x020C
#define WM_NCXBUTTONDOWN 0x00AB
#define WM_NCXBUTTONUP 0x00AC
#define WM_NCXBUTTONDBLCLK 0x00AD
#define XBUTTON1 0x0001
#define XBUTTON2 0x0002

#include "BBApi.h"
#include <windows.h>
#include <stdlib.h>
#include <time.h>

//===========================================================================

#define PLUGIN_NAME		1
#define PLUGIN_VERSION	2
#define PLUGIN_AUTHOR	3
#define PLUGIN_RELEASE	4
#define PLUGIN_LINK		5
#define PLUGIN_EMAIL	6

#define BBFOOMP_UPDATE_TIMER 1
#define BB_BRINGTOFRONT 10504
#define TITLEWIDTH r.right - r.left

const short int NUM_BUTTONS = 9;

enum ButtonType
{
	REWIND_BUTTON,
	PLAY_BUTTON,
	PAUSE_BUTTON,
	STOP_BUTTON,
	FORWARD_BUTTON,
	PLAYLIST_BUTTON,
	OPEN_BUTTON,
	UPARROW_BUTTON,
	DOWNARROW_BUTTON
};

struct FoompButton
{
	void draw(HDC buf);
	int width();
	int height();
	bool pressed;
	bool clicked(int mouseX, int mouseY);

	RECT hitrect;
	int x, y;
	ButtonType type;
	char cmdarg[256];
	char altcmdarg[256];
private:
	void drawShape(HDC buf, int Penx, int Peny);
};


//===========================================================================

HINSTANCE hInstance;
HWND hwndPlugin, hwndBlackbox, hwndSlit;

// Blackbox messages we want to subscribe to...
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

// Path Variables
char rcpath[MAX_PATH];
char stylepath[MAX_PATH] = "";

// Variables that will be (later) dealt with with the settings.rc
int screenwidth, screenheight;
int xpos, ypos;
int width, height, BorderWidth;
int FooMode, FooWidth, FooModePrev;
char FooPath[MAX_LINE_LENGTH];
FoompButton buttons[NUM_BUTTONS];


// Style items
StyleItem OuterStyle, InnerStyle, ButtonStyle;
COLORREF ShadowColor;

char scFont[MAX_LINE_LENGTH];

// Miscellaneous
bool scHidden;
bool FooDockedToSlit, FooOnTop, usingWin2kXP, FooTrans, FooAlign, FooShadowsEnabled;
int FooScrollSpeed;
bool recbinEmpty;
int transparencyAlpha;
int useRegKey;
const int button_spacing = 12;
bool FirstUpdate; // Moved this up here from below.

// Menu Class
Menu *scMenu, *scSubMenu, *scSubMenu2;

// Display/graphic variables
int DisplayMode;	// If 1 = display mode, then mode = title; if 2 = display mode, then mode = controls.

// Determines style:
int InnerStyleIndex, OuterStyleIndex;

// Title [DisplayMode] variables, classes, etc.
struct Finfo {
	char song_title[MAX_LINE_LENGTH];
	HWND FooHandle;
	void update();
};
Finfo *FooClass;
char CurrentSong[MAX_LINE_LENGTH], DisplayedSong[MAX_LINE_LENGTH], jAmpScrollFiller[256] = "       ", NoInfoText[MAX_LINE_LENGTH];
bool foobar_v9 = false; // Different commandline syntax needed for newer versions.



//===========================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void UpdateTitle();
void UpdatePosition();
void ToggleDockedToSlit();
void GetStyleSettings();
void GetWindowName();
void ReadRCSettings();
void WriteDefaultRCSettings();
void Transparency();
void ClickMouse(int mouseX, int mouseY);
void TrackMouse();
void ChangeRectStyle(int style);
void refresh();
void DispModeControls(RECT r, HDC buf);
void DispModeTitle(RECT r, HDC buf, HDC src, HDC hdc, HBITMAP bufbmp, HBITMAP srcbmp, HBITMAP oldbuf, PAINTSTRUCT ps);
void CalculateButtonPositions(RECT r);
COLORREF GetShadowColor(StyleItem &style);


//===========================================================================

extern "C"
{
	__declspec(dllexport) int beginPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) int beginPluginEx(HINSTANCE hMainInstance, HWND hBBSlit);
	__declspec(dllexport) int beginSlitPlugin(HINSTANCE hMainInstance, HWND hwndBBSlit);
	__declspec(dllexport) void endPlugin(HINSTANCE hMainInstance);
	__declspec(dllexport) LPCSTR pluginInfo(int field);

}

//===========================================================================

#endif
