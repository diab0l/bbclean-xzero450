/*
 ============================================================================
 Blackbox for Windows: Plugin BBCalendar by Miroslav Petrasko (Theo)
 ============================================================================
 Copyright © 2001-2004 The Blackbox for Windows Development Team
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

 ============================================================================
*/



#ifndef __BBCALENDAR_H
#define __BBCALENDAR_H

#ifndef ULONG_PTR
#define ULONG_PTR DWORD
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED	0x00080000
#define LWA_COLORKEY	0x00000001
#define LWA_ALPHA		0x00000002
#endif // ndef WS_EX_LAYERED

#define NOTHING		0
#define TOP			1
#define BOTTOM		2
#define LEFT		3
#define RIGHT		4

#define HORIZONTAL	0
#define VERTICAL	1
#define LINE		2
#define ROW			3
#define FREEE		4

#define NONE		0
#define BMP			1
#define TOOLBAR		2
#define BUTTON		3
#define BUTTONPR	4
#define LABEL		5
#define WINLABEL	6
#define CLOCK		7
#define RECTT		8
#define TRANS		9
#define BORDER		10

#define POSS1		1
#define POSS2		2
#define BOTH		3

#define CALENDAR	0
#define DATEC		1
#define WEEK		2
#define DAYS		3
#define CURRENTD	4
#define ALARM		5

#define CENTER		0
#define STRETCH		1

#define DATEFONT	0
#define WEEKFONT	1
#define DAYSFONT	2
#define DEFAULT		3

#include <windows.h>
#include <time.h>
#include "BBApi.h"
#include "AggressiveOptimize.h"

#pragma comment(lib, "Blackbox.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "msimg32.lib") 


//#include "StdAfx.h"

#define IDT_TIMER 1
//typedef unsigned __int64 ULONG_PTR;
//typedef DWORD ULONG_PTR;
//typedef float REAL;
//typedef unsigned __int64* ULONG_PTR;

//===========================================================================

static int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
char weekDayNames[6]; 
bool drawType;
int moveMonth=0;


//gdi+ structs
//Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
//ULONG_PTR						gdiplusToken;

//OS info storage
DWORD      dwId;
DWORD      dwMajorVer;
DWORD      dwMinorVer;

RECT rect,r1;
RECT dayRect[31];

int osx = 7;
int osy = 6;
//temp storage
static char szTemp[MAX_LINE_LENGTH];

//window instances
HINSTANCE hInstance;
HWND hwndBBCalendar, hwndBlackbox;
bool inSlit = false;	//Are we loaded in the slit? (default of no)
HWND hSlit;				//The Window Handle to the Slit (for if we are loaded)

// Blackbox messages we want to "subscribe" to:
// BB_RECONFIGURE -> Sent when changing style and on reconfigure
// BB_BROADCAST -> Broadcast message (bro@m)
int msgs[] = {BB_RECONFIGURE, BB_BROADCAST, 0};

bool isAlarm[31];
bool isMAlarm[31];
bool isYAlarm[31];
//file path storage
char rcpath[MAX_PATH];
char stylepath[MAX_PATH];
char alarmpath[MAX_PATH];
char freeformpath[MAX_PATH];
char namepath[MAX_PATH];
char alarm[MAX_PATH];
char htime[20];
//start up positioning
int ScreenWidth;
int ScreenHeight;

bool drawBorder;
bool fullTrans = true;

//.rc file settings
int xpos, ypos;
int width, height;

int text_pos = 1;
bool wantInSlit;
bool alwaysOnTop;
bool snapWindow;
bool pluginToggle;
bool transparency;
bool showGrid;
bool sundayFirst;

char clockformat[256];
char drawclock[256];
bool shownames;
int alpha;
int day;
int month;
int year;
int move;  //how much we have to move the first day in the month

//char windowStyle[24];
int hour,minute;
//time stuff
time_t systemTime;
static char currentDate[10];
struct tm *localTime;
struct tm *helpTime;

char fontFace[4][256];
char name[100];
int fontSize;
int dateFontSize;

int dayx[32];
int dayy[32];
int days[32];

char bitmapfile[7][MAX_PATH];
 
int texta = 1;
int wpos = 1;
int bopt = 0;
bool deffont = false;

//style items......
//-----------------------------------------------------------------
COLORREF borderColor;
int bevelWidth;
int borderWidth;

COLORREF fontColor;
COLORREF nfontColor;

StyleItem *button, *buttonpr, *toolbar, *label, *winlabel, *cclock;

COLORREF buttonColor, buttonColorTo;
COLORREF buttonprColor, buttonprColorTo;
COLORREF labelColor, labelColorTo;
COLORREF winlabelColor, winlabelColorTo;
COLORREF clockColor, clockColorTo;
COLORREF toolbarColor, toolbarColorTo;

COLORREF tfontColor;
COLORREF buttonfontColor;
COLORREF buttonprfontColor;
COLORREF labelfontColor;
COLORREF winlabelfontColor;
COLORREF clockfontColor;

int astyle = 2;
int cdstyle = 2;
int cstyle = 2;
int wstyle = 2;
int dstyle = 2;
int nstyle = 2;
//-----------------------------------------------------------------
int placement = 6;
int drawMode = 0;
int hday;	

POINT poloha;

//menu items
Menu *myMenu, *windowStyleSubmenu, *configSubmenu, *settingsSubmenu, *weekSubmenu, *dateSubmenu;
Menu *calendarConfigSubmenu, *generalConfigSubmenu, *modeSubmenu, *textaSubmenu;
Menu *browseSubmenu,*bitmapSubmenu;
Menu *alarmStyleSubmenu,*calendarStyleSubmenu,*cdayStyleSubmenu,*weekStyleSubmenu,*dateStyleSubmenu, *daysStyleSubmenu;
Menu *monthSubmenu, *imageSubmenu, *imageOptionsSubmenu, *fontSubmenu, *placementSubmenu;

//===========================================================================
LRESULT CALLBACK MesProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
RECT  okRect;
bool pressed = false;

int CreateMessageBox();
void DestroyMessageBox();
HWND hwndMessage;
char dmessage[MAX_PATH];
char dcaption[MAX_PATH];
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
void InitBBCalendar();
void getCurrentDate();
void mySetTimer();
void setStatus();
void createMenu();
void drawCalendar(HDC &hdc, RECT &re);
void drawRCalendar(HDC &hdc, RECT &re);
void drawFree(HDC &hdc);
void drawWeek(HDC &hdc, RECT &re);
void drawName(HDC &hdc, RECT &re);
void getAlarms();
void createAlarmFile();
void executeAlarm(int ddd);
int getDay(POINT &pos);
void drawRect(HDC &hdc, int type, RECT &re, LPCSTR path, LPCSTR text);
void setFontColor();
void conRect(RECT &recttc, int con);
LPCSTR getName();
void place(int where);



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
