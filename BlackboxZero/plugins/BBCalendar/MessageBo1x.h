
#if !defined(AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_)
#define AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "windows.h"
//#include "bbcalendar.h"
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



#endif // !defined(AFX_MESSAGEBOX_H__A2893FB7_C936_11D3_B0FA_0040054C5E60__INCLUDED_)
