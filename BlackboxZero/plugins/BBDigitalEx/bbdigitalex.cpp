/*
 ============================================================================
 Blackbox for Windows: Plugin BBDigitalEx 1.0 by Miroslav Petrasko [Theo]
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

  For additional license information, please read the included license

 ============================================================================
*/

#include "bbdigitalex.h"
#include "resource.h"

LPSTR szAppName = "BBDigitalEx";		// The name of our window class, etc.
LPSTR szVersion = "BBDigitalEx v1.0 b7";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0 beta 7";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-01-25";
LPSTR szInfoLink = "theo.host.sk";
LPSTR szInfoEmail = "theo.devil@gmail.com";

//===========================================================================

int beginPlugin(HINSTANCE hPluginInstance)
{
	WNDCLASS wc;
	hwndBlackbox = GetBBWnd();
	hInstance = hPluginInstance;

	// Register the window class...
	ZeroMemory(&wc,sizeof(wc));
	wc.lpfnWndProc = WndProc;			// our window procedure
	wc.hInstance = hPluginInstance;		// hInstance of .dll
	wc.lpszClassName = szAppName;		// our window class name
	if (!RegisterClass(&wc)) 
	{
		MessageBox(hwndBlackbox, "Error registering window class", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Get plugin and style settings...
	ReadRCSettings();
//	executeAlarm();
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBDigitalEx();
	GetStyleSettings();
	getCurrentDate();

	// Create the window...
	hwndBBDigitalEx = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						szAppName,										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,	// window parameters
						xpos,											// x position
						ypos,											// y position
						width,											// window width
						height,											// window height
						NULL,											// parent window
						NULL,											// no menu
						hPluginInstance,								// hInstance of .dll
						NULL);
	if (!hwndBBDigitalEx)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	//Start the plugin timer
	mySetTimer();
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBDigitalEx);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBDigitalEx, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBDigitalEx, GWL_USERDATA, magicDWord);
	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBDigitalEx, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBDigitalEx, SW_SHOW);

	InvalidateRect(hwndBBDigitalEx, NULL, true);
	
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBDigitalExReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBDigitalEx()
{
	
	//Get Platform type
	dwId = 0;
    dwMajorVer = 0;
    dwMinorVer = 0;

	//Get Platform type
	_GetPlatformId (&dwId, &dwMajorVer, &dwMinorVer);
    
	//get screen dimensions
	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	
}

//===========================================================================

void endPlugin(HINSTANCE hPluginInstance)
{
	// Release our timer resources
	KillTimer(hwndBBDigitalEx, IDT_TIMER);
//	KillTimer(hwndBBDigitalEx, IDT_ALARMTIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...


	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBDigitalEx, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBDigitalEx);
	// Destroy our window...
	DestroyWindow(hwndBBDigitalEx);
	// Unregister window class...
	UnregisterClass(szAppName, hPluginInstance);
}

//===========================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{		
		// Window update process...
		case WM_PAINT:
		{
			// Create buffer hdc's, bitmaps etc.
			PAINTSTRUCT ps;  RECT r;

			//get screen buffer
            HDC hdc_scrn = BeginPaint(hwnd, &ps);

			//retrieve the coordinates of the window's client area.
			GetClientRect(hwnd, &r);

			//to prevent flicker of the display, we draw to memory first,
            //then put it on screen in one single operation. This is like this:

            //first get a new 'device context'
			HDC hdc = CreateCompatibleDC(NULL);

			
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc_scrn, r.right, r.bottom);
            SelectObject(hdc, bufbmp);
			

			if(drawBorder)
			{
				//Make background gradient
				if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit))
				{

				MakeGradient(hdc, r, myStyleItem->type,
							backColor, backColorTo,
							myStyleItem->interlaced,
							myStyleItem->bevelstyle,
							myStyleItem->bevelposition,
							bevelWidth, borderColor,
							borderWidth);
				}
				
				// if we draw border we have to make the painting rectangle smaller
				r.left = r.left + (bevelWidth + borderWidth);
				r.top = r.top + (bevelWidth + borderWidth);
				r.bottom = (r.bottom - (bevelWidth + borderWidth));
				r.right = (r.right - (bevelWidth + borderWidth));
			}
			

			SetBkMode(hdc, TRANSPARENT);
		
			

			// if fultrans the whole backgorund is painted pink

			if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				{
					HBRUSH hbOrig, hBrush;
					GetClientRect(hwnd, &rec);
					hBrush = CreateSolidBrush(0xFF00FF);
					hbOrig = (HBRUSH)SelectObject(hdc, hBrush);
					Rectangle(hdc, -1,-1,rec.right+1, rec.bottom+1);
					DeleteObject(hBrush);
					DeleteObject(hbOrig);
				}
	
						// if a bitmap path is found the bitmap is painter
			
					if (!strcmp(bitmapFile,".none")==0) 
						{
					
					HANDLE image;
					image = LoadImage(NULL, bitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					
					HDC hdcMem = CreateCompatibleDC(hdc);
					
					HBITMAP old = (HBITMAP) SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);

	
					TransparentBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
					SelectObject(hdcMem, old);
					DeleteObject(old);
					DeleteObject(image);
					DeleteDC(hdcMem);
					
						}
					else 
							
			// the second background is painted

					if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit))
					{
				
					MakeGradient(hdc, r, myStyleItem2->type,
							backColor2, backColorTo2,
							myStyleItem2->interlaced,
							myStyleItem2->bevelstyle,
							myStyleItem2->bevelposition,
							bevelWidth, borderColor, 0); 
					}
		
				

				r.left +=3;
				r.right -=3;
				r.bottom -=3;
				r.top += 3;

			
				HGDIOBJ otherfont = CreateFont(fontSize, 
					0, (text_pos<3)?0:900,  0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace);
				
				SelectObject(hdc, otherfont);
				SetTextColor(hdc,fontColor);

			switch (text_pos)
			{
				case NOTHING:
						drawClock(hdc,r,drawMode);
						break;
				
				case TOP:
						r.top += fontSize+4;
						drawClock(hdc,r,drawMode);
						r.top -= fontSize+4;
				    	DrawText(hdc,drawclock,-1,&r,DT_TOP | DT_CENTER | DT_SINGLELINE);
						break;
				
				case BOTTOM:
						r.bottom -= fontSize+4;
						drawClock(hdc,r,drawMode);
						r.bottom += fontSize+4;
						DrawText(hdc,drawclock,-1,&r,DT_BOTTOM | DT_CENTER | DT_SINGLELINE);
						break;

				case LEFT:
						r.left += fontSize+4;
						drawClock(hdc,r,drawMode);
						r.left -= fontSize+4;
						r.right = r.left + fontSize;
						r.bottom = r.bottom + fontSize;
						DrawText(hdc,drawclock,-1,&r,DT_BOTTOM | DT_LEFT | DT_SINGLELINE);
						break;
				
				case RIGHT:
						r.right -= fontSize+4;
						drawClock(hdc,r,drawMode);
						r.right += fontSize+4;
						r.left = r.right - fontSize;
						r.bottom = r.bottom + fontSize;
						DrawText(hdc,drawclock,-1,&r,DT_BOTTOM | DT_LEFT | DT_SINGLELINE);
						break;
			}
				
				
				DeleteObject(otherfont);
				//Paint to the screen
				BitBlt(hdc_scrn, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
			
			
			// Remember to delete all objects!
			
			SelectObject(hdc, bufbmp); //mortar: select just incase it is no longer in the context
			DeleteDC(hdc);         //gdi: first delete the dc
			DeleteObject(bufbmp);  //gdi: now the bmp is free

			//takes care of hdc_scrn
			EndPaint(hwnd, &ps);

			return 0;
		}
		break;

		// ==========

		// If Blackbox sends a reconfigure message, load the new style settings and update the window...
		case BB_RECONFIGURE:
		{
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
			GetStyleSettings();
			InvalidateRect(hwndBBDigitalEx, NULL, true);
		}
		break;

		// ==========

		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);
		
			if (!_stricmp(szTemp, "@BBShowPlugins") &&  pluginToggle && !inSlit)
			{
				// Show window and force update...
				ShowWindow( hwndBBDigitalEx, SW_SHOW);
				InvalidateRect( hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBDigitalEx, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExAbout"))
			{
				sprintf(szTemp, "%s\n\n%s ©2004 %s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBlackbox,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type
				
				

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
			}
			else if (!_stricmp(szTemp, "@BBDigitalExPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBDigitalExOnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;

				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBDigitalEx, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBDigitalEx, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBDigitalEx);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBDigitalEx);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBDigitalExFullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBDigitalExSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBDigitalExDrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExDrawMode"))
			{
				drawMode = !drawMode;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalEx24"))
			{
				h24format = !h24format;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExEnableAlarms"))
			{
				alarms = !alarms;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExRemove0"))
			{
				remove_zero = !remove_zero;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExShowSeconds"))
			{
				showSeconds = !showSeconds;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExBlink"))
			{
				blink = !blink;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExNoBitmap"))
			{
				//noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExNoCBitmap"))
			{
				//noBitmap = true;
				strcpy(clockbitmapFile, ".none");

				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExEditAlarmsRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, alarmpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExReloadSettings"))
			{
				
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBDigitalEx);

					//Re-initialize
					ReadRCSettings();
					InitBBDigitalEx();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBDigitalEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBDigitalEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBDigitalEx);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBDigitalEx, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBDigitalExSaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExMode", 15))
			{
				drawMode = atoi(szTemp + 16);
				
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExFontSize", 19))
			{
				fontSize = atoi(szTemp + 20);
				
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExClockFormat", 22))
			{
				strcpy(clockformat,szTemp + 24);
				
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExWidth", 16))
			{ //changing the clock size
				width = atoi(szTemp + 17);
				
				if ( alwaysOnTop) SetWindowPos( hwndBBDigitalEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBDigitalEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExHeight", 17))
			{ //changing the clock size
				height = atoi(szTemp + 18);
									
				if ( alwaysOnTop) SetWindowPos( hwndBBDigitalEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBDigitalEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				if (transparency) setStatus();					
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExSetBitmap", 20))
			{
				
				strcpy(bitmapFile,szTemp + 22);
			//	noBitmap = false;
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExMessage", 18))
			{
				
				static char mess[MAX_LINE_LENGTH];
				strcpy(mess,szTemp + 20);
				sprintf(szTemp, "%.2d:%.2d %s", hour,minute,szAppName);

				CMessageBox box(NULL,					// hWnd
								_T(mess),						// Text
								_T(szTemp),						// Caption
								MB_OK | MB_SETFOREGROUND);		// type

				box.SetIcon(IDI_C, hInstance);
				
				box.DoModal();
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExPlayWav", 18))
			{
				strcpy(szTemp,szTemp + 20);
				PlaySound(szTemp, NULL, SND_FILENAME | SND_ASYNC);
							
			}
			else if (!_strnicmp(szTemp, "@BBDigitalExDatePossition", 24))
			{
				text_pos = atoi(szTemp + 25);
			
				InvalidateRect(hwndBBDigitalEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBDigitalExLoadBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBDigitalEx;
			ofn.lpstrFile = bitmapFile;
			ofn.nMaxFile = sizeof(bitmapFile);
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
//			ofn.lpstrInitialDir = defaultpath;
//			ofn.lpstrTitle = title;
//			ofn.lpstrDefExt = defaultextension;	

			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (!GetOpenFileName(&ofn)) strcpy(bitmapFile,".none");

			InvalidateRect(hwndBBDigitalEx, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BBDigitalExLoadCBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBDigitalEx;
			ofn.lpstrFile = clockbitmapFile;
			ofn.nMaxFile = sizeof(clockbitmapFile);
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
//			ofn.lpstrInitialDir = defaultpath;
//			ofn.lpstrTitle = title;
//			ofn.lpstrDefExt = defaultextension;	

			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
			if (!GetOpenFileName(&ofn)) strcpy(clockbitmapFile,".none");

			InvalidateRect(hwndBBDigitalEx, NULL, true);

			}
		}
		break;

		// ==========

		case WM_WINDOWPOSCHANGING:
		{
			// Is SnapWindowToEdge enabled?
			if (!inSlit && snapWindow)
			{
				// Snap window to screen edges (if the last bool is false it uses the current DesktopArea)
				if(IsWindowVisible(hwnd)) SnapWindowToEdge((WINDOWPOS*)lParam, 10, true);
			}
		}
		break;

		// ==========

		// Save window position if it changes...
		case WM_WINDOWPOSCHANGED:
		{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;
				xpos = windowpos->x;
				ypos = windowpos->y;
		}
		break;

		// ==========

		case WM_DISPLAYCHANGE:
		{
			if(!inSlit || !hSlit)
			{
				// IntelliMove(tm)... <g>
				// (c) 2003 qwilk
				//should make this a function so it can be used on startup in case resolution changed since
				//the last time blackbox was used.
				int relx, rely;
				int oldscreenwidth = ScreenWidth;
				int oldscreenheight = ScreenHeight;
				ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
				ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
				if (xpos > oldscreenwidth / 2)
				{
					relx = oldscreenwidth - xpos;
					xpos = ScreenWidth - relx;
				}
				if (ypos > oldscreenheight / 2)
				{
					rely = oldscreenheight - ypos;
					ypos = ScreenHeight - rely;
				}
				MoveWindow(hwndBBDigitalEx, xpos, ypos, width, height, true);
			}
		}
		break;

		// ==========
		// Allow window to move if the cntrl key is not pressed...
		case WM_NCHITTEST:
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				return HTCAPTION;
			else
				return HTCLIENT;
		}
		break;

		case WM_NCLBUTTONDOWN:
		{
			/* Please do not allow plugins to be moved in the slit.
			 * That's not a request..  Okay, so it is.. :-P
			 * I don't want to hear about people losing their plugins
			 * because they loaded it into the slit and then moved it to
			 * the very edge of the slit window and can't get it back..
			 */
			
			if((!inSlit))
				return DefWindowProc(hwnd,message,wParam,lParam);
		}
		break;

	/*	case WM_LBUTTONUP: 
		{
			//	mag = !mag;
			//	InvalidateRect(hwndBBDigitalEx, NULL, false);
		}
		break;
	*/	
		// ==========

		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			// show the menu...
			createMenu();
		}
		break;
       //-------------------------------------------------

		case WM_LBUTTONUP: 
		{
			//open control panel with:  control timedate.cpl,system,0
			BBExecute(GetDesktopWindow(), NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
		}
		break;
	
		// ==========

		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_TIMER:
				{
					//redraw the window
					show = !show;
					getCurrentDate();
					InvalidateRect(hwndBBDigitalEx, NULL, false);
					if ((second == 0)&&(show)&&(alarms)) executeAlarm();
					
				} break;

			/*	case IDT_ALARMTIMER:
				{
					//redraw the window
					//show = !show;
					
					InvalidateRect(hwndBBDigitalEx, NULL, false);
				} break; */
			}
		}
		break;

	/*	case WM_MOUSEMOVE:
			{
			InvalidateRect(hwndBBDigitalEx, NULL, false);
			}break;
*/
		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

//===========================================================================
//Need to clean this function up so that it is more readable, without removing functionality.

void GetStyleSettings()
{
	// Get the path to the current style file from Blackbox...
	strcpy(stylepath, stylePath());

	// ...and some additional parameters
	bevelWidth = ReadInt(stylepath, "bevelWidth:", 2);
	borderWidth = ReadInt(stylepath, "borderWidth:", 1);

	// Get the applicable color settings from the current style...
	backColor = ReadColor(stylepath, "toolbar.color:", "#000000");
	backColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#FFFFFF");
	
	borderColor = ReadColor(stylepath, "borderColor:", "#000000");

	// ...gradient type, bevel etc. from toolbar:(using a StyleItem)...
	char tempstyle[MAX_LINE_LENGTH];
	strcpy(tempstyle, ReadString(stylepath, "toolbar:", "Flat Gradient Vertical"));
	if (myStyleItem) delete myStyleItem;
	myStyleItem = new StyleItem;
	ParseItem(tempstyle, myStyleItem);

	
	
 if(StrStrI(windowStyle, "label") != NULL  && strlen(windowStyle) < 6)
	{
		// ...gradient type, bevel etc. from toolbar.label:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.label:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (myStyleItem2) delete myStyleItem2;	//if everything is found in toolbar.label: then make a new StyleItem
			myStyleItem2 = new StyleItem;			
			ParseItem(tempstyle2, myStyleItem2);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.label.color:", "")))
				backColor2 = ReadColor(stylepath, "toolbar.label.color:", "#000000");
			else
    			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.label.colorTo:", "")))
				backColorTo2 = ReadColor(stylepath, "toolbar.label.colorTo:", "#000000");
			else
				backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			fontColor = ReadColor(stylepath, "toolbar.label.textColor:", "#FFFFFF");
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.label
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	} 
	else if(StrStrI(windowStyle, "windowlabel") != NULL)
	{
		// ...gradient type, bevel etc. from toolbar.windowLabel:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.windowLabel:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (myStyleItem2) delete myStyleItem2;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			myStyleItem2 = new StyleItem;			
			ParseItem(tempstyle2, myStyleItem2);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.windowLabel.color:", "")))
				backColor2 = ReadColor(stylepath, "toolbar.windowLabel.color:", "#000000");
			else
    			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.windowLabel.colorTo:", "")))
				backColorTo2 = ReadColor(stylepath, "toolbar.windowLabel.colorTo:", "#000000");
			else
				backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			fontColor = ReadColor(stylepath, "toolbar.windowLabel.textColor:", "#FFFFFF");
		}
	}
	else if(StrStrI(windowStyle, "toolbar") != NULL)
	{
		if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");		//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");	//the toolbar.color if parent relative is found for toolbar.windowLabel
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
	}
		else if(StrStrI(windowStyle, "buttonpr") != NULL)
	{
		// ...gradient type, bevel etc. from toolbar.windowLabel:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.button.pressed:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (myStyleItem2) delete myStyleItem2;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			myStyleItem2 = new StyleItem;			
			ParseItem(tempstyle2, myStyleItem2);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.button.pressed.color:", "")))
				backColor2 = ReadColor(stylepath, "toolbar.button.pressed.color:", "#000000");
			else
    			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.button.pressed.colorTo:", "")))
				backColorTo2 = ReadColor(stylepath, "toolbar.button.pressed.colorTo:", "#000000");
			else
				backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			fontColor = ReadColor(stylepath, "toolbar.button.pressed.picColor:", "#FFFFFF");
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	}
	else if(StrStrI(windowStyle, "buttonnp") != NULL)
	{
		// ...gradient type, bevel etc. from toolbar.windowLabel:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.button:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (myStyleItem2) delete myStyleItem2;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			myStyleItem2 = new StyleItem;			
			ParseItem(tempstyle2, myStyleItem2);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.button.color:", "")))
				backColor2 = ReadColor(stylepath, "toolbar.button.color:", "#000000");
			else
    			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.button.colorTo:", "")))
				backColorTo2 = ReadColor(stylepath, "toolbar.button.colorTo:", "#000000");
			else
				backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			fontColor = ReadColor(stylepath, "toolbar.button.picColor:", "#FFFFFF");
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	}
	else
	{
		// ...gradient type, bevel etc. from toolbar.clock:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.clock:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (myStyleItem2) delete myStyleItem2;	//if everything is found in toolbar.clock: then make a new StyleItem
			myStyleItem2 = new StyleItem;			
			ParseItem(tempstyle2, myStyleItem2);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.clock.color:", "")))
				backColor2 = ReadColor(stylepath, "toolbar.clock.color:", "#000000");
			else
    			backColor2 = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.clock.colorTo:", "")))
				backColorTo2 = ReadColor(stylepath, "toolbar.clock.colorTo:", "#000000");
			else
				backColorTo2 = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			fontColor = ReadColor(stylepath, "toolbar.clock.textColor:", "#FFFFFF");
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	}

	// ...font settings...
	strcpy(fontFace, ReadString(stylepath, "toolbar.font:", ""));
	if (!_stricmp(fontFace, "")) strcpy(fontFace, ReadString(stylepath, "*font:", "Tahoma"));
	
}

//===========================================================================

void ReadRCSettings()
{
	char temp[MAX_LINE_LENGTH], path[MAX_LINE_LENGTH], defaultpath[MAX_LINE_LENGTH];
	int nLen;
		
	// First we look for the config file in the same folder as the plugin...
	GetModuleFileName(hInstance, rcpath, sizeof(rcpath));
	nLen = strlen(rcpath) - 1;
	while (nLen >0 && rcpath[nLen] != '\\') nLen--;
	rcpath[nLen + 1] = 0;
	strcpy(temp, rcpath);
	strcpy(path, rcpath);
	strcpy(alarmpath, rcpath);

	strcat(alarmpath,"alarms.rc");
	if (!FileExists(alarmpath)) createAlarmFile();
	
	strcat(temp, "bbdigitalex.rc");
	strcat(path, "bbdigitalexrc");
	// ...checking the two possible filenames bbanalog.rc and bbanalogrc ...
	
	if (FileExists(temp)) strcpy(rcpath, temp);
	else if (FileExists(path)) strcpy(rcpath, path);

	// ...if not found, we try the Blackbox directory...
	else
	{
		// ...but first we save the default path (bbanalog.rc in the same
		// folder as the plugin) just in case we need it later (see below)...
		strcpy(defaultpath, temp);
		GetBlackboxPath(rcpath, sizeof(rcpath));
		strcpy(temp, rcpath);
		strcpy(path, rcpath);
			
		strcat(temp, "bbdigitalex.rc");
		strcat(path, "bbdigitalexrc");
		if (FileExists(temp)) strcpy(rcpath, temp);
		else if (FileExists(path)) strcpy(rcpath, path);
		else // If no config file was found, we use the default path and settings, and return
		{
			strcpy(rcpath, defaultpath);
			xpos = ypos = 10;
			width = 100; 
			height = 70;
			alpha = 160;
			wantInSlit = true;
			alwaysOnTop = true;
			snapWindow = true;
			transparency = false;
			fullTrans = false;
			drawBorder = false;
			pluginToggle = false;
			fontSize = 6;
			strcpy(windowStyle, "windowlabel");
			strcpy(bitmapFile, ".none");
			strcpy(clockformat, "%d %a %#H:%M");
		//	noBitmap = true;
			h24format = true;
			drawMode = HORIZONTAL;
//			drawDate = true;
//			magStart = false;

			WriteRCSettings();
			
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbdigitalex.x:", 10);
	ypos = ReadInt(rcpath, "bbdigitalex.y:", 10);

	width = ReadInt(rcpath, "bbdigitalex.width:", 100);
	height = ReadInt(rcpath, "bbdigitalex.height:", 50);

	text_pos = ReadInt(rcpath, "bbdigitalex.date.possition:", 0);
	if ((text_pos<0) ||(text_pos>4)) text_pos = 0; 

	alpha = ReadInt(rcpath, "bbdigitalex.alpha:", 160);
	if(alpha > 255) alpha = 255;
	
	if(ReadString(rcpath, "bbdigitalex.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbdigitalex.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "bbdigitalex.alwaysOnTop:", true);
	
	drawBorder = ReadBool(rcpath, "bbdigitalex.drawBorder:", true);
	drawMode = ReadInt(rcpath, "bbdigitalex.drawMode:", HORIZONTAL);
	remove_zero = ReadBool(rcpath, "bbdigitalex.remove.0:", false);
	h24format = ReadBool(rcpath, "bbdigitalex.24hFormat:", false);
	showSeconds = ReadBool(rcpath, "bbdigitalex.show.seconds:", true);
	alarms = ReadBool(rcpath, "bbdigitalex.enabled.alarms:", false);
//	if (magStart) mag = true;

	snapWindow = ReadBool(rcpath, "bbdigitalex.snapWindow:", true);
	blink = ReadBool(rcpath, "bbdigitalex.blink:", true);
	transparency = ReadBool(rcpath, "bbdigitalex.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbdigitalex.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbdigitalex.fontSize:", 6);
	alwaysOnTop = ReadBool(rcpath, "bbdigitalex.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbdigitalex.pluginToggle:", false);
	strcpy(windowStyle, ReadString(rcpath, "bbdigitalex.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");
//	strcpy(bitmapFile, ReadString(rcpath, "bbdigitalex.bitmapFile:", ".none"));
//	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 

	strcpy(bitmapFile, ReadString(rcpath, "bbdigitalex.bitmapFile:",	".none"));
	strcpy(clockbitmapFile, ReadString(rcpath, "bbdigitalex.clockbitmapFile:",	".none"));
	
	strcpy(clockformat, ReadString(rcpath, "bbdigitalex.clockformat:", "%d %a %#H:%M"));

}
//---------------------------------------------------------------------------
void createAlarmFile()
{
	static char szTemp[MAX_LINE_LENGTH];
//	static char temp[8];
	DWORD retLength = 0;

	HANDLE file = CreateFile(alarmpath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		sprintf(szTemp, "!============================\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! BBDigitalEx %s alarms file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! Enter alarms here - one per line.\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! For example:\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! 15.46: @ShrinkMemory \r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! 01.12: @BB8BallPredict \r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	}
	CloseHandle(file);
}

void WriteRCSettings()
{


	static char szTemp[MAX_LINE_LENGTH];
	static char temp[8];
	
	DWORD retLength = 0;

	// Write plugin settings to config file, using path found in ReadRCSettings()...
	HANDLE file = CreateFile(rcpath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		sprintf(szTemp, "!============================\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! BBDigitalEx %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbdigitalex.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbdigitalex.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.date.possition: %d\r\n", text_pos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbdigitalex.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.drawBorder: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.drawMode: %d\r\n", drawMode, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(remove_zero) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.remove.0: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(showSeconds) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.show.seconds: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(h24format) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.24hFormat: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alarms) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.enabled.alarms: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(blink) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbdigitalex.blink: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

    	sprintf(szTemp, "bbdigitalex.fontSize: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbdigitalex.clockformat: %s\r\n", clockformat);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.bitmapFile: %s\r\n", bitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbdigitalex.clockbitmapFile: %s\r\n", clockbitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	
	}
	CloseHandle(file);


	


}

//===========================================================================

//Plugin info for later BB4win support
LPCSTR pluginInfo(int field)
{
	// pluginInfo is used by Blackbox for Windows to fetch information about
	// a particular plugin. At the moment this information is simply displayed
	// in an "About loaded plugins" MessageBox, but later on this will be
	// expanded into a more advanced plugin handling system. Stay tuned! :)

	switch (field)
	{
		case 1:
			return szAppName; // Plugin name
		case 2:
			return szInfoVersion; // Plugin version
		case 3:
			return szInfoAuthor; // Author
		case 4:
			return szInfoRelDate; // Release date, preferably in yyyy-mm-dd format
		case 5:
			return szInfoLink; // Link to author's website
		case 6:
			return szInfoEmail; // Author's email

		// ==========

		default:
			return szVersion; // Fallback: Plugin name + version, e.g. "MyPlugin 1.0"
	}
}

//===========================================================================

//so there you just use BBSLWA like normal SLWA
//(c)grischka
BOOL WINAPI BBSetLayeredWindowAttributes(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	static BOOL (WINAPI *pSLWA)(HWND, COLORREF, BYTE, DWORD);
	static unsigned int f=0;
	for (;;) {
		if (2==f)   return pSLWA(hwnd, crKey, bAlpha, dwFlags);
		// if it's not there, just do nothing and report success
		if (f)      return TRUE;
		*(FARPROC*)&pSLWA = GetProcAddress(GetModuleHandle("USER32"), "SetLayeredWindowAttributes");
		f = pSLWA ? 2 : 1;
	}
}

//===========================================================================

//check for OS version
int WINAPI _GetPlatformId(DWORD *pdwId, DWORD *pdwMajorVer, DWORD *pdwMinorVer)
{
	OSVERSIONINFO  osvinfo;
	ZeroMemory(&osvinfo, sizeof(osvinfo));
	osvinfo.dwOSVersionInfoSize = sizeof (osvinfo);
	GetVersionEx(&osvinfo);
	*pdwId       = osvinfo.dwPlatformId;
	*pdwMajorVer = osvinfo.dwMajorVersion;
	*pdwMinorVer = osvinfo.dwMinorVersion;
	return 0;
}

//===========================================================================


void mySetTimer()
{
	
		//Start the 0.50 second plugin timer
		SetTimer(hwndBBDigitalEx,		// handle to main window 
				IDT_TIMER,			// timer identifier 
				500,				// second interval 
				(TIMERPROC) NULL);	// no timer callback 
		
/*		SetTimer(hwndBBDigitalEx,		// handle to main window 
				IDT_ALARMTIMER,			// timer identifier 
				60000,				// second interval 
				(TIMERPROC) NULL);	// no timer callback 
*/	
}


//===========================================================================

int beginSlitPlugin(HINSTANCE hMainInstance, HWND hBBSlit)
{
	/* Since we were loaded in the slit we need to remember the Slit
	 * HWND and make sure we remember that we are in the slit ;)
	 */
	inSlit = true;
	hSlit = hBBSlit;

	// Start the plugin like normal now..
	return beginPlugin(hMainInstance);
}


//--------------------------------------------------------------------
//sets the transparency and the full transparent status
void setStatus()
{

	if(!inSlit)
					{
						if (transparency && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBDigitalEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBDigitalEx, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBDigitalEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBDigitalEx, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBDigitalEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBDigitalEx, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBDigitalEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBDigitalEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBDigitalEx, NULL, false);		
}


void getCurrentDate()
{
//	if (div(year,4).rem==0) daysInMonth[1]=29;
//	else daysInMonth[1]=28;
	
	time(&systemTime);
	localTime = localtime(&systemTime);
	second	= localTime->tm_sec;
	minute	= localTime->tm_min;
	hour		= localTime->tm_hour;
	rhour = hour;
	if (hour>=12 && hour<=23) strcpy(mer,"pm");
	else strcpy(mer,"am");
	
    if (!h24format){
	if(hour > 12) hour -= 12;
	if(hour == 0) hour = 12;
	}
/*	month = localTime->tm_mon;
	year = localTime->;
	day = localTime->tm_mday;
*/
	strftime(drawclock,256,clockformat, localTime);

	strftime(szTemp, 10, "%m", localTime);
	month = (int)atoi(szTemp);
	strftime(szTemp, 10, "%Y", localTime);
	year = (int)atoi(szTemp);
	strftime(szTemp, 10, "%d", localTime);
	day = (int)atoi(szTemp);

}

void drawClock(HDC &hdc, RECT r, int mode)
{
	switch (mode)
	{
			
	case  HORIZONTAL:
			if (showSeconds)
				
			{
				vl = (r.bottom - r.top)/2;
				hl = ((r.right - r.left)-42)/6;

				if (!((remove_zero) && (div((int)hour,10).quot == 0)))	drawNumber(hdc, div((int)hour,10).quot, r.left +2, r.top, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + hl + 8, r.top, vl, hl);

				if (show||!blink)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2 - 9 - hl, r.top, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left + (r.right - r.left)/2 - 3 - hl, r.top, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left + (r.right - r.left)/2 +3, r.top, vl, hl);
				
				if (show||!blink)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2 + 7 + hl , r.top, vl, hl);
				
				drawNumber(hdc, div((int)second,10).quot, r.right - 2*hl - 8, r.top, vl, hl);
				drawNumber(hdc, div((int)second,10).rem, r.right - 2 - hl, r.top, vl, hl);
			}
			else
			{
				vl = (r.bottom - r.top)/2;
				hl = ((r.right - r.left)-30)/4;

				if (!((remove_zero) && (div((int)hour,10).quot == 0)))	drawNumber(hdc, div((int)hour,10).quot, r.left +2, r.top, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + hl + 8, r.top, vl, hl);

				if (show||!blink)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2, r.top, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.right - 2*hl - 8, r.top, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.right - 2 - hl, r.top, vl, hl);
			}
			break;

	case  VERTICAL:
	
			if (showSeconds)
				
			{
				hl = ((r.right - r.left)-9)/2;
				vl = (r.bottom - r.top-21)/6;

				drawNumber(hdc, div((int)hour,10).quot, r.left + 2, r.top + 2, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + 8 + hl, r.top + 2  , vl, hl);

				if (show||!blink)	drawNumber(hdc, 11, r.left + 2 , r.top + (r.bottom - r.top)/2 - vl-5, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left + 2, r.top + (r.bottom - r.top)/2 - vl, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left+ 8 + hl, r.top + (r.bottom - r.top)/2 - vl, vl, hl);
				
				if (show||!blink)	drawNumber(hdc, 11, r.left + 2, r.top + (r.bottom - r.top)/2 + vl+5, vl, hl);
				
				drawNumber(hdc, div((int)second,10).quot, r.left+2, r.bottom - 2 - 2*vl, vl, hl);
				drawNumber(hdc, div((int)second,10).rem, r.left+ 8 + hl, r.bottom - 2 - 2*vl, vl, hl);
			}
			else
			{
				hl = ((r.right - r.left)-9)/2;
				vl = (r.bottom - r.top-15)/4;

				drawNumber(hdc, div((int)hour,10).quot, r.left + 2, r.top + 2, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + 8 + hl, r.top + 2  , vl, hl);

				if (show||!blink)	drawNumber(hdc, 11, r.left + 2 , r.top + (r.bottom - r.top)/2, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left+2, r.bottom - 2 - 2*vl, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left+ 8 + hl, r.bottom - 2 - 2*vl, vl, hl);
			
			}
			break;

	case JUSTTEXT:
		    break;

	case BITMP:
		//----------------------------------------------------------
//overlay bitmap
	if ((!strcmp(clockbitmapFile,".none")==0)&&(FileExists(clockbitmapFile)))
						{
					
					
					
					HANDLE image;
					image = LoadImage(NULL, clockbitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					
					HDC hdcMem = CreateCompatibleDC(hdc);
					
					HBITMAP old = (HBITMAP) SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);

	
//					TransparentBlt(hdc, r.left, r.top, r.right, r.bottom, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
					TransparentBlt(hdc, r.left, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)hour,10).quot*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);
					TransparentBlt(hdc, r.left+(r.right-r.left)/7, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)hour,10).rem*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);

				if (show||!blink)	TransparentBlt(hdc, r.left+(r.right-r.left)/7*2, r.top, ((r.right-r.left)/14), r.bottom-r.top, hdcMem, 10*(int)(bitmap.bmWidth/10.5), 0, (int)(bitmap.bmWidth/21), bitmap.bmHeight, 0xff00ff);
				

					TransparentBlt(hdc, r.left+(r.right-r.left)/7*2.5, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)minute,10).quot*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);
					TransparentBlt(hdc, r.left+(r.right-r.left)/7*3.5, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)minute,10).rem*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);


				if (show||!blink)	TransparentBlt(hdc, r.left+(r.right-r.left)/7*4.5, r.top, ((r.right-r.left)/14), r.bottom-r.top, hdcMem, 10*(int)(bitmap.bmWidth/10.5), 0, (int)(bitmap.bmWidth/21), bitmap.bmHeight, 0xff00ff);
				

					TransparentBlt(hdc, r.left+(r.right-r.left)/7*5, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)second,10).quot*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);
					TransparentBlt(hdc, r.left+(r.right-r.left)/7*6, r.top, ((r.right-r.left)/7), r.bottom-r.top, hdcMem, 0+(div((int)second,10).rem*(int)(bitmap.bmWidth/10.5)), 0, (int)(bitmap.bmWidth/10.5), bitmap.bmHeight, 0xff00ff);

					SelectObject(hdcMem, old);
					DeleteObject(old);
					DeleteObject(image);
					DeleteDC(hdcMem);
				}
	else drawClock(hdc, r, 1);
//--------------------------------------------
		    break;

	}
}
void drawNumber(HDC &hdc, int number, int x, int y, int vlength, int hlength)
{
	
	switch (number)
	{
	case 1:{
				//drawPart(hdc, x, y, vlength, true);
				//drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
				//drawPart(hdc, x, y + vlength, vlength, true);
				//drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				//drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 2:{
				//drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
				drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				//drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 3:{
				//drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
			//	drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 4:{
				drawPart(hdc, x, y, vlength, true);
				//drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
				//drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				//drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 5:{
				drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
			//	drawPart(hdc, x + hlength, y, vlength, true);
			//	drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 6:{
				drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				//drawPart(hdc, x + hlength, y, vlength, true);
				drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 7:{
			//	drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x+hlength, y, vlength, true);
			//	drawPart(hdc, x, y+vlength, vlength, true);
			//	drawPart(hdc, x, y+vlength, hlength, false);
				drawPart(hdc, x +hlength, y+vlength, vlength, true);
			//	drawPart(hdc, x, y+vlength+vlength, hlength, false);
				break;
		   }
	case 8:{
				drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
				drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 9:{
				drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
			//	drawPart(hdc, x, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 0:{
			    drawPart(hdc, x, y, vlength, true);
				drawPart(hdc, x, y, hlength, false);
				drawPart(hdc, x + hlength, y, vlength, true);
				drawPart(hdc, x, y + vlength, vlength, true);
			//	drawPart(hdc, x, y + vlength, hlength, false);
				drawPart(hdc, x + hlength, y + vlength, vlength, true);
				drawPart(hdc, x, y + vlength + vlength, hlength, false);
				break;
		   }
	case 10:{
				drawPart(hdc, x,y+ vlength/2,3,false);
				drawPart(hdc, x,y+ vlength/2+vlength,3,false);
				
				break;
		   }
	case 11:{
				drawPart(hdc, x + hlength/2,y,3,false);
				drawPart(hdc, x + hlength/2 + hlength + 3,y,3,false);
				
				break;
		   }
	/*case 12:{//a
				drawPart(hdc, x,y+ vlength/2,3,false);
				drawPart(hdc, x,y+ vlength/2+vlength,3,false);
				
				break;
		   }
	case 13:{//m
				drawPart(hdc, x,y+ vlength/2,3,false);
				drawPart(hdc, x,y+ vlength/2+vlength,3,false);
				
				break;
		   }*/
	}
}


 void drawPart(HDC  &hdc, int x, int y, int length, bool dir)
{
	
	HPEN hPen;
    hPen = CreatePen(PS_SOLID,1,fontColor);
	SelectObject(hdc,hPen);
  
	if (dir)
	{
	MoveToEx(hdc,x,y,NULL);
	LineTo(hdc,x+1,y+1);
	LineTo(hdc,x+1,y+length-1);
	LineTo(hdc,x,y+length);
	LineTo(hdc,x-1,y+length-1);
	LineTo(hdc,x-1,y+1);
	LineTo(hdc,x,y);
	}
	else
	{
	MoveToEx(hdc,x,y,NULL);
	LineTo(hdc,x+1,y-1);
	LineTo(hdc,x+length-1,y-1);
	LineTo(hdc,x+length,y);
	LineTo(hdc,x+length-1,y+1);
	LineTo(hdc,x+1,y+1);
	LineTo(hdc,x,y);
	}

	SelectObject(hdc,hPen);
	DeleteObject(hPen);
}

void executeAlarm()
{
	sprintf(htime,"%.2d.%.2d:",rhour,minute);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));
	
	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);

/*
	sprintf(htime,"N%d.%.2d:",div(rhour,10).rem,minute);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);


	sprintf(htime,"%dN.%.2d:",div(rhour,10).quot,minute);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);



	sprintf(htime,"%.2d.N%d:",rhour,div(minute,10).rem);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);


	sprintf(htime,"%.2d.%dN:",rhour,div(minute,10).quot);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);

*/
	sprintf(htime,"%.2d.NN:",rhour);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);

/*
	sprintf(htime,"%NN.%.2d:",minute);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);
*/

}

void createMenu()
{
//	bool tempBool = false;

	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
		
			//Now we define all menus and submenus
			
			otherSubmenu = MakeMenu("Other");

			MakeMenuItem(otherSubmenu, "Draw Border", "@BBDigitalExDrawBorder", drawBorder);
	     	MakeMenuItem(otherSubmenu, "Blink :", "@BBDigitalExBlink", blink);
			MakeMenuItem(otherSubmenu, "Show Seconds", "@BBDigitalExShowSeconds", showSeconds);
			MakeMenuItem(otherSubmenu, "24h Format", "@BBDigitalEx24", h24format);
			MakeMenuItem(otherSubmenu, "Remove First 0", "@BBDigitalExRemove0", remove_zero);
			MakeMenuItem(otherSubmenu, "Enable Alarms", "@BBDigitalExEnableAlarms", alarms);
			MakeMenuItemInt(otherSubmenu, "Width", "@BBDigitalExWidth", width, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Height", "@BBDigitalExHeight", height, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Font Size", "@BBDigitalExFontSize", fontSize, 6, 50);
			MakeMenuItemString(otherSubmenu, "Text Format", "@BBDigitalExClockFormat", clockformat);
	

			windowStyleSubmenu = MakeMenu("Style");
			//MakeMenuNOP(windowStyleSubmenu, "___________________");
			//if(StrStrI(windowStyle, "toolbar") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar:", "@BBDigitalExStyleToolbar", (StrStrI(windowStyle, "toolbar") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "buttonnp") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button:", "@BBDigitalExStyleButton", (StrStrI(windowStyle, "buttonnp") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "buttonpr") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", "@BBDigitalExStyleButtonPr", (StrStrI(windowStyle, "buttonpr") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.label:", "@BBDigitalExStyleLabel", (StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6));
			//tempBool = false;
			//if(StrStrI(windowStyle, "windowlabel") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", "@BBDigitalExStyleWindowLabel", (StrStrI(windowStyle, "windowlabel") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "clock") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", "@BBDigitalExStyleClock", (StrStrI(windowStyle, "clock") != NULL));
					
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBDigitalExSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBDigitalExPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBDigitalExOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBDigitalExTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBDigitalExSetTransparent",alpha,0,255);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBDigitalExFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBDigitalExSnapToEdge", snapWindow);

		//	browseSubmenu = MakeMenu("Browse");
		//	MakeMenuItem(browseSubmenu, "Browse...", "@BBDigitalExLoadBitmap", false);

			bitmapSubmenu = MakeMenu("Bitmap");
		//	MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
			MakeMenuNOP(bitmapSubmenu, "Background Bitmap");
			MakeMenuItem(bitmapSubmenu, "Browse...", "@BBDigitalExLoadBitmap", false);
			MakeMenuItem(bitmapSubmenu, "Nothing", "@BBDigitalExNoBitmap", (strcmp(bitmapFile,".none")==0));
			MakeMenuNOP(bitmapSubmenu, "Numbers Bitmap");
			MakeMenuItem(bitmapSubmenu, "Browse...", "@BBDigitalExLoadCBitmap", false);
			MakeMenuItem(bitmapSubmenu, "Nothing", "@BBDigitalExNoCBitmap", (strcmp(clockbitmapFile,".none")==0));

			modeSubmenu = MakeMenu("Draw Mode");
		//	tempBool = false;
		//	if (drawMode == 0) tempBool = true;
			MakeMenuItem(modeSubmenu, "Vertical", "@BBDigitalExMode 0", (drawMode == 0));
			MakeMenuItem(modeSubmenu, "Horizontal", "@BBDigitalExMode 1", (drawMode == 1));
			MakeMenuItem(modeSubmenu, "Just Text", "@BBDigitalExMode 2", (drawMode == 2));
			MakeMenuItem(modeSubmenu, "Bitmap", "@BBDigitalExMode 3", (drawMode == 3));

			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Alarms", "@BBDigitalExEditAlarmsRC", false);
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBDigitalExEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBDigitalExReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBDigitalExSaveSettings", false);
			
			dateSubmenu = MakeMenu("Date Position");
			//tempBool = false;
			//if (text_pos == 0) tempBool = true;
			MakeMenuItem(dateSubmenu, "No Date", "@BBDigitalExDatePossition 0", (text_pos == 0));
			//tempBool = false;
			//if (text_pos == 1) tempBool = true;
			MakeMenuItem(dateSubmenu, "Top", "@BBDigitalExDatePossition 1", (text_pos == 1));
			//tempBool = false;
			//if (text_pos == 2) tempBool = true;
			MakeMenuItem(dateSubmenu, "Bottom", "@BBDigitalExDatePossition 2", (text_pos == 2));
			//tempBool = false;
			//if (text_pos == 3) tempBool = true;
			MakeMenuItem(dateSubmenu, "Left", "@BBDigitalExDatePossition 3", (text_pos == 3));
			//tempBool = false;
			//if (text_pos == 4) tempBool = true;
			MakeMenuItem(dateSubmenu, "Right", "@BBDigitalExDatePossition 4", (text_pos == 4));
			
			//attach defined menus together
			myMenu = MakeMenu("BBDigitalEx 1.0");
			
			MakeSubmenu(configSubmenu, modeSubmenu, "Draw Mode");
			MakeSubmenu(configSubmenu, dateSubmenu, "Date");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, otherSubmenu, "Other");
			MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
			
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About", "@BBDigitalExAbout", false);
			ShowMenu(myMenu);
}

// the end ....