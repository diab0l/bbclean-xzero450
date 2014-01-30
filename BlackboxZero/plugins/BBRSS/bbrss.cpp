/*
 ============================================================================
 Blackbox for Windows: Plugin BBRSS 1.0 by Miroslav Petrasko [Theo]
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

#include "bbrss.h"
#include "resource.h"

LPSTR szAppName = "BBRSS";		// The name of our window class, etc.
LPSTR szVersion = "BBRSS v1.0";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-10-22";
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
	strcpy(mes,"");
    getRssFeed();
//	for(int i=1;i<10;i++) part[i]=0;
	
//	executeAlarm();
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBRSS();
	GetStyleSettings();
//	getCurrentDate();
//	drawline = 11;

	// Create the window...
	hwndBBRSS = CreateWindowEx(
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
	if (!hwndBBRSS)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	//Start the plugin timer
	
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBRSS);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBRSS, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBRSS, GWL_USERDATA, magicDWord);
	// Make the window AlwaysOnTop?
	setPos();
	//if(alwaysOnTop) SetWindowPos(hwndBBRSS, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBRSS, SW_SHOW);
    drawline = -1;
	InvalidateRect(hwndBBRSS, NULL, false);
	mySetTimer();
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBRSSReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBRSS()
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
	KillTimer(hwndBBRSS, IDT_TIMER);
	KillTimer(hwndBBRSS, IDT_MOUSETIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...


	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBRSS, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBRSS);
	// Destroy our window...
	DestroyWindow(hwndBBRSS);
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

		if ((drawline>-1)&&(drawline<10)){
		
		
		showrect = r;
		showrect.top = r.top + (drawline) * 13;
		showrect.bottom = showrect.top + 13;

		MakeGradient(hdc, showrect, myStyleItem2->type,
							backColor2, backColorTo2,
							myStyleItem2->interlaced,
							myStyleItem2->bevelstyle,
							myStyleItem2->bevelposition,
							bevelWidth, borderColor,
							0);

			SetTimer(hwndBBRSS,		// handle to main window 
				IDT_MOUSETIMER,			// timer identifier 
				500,				// second interval 
				(TIMERPROC) NULL);

				}
				

		HGDIOBJ otherfont = CreateFont(13, 
					0, 0,  0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
					"Tahoma");
				//		fontFace);
				
				SelectObject(hdc, otherfont);
				SetTextColor(hdc,fontColor);
	    
	

					DrawText(hdc,mes,-1,&r,DT_TOP | DT_LEFT);
		

		

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
			InvalidateRect(hwndBBRSS, NULL, true);
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
				ShowWindow( hwndBBRSS, SW_SHOW);
				InvalidateRect( hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBRSS, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBRSSAbout"))
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
			else if (!_stricmp(szTemp, "@BBRSSPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBRSSOnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;
				setPos();
				//if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBRSS, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				//else SetWindowPos(hwndBBRSS, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBRSSOnBottom"))
			{
				// Always on top...
				alwaysOnBottom = !alwaysOnBottom;
				setPos();
				//if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBRSS, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				//else SetWindowPos(hwndBBRSS, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBRSSSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBRSS);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBRSS);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBRSSFullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBRSSSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBRSSDrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSDrawMode"))
			{
				drawMode = !drawMode;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSS24"))
			{
				h24format = !h24format;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSEnableAlarms"))
			{
				alarms = !alarms;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSRemove0"))
			{
				remove_zero = !remove_zero;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSReset"))
			{
					for(int i=1;i<10;i++) part[i]=0;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSNoBitmap"))
			{
				noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBRSSEditAlarmsRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, alarmpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBRSSReloadSettings"))
			{
				
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBRSS);

					//Re-initialize
					ReadRCSettings();
					InitBBRSS();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();
					setPos();
					//set window on top is alwaysontop: is true
				//	if ( alwaysOnTop) SetWindowPos( hwndBBRSS, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				//	else SetWindowPos( hwndBBRSS, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBRSS);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBRSS, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBRSSSaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBRSSMode", 15))
			{
				drawMode = atoi(szTemp + 16);
				
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSFontSize", 19))
			{
				fontSize = atoi(szTemp + 20);
				
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSFeed", 9))
			{
				strcpy(rssfeed,szTemp + 11);
				getRssFeed();
				
				InvalidateRect(hwndBBRSS, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBRSSWidth", 10))
			{ //changing the clock size
				width = atoi(szTemp + 11);
				setPos();
			//	if ( alwaysOnTop) SetWindowPos( hwndBBRSS, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
			//else SetWindowPos( hwndBBRSS, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSHeight", 11))
			{ //changing the clock size
				height = atoi(szTemp + 12);
									
			//	if ( alwaysOnTop) SetWindowPos( hwndBBRSS, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
			//	else SetWindowPos( hwndBBRSS, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				setPos();
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				if (transparency) setStatus();					
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSSetBitmap", 20))
			{
				
				strcpy(bitmapFile,szTemp + 22);
				noBitmap = false;
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBRSSMessage", 18))
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
			else if (!_strnicmp(szTemp, "@BBRSSPlayWav", 18))
			{
				strcpy(szTemp,szTemp + 20);
				PlaySound(szTemp, NULL, SND_FILENAME | SND_ASYNC);
							
			}

			else if (!_strnicmp(szTemp, "@BBRSSFav", 8))
			{
				strcpy(rssfeed,szTemp + 10);
				getRssFeed();
				InvalidateRect(hwndBBRSS, NULL, false);
				
				//PlaySound(szTemp, NULL, SND_FILENAME | SND_ASYNC);
							
			}

			else if (!_strnicmp(szTemp, "@BBRSSDatePossition", 24))
			{
				text_pos = atoi(szTemp + 25);
			
				InvalidateRect(hwndBBRSS, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBRSSLoadBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBRSS;
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
			if (GetOpenFileName(&ofn)) noBitmap = false;

			InvalidateRect(hwndBBRSS, NULL, true);

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

	/*	case WM_DISPLAYCHANGE:
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
				MoveWindow(hwndBBRSS, xpos, ypos, width, height, true);
			}
		}
		break;*/

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
			//	InvalidateRect(hwndBBRSS, NULL, false);
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
	//	readMessages();
	//	InvalidateRect(hwndBBRSS, NULL, false);
		//PostMessage(hwndBlackbox, BB_COMMAND, (WPARAM)li[1], 0);
		
		POINT pos;
		GetCursorPos(&pos);	
		RECT windowRect;
		GetWindowRect(hwndBBRSS,&windowRect);
		pos.x -= windowRect.left;
		pos.y -= windowRect.top;
		
	    BBExecute(GetDesktopWindow(), "open", li[pos.y/13], NULL, NULL, SW_SHOWNORMAL, false);
	
		/*for (int i=1;i<10;i++)
			if	(PtInRect(&plane[i],pos)) {part[i]=2;InvalidateRect(hwndBBRSS, NULL, false);}
		
			if (getResult(2)) {MessageBox(hwndBBRSS,"O Wins","Result",MB_OK | MB_ICONERROR | MB_TOPMOST);for(int i=1;i<10;i++) part[i]=0; InvalidateRect(hwndBBRSS, NULL, false);}
*/
	
		
		}
		break;

		case WM_RBUTTONUP: 
		{
			
	/*	POINT pos;
		GetCursorPos(&pos);	
		RECT windowRect;
		GetWindowRect(hwndBBRSS,&windowRect);
		pos.x -= windowRect.left;
		pos.y -= windowRect.top;
		for (int i=1;i<10;i++)
			if	(PtInRect(&plane[i],pos)) {part[i]=2;InvalidateRect(hwndBBRSS, NULL, false);}
		
			if (getResult(2)) {MessageBox(hwndBBRSS,"O Wins","Result",MB_OK | MB_ICONERROR | MB_TOPMOST);for(int i=1;i<10;i++) part[i]=0; InvalidateRect(hwndBBRSS, NULL, false);}

	*/		
			//open control panel with:  control timedate.cpl,system,0
	//		BBExecute(GetDesktopWindow(), NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
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
					//show = !show;
					//getCurrentDate();
					getRssFeed();
					drawline = -1;
					InvalidateRect(hwndBBRSS, NULL, false);
					
				//	if ((second == 0)&&(show)&&(alarms)) executeAlarm();
					
				} break;

				case IDT_MOUSETIMER:
				{
					drawline = -1;
					InvalidateRect(hwndBBRSS, NULL, false);
					KillTimer(hwndBBRSS, IDT_MOUSETIMER);
				} break; 
			}
		}
		break;

		case WM_MOUSEMOVE:
			{
					POINT pos;
					GetCursorPos(&pos);	
					RECT windowRect;


					GetWindowRect(hwndBBRSS,&windowRect);
					
					if	(PtInRect(&windowRect,pos)){
					pos.x -= windowRect.left;
					pos.y -= windowRect.top;		

					drawline = pos.y/13;} else drawline = -1;
		
					InvalidateRect(hwndBBRSS, NULL, false);
			}break;

/*		case WM_NCMOUSELEAVE:
			{
			
			drawline = -1;
			InvalidateRect(hwndBBRSS, NULL, false);

			}break;*/

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
	strcpy(npath, rcpath);

	strcat(alarmpath,"alarms.rc");
	strcat(npath,"ns.rc");
	if (!FileExists(alarmpath)) createAlarmFile();
	
	strcat(temp, "bbrss.rc");
	strcat(path, "bbrssrc");
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
			
		strcat(temp, "bbrss.rc");
		strcat(path, "bbrssrc");
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
			alwaysOnBottom = false;
			snapWindow = true;
			transparency = false;
			fullTrans = false;
			drawBorder = false;
			pluginToggle = false;
			fontSize = 6;
			strcpy(windowStyle, "windowlabel");
			strcpy(rssfeed, "http://rss.syntechsoftware.com/nforce-apps.xml");
			strcpy(bitmapFile, ".none");
			strcpy(clockformat, "%d %a %#H:%M");
			noBitmap = true;
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
	xpos = ReadInt(rcpath, "bbrss.x:", 10);
	ypos = ReadInt(rcpath, "bbrss.y:", 10);

	width = ReadInt(rcpath, "bbrss.width:", 100);
	height = ReadInt(rcpath, "bbrss.height:", 50);

	text_pos = ReadInt(rcpath, "bbrss.date.possition:", 0);
	if ((text_pos<0) ||(text_pos>4)) text_pos = 0; 

	alpha = ReadInt(rcpath, "bbrss.alpha:", 160);
	if(alpha > 255) alpha = 255;
	
	if(ReadString(rcpath, "bbrss.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbrss.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "bbrss.alwaysOnTop:", true);
	alwaysOnBottom = ReadBool(rcpath, "bbrss.alwaysOnBottom:", false);
	
	drawBorder = ReadBool(rcpath, "bbrss.drawBorder:", true);
	drawMode = ReadInt(rcpath, "bbrss.drawMode:", HORIZONTAL);
	remove_zero = ReadBool(rcpath, "bbrss.remove.0:", false);
	h24format = ReadBool(rcpath, "bbrss.24hFormat:", false);
	showSeconds = ReadBool(rcpath, "bbrss.show.seconds:", true);
	alarms = ReadBool(rcpath, "bbrss.enabled.alarms:", false);
//	if (magStart) mag = true;

	snapWindow = ReadBool(rcpath, "bbrss.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbrss.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbrss.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbrss.fontSize:", 6);
//	alwaysOnTop = ReadBool(rcpath, "bbrss.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbrss.pluginToggle:", false);
	strcpy(windowStyle, ReadString(rcpath, "bbrss.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");
	strcpy(bitmapFile, ReadString(rcpath, "bbrss.bitmapFile:", ".none"));
	strcpy(rssfeed, ReadString(rcpath, "bbrss.rssfeed:", "http://rss.syntechsoftware.com/nforce-apps.xml"));
	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 
	
	strcpy(clockformat, ReadString(rcpath, "bbrss.clockformat:", "%d %a %#H:%M"));

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

		sprintf(szTemp, "! BBRSS %s alarms file.\r\n",szInfoVersion);
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

		sprintf(szTemp, "! BBRSS %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbrss.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbrss.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbrss.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbrss.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	sprintf(szTemp, "bbrss.date.possition: %d\r\n", text_pos, temp);
	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbrss.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnBottom) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.alwaysOnBottom: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbrss.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbrss.drawBorder: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	sprintf(szTemp, "bbrss.drawMode: %d\r\n", drawMode, temp);
	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	(remove_zero) ? strcpy(temp, "true") : strcpy(temp, "false");
	//	sprintf(szTemp, "bbrss.remove.0: %s\r\n", temp);
 	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	(showSeconds) ? strcpy(temp, "true") : strcpy(temp, "false");
	//	sprintf(szTemp, "bbrss.show.seconds: %s\r\n", temp);
 	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	(h24format) ? strcpy(temp, "true") : strcpy(temp, "false");
	//	sprintf(szTemp, "bbrss.24hFormat: %s\r\n", temp);
 	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	//	(alarms) ? strcpy(temp, "true") : strcpy(temp, "false");
	//	sprintf(szTemp, "bbrss.enabled.alarms: %s\r\n", temp);
 	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

    	sprintf(szTemp, "bbrss.fontSize: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
	//	sprintf(szTemp, "bbrss.clockformat: %s\r\n", clockformat);
	//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbrss.bitmapFile: %s\r\n", bitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbrss.rssfeed: %s\r\n", rssfeed);
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
		SetTimer(hwndBBRSS,		// handle to main window 
				IDT_TIMER,			// timer identifier 
				300000,				// 5min interval 
				(TIMERPROC) NULL);	// no timer callback 
		
/*		SetTimer(hwndBBRSS,		// handle to main window 
				IDT_MOUSETIMER,			// timer identifier 
				1000,				// second interval 
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
								SetWindowLong(hwndBBRSS, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBRSS, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBRSS, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBRSS, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBRSS, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBRSS, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBRSS, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBRSS, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBRSS, NULL, false);		
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

/*void drawClock(HDC &hdc, RECT r, int mode)
{
/*	switch (mode)
	{
			
	case  HORIZONTAL:
			if (showSeconds)
				
			{
				vl = (r.bottom - r.top)/2;
				hl = ((r.right - r.left)-42)/6;

				if (!((remove_zero) && (div((int)hour,10).quot == 0)))	drawNumber(hdc, div((int)hour,10).quot, r.left +2, r.top, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + hl + 8, r.top, vl, hl);

				if (show)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2 - 9 - hl, r.top, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left + (r.right - r.left)/2 - 3 - hl, r.top, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left + (r.right - r.left)/2 +3, r.top, vl, hl);
				
				if (show)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2 + 7 + hl , r.top, vl, hl);
				
				drawNumber(hdc, div((int)second,10).quot, r.right - 2*hl - 8, r.top, vl, hl);
				drawNumber(hdc, div((int)second,10).rem, r.right - 2 - hl, r.top, vl, hl);
			}
			else
			{
				vl = (r.bottom - r.top)/2;
				hl = ((r.right - r.left)-30)/4;

				if (!((remove_zero) && (div((int)hour,10).quot == 0)))	drawNumber(hdc, div((int)hour,10).quot, r.left +2, r.top, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + hl + 8, r.top, vl, hl);

				if (show)	drawNumber(hdc, 10, r.left + (r.right - r.left)/2, r.top, vl, hl);
				
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

				if (show)	drawNumber(hdc, 11, r.left + 2 , r.top + (r.bottom - r.top)/2 - vl-5, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left + 2, r.top + (r.bottom - r.top)/2 - vl, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left+ 8 + hl, r.top + (r.bottom - r.top)/2 - vl, vl, hl);
				
				if (show)	drawNumber(hdc, 11, r.left + 2, r.top + (r.bottom - r.top)/2 + vl+5, vl, hl);
				
				drawNumber(hdc, div((int)second,10).quot, r.left+2, r.bottom - 2 - 2*vl, vl, hl);
				drawNumber(hdc, div((int)second,10).rem, r.left+ 8 + hl, r.bottom - 2 - 2*vl, vl, hl);
			}
			else
			{
				hl = ((r.right - r.left)-9)/2;
				vl = (r.bottom - r.top-15)/4;

				drawNumber(hdc, div((int)hour,10).quot, r.left + 2, r.top + 2, vl, hl);
				drawNumber(hdc, div((int)hour,10).rem, r.left + 8 + hl, r.top + 2  , vl, hl);

				if (show)	drawNumber(hdc, 11, r.left + 2 , r.top + (r.bottom - r.top)/2, vl, hl);
				
				drawNumber(hdc, div((int)minute,10).quot, r.left+2, r.bottom - 2 - 2*vl, vl, hl);
				drawNumber(hdc, div((int)minute,10).rem, r.left+ 8 + hl, r.bottom - 2 - 2*vl, vl, hl);
			
			}
			break;

	case JUSTTEXT:
		    break;

	}
}
/*void drawNumber(HDC &hdc, int number, int x, int y, int vlength, int hlength)
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
/*	}
}
*/

 void drawPart(HDC  &hdc, RECT dr, BOOL type)
{
	conRect(dr,5);
	HPEN hPen;
//	HBRUSH hBrush;
    hPen = CreatePen(PS_SOLID,3,type?0x0000ff:0xff0000);
//	hBrush = CreateSolidBrush(0xffffff);
	SelectObject(hdc,hPen);
//	SelectObject(hdc,hBrush);
if (type)
{	MoveToEx(hdc,dr.left,dr.top,NULL);
	LineTo(hdc,dr.right,dr.bottom);

	MoveToEx(hdc,dr.left,dr.bottom,NULL);
	LineTo(hdc,dr.right,dr.top);
}
else

    Arc(hdc,dr.left,dr.top,dr.right,dr.bottom,dr.left + (dr.right - dr.left)/2,dr.top,dr.left + (dr.right - dr.left)/2,dr.top);

//	SelectObject(hdc,hPen);
	DeleteObject(hPen);
//	DeleteObject(hBrush);
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

			MakeMenuItem(otherSubmenu, "Draw Border", "@BBRSSDrawBorder", drawBorder);
	    //	MakeMenuItem(otherSubmenu, "Draw Mode", "@BBRSSDrawMode", drawMode);
		//	MakeMenuItem(otherSubmenu, "Show Seconds", "@BBRSSShowSeconds", showSeconds);
		//	MakeMenuItem(otherSubmenu, "24h Format", "@BBRSS24", h24format);
		//	MakeMenuItem(otherSubmenu, "Remove First 0", "@BBRSSRemove0", remove_zero);
		//	MakeMenuItem(otherSubmenu, "Enable Alarms", "@BBRSSEnableAlarms", alarms);
			MakeMenuItemInt(otherSubmenu, "Width", "@BBRSSWidth", width, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Height", "@BBRSSHeight", height, 20, 400);
		//	MakeMenuItemInt(otherSubmenu, "Font Size", "@BBRSSFontSize", fontSize, 6, 50);
			MakeMenuItemString(otherSubmenu, "RSS Feed", "@BBRSSFeed", rssfeed);
	

			windowStyleSubmenu = MakeMenu("Style");
			//MakeMenuNOP(windowStyleSubmenu, "___________________");
			//if(StrStrI(windowStyle, "toolbar") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar:", "@BBRSSStyleToolbar", (StrStrI(windowStyle, "toolbar") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "buttonnp") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button:", "@BBRSSStyleButton", (StrStrI(windowStyle, "buttonnp") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "buttonpr") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", "@BBRSSStyleButtonPr", (StrStrI(windowStyle, "buttonpr") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.label:", "@BBRSSStyleLabel", (StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6));
			//tempBool = false;
			//if(StrStrI(windowStyle, "windowlabel") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", "@BBRSSStyleWindowLabel", (StrStrI(windowStyle, "windowlabel") != NULL));
			//tempBool = false;
			//if(StrStrI(windowStyle, "clock") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", "@BBRSSStyleClock", (StrStrI(windowStyle, "clock") != NULL));
					
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBRSSSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBRSSPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBRSSOnTop", alwaysOnTop);
			MakeMenuItem(generalConfigSubmenu, "Always on bottom", "@BBRSSOnBottom", alwaysOnBottom);
		
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBRSSTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBRSSSetTransparent",alpha,0,255);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBRSSFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBRSSSnapToEdge", snapWindow);

			browseSubmenu = MakeMenu("Browse");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBRSSLoadBitmap", false);

			bitmapSubmenu = MakeMenu("Bitmap");
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
			MakeMenuItem(bitmapSubmenu, "Nothing", "@BBRSSNoBitmap", noBitmap);

		//	modeSubmenu = MakeMenu("Draw Mode");
		//	tempBool = false;
		//	if (drawMode == 0) tempBool = true;
		//	MakeMenuItem(modeSubmenu, "Vertical", "@BBRSSMode 0", (drawMode == 0));
		//	MakeMenuItem(modeSubmenu, "Horizontal", "@BBRSSMode 1", (drawMode == 1));
		//	MakeMenuItem(modeSubmenu, "Just Text", "@BBRSSMode 2", (drawMode == 2));

			settingsSubmenu = MakeMenu("Settings");
		//	MakeMenuItem(settingsSubmenu, "Edit Alarms", "@BBRSSEditAlarmsRC", false);
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBRSSEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBRSSReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBRSSSaveSettings", false);
			
			favSubmenu = MakeMenu("Favorites");
			//tempBool = false;
			//if (text_pos == 0) tempBool = true;
			
			char fav[MAX_LINE_LENGTH];
			char lin[MAX_LINE_LENGTH];
			char bro[MAX_LINE_LENGTH];

		for (int i=0;i<10;i++){
	//	int i=1;
			strcpy(lin, "bbrss.fav");
			char num[3];
			itoa(i,num,10);
			strcat(lin, num);
			strcat(lin, ":");
			strcpy(fav, ReadString(alarmpath, lin, "sss"));
			strcpy(bro, "@BBRSSFav ");
			strcat(bro, fav);
			
			
			MakeMenuItem(favSubmenu, fav, bro, false);
			}
			
			//tempBool = false;
			//if (text_pos == 1) tempBool = true;
	/*		MakeMenuItem(dateSubmenu, "Top", "@BBRSSDatePossition 1", (text_pos == 1));
			//tempBool = false;
			//if (text_pos == 2) tempBool = true;
			MakeMenuItem(dateSubmenu, "Bottom", "@BBRSSDatePossition 2", (text_pos == 2));
			//tempBool = false;
			//if (text_pos == 3) tempBool = true;
			MakeMenuItem(dateSubmenu, "Left", "@BBRSSDatePossition 3", (text_pos == 3));
			//tempBool = false;
			//if (text_pos == 4) tempBool = true;
			MakeMenuItem(dateSubmenu, "Right", "@BBRSSDatePossition 4", (text_pos == 4));
		*/	
			//attach defined menus together
			myMenu = MakeMenu("BBRSS 1.0");
			
		//	MakeSubmenu(configSubmenu, modeSubmenu, "Draw Mode");
	//		MakeSubmenu(configSubmenu, fSubmenu, "Date");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, otherSubmenu, "Other");
			MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
			
	//		MakeMenuItem(myMenu, "Reset", "@BBRSSReset", false);
			MakeSubmenu(myMenu, favSubmenu, "Favorites");
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About", "@BBRSSAbout", false);
			ShowMenu(myMenu);
}

void conRect(RECT &recttc, int con)
{
	recttc.left += con;
	recttc.right -= con;
	recttc.top +=con;
	recttc.bottom -=con;
}

int getResult(int who)
{
	if (part[5] == who)
		if ((part[1] == who && part[9] == who) ||
			(part[4] == who && part[6] == who) ||
			(part[7] == who && part[3] == who) ||
			(part[2] == who && part[8] == who))
			return 1;

	if (part[1] == who)
		if ((part[4] == who && part[7] == who) ||
			(part[2] == who && part[3] == who))
			return 1;

	if (part[9] == who)
		if ((part[7] == who && part[8] == who) ||
			(part[3] == who && part[6] == who))
			return 1;

	BOOL draw=true;
		for(int i=1;i<10;i++) if (part[i]==0) draw=false;

	if (draw) return 2; 

	
	return 0;
}

int getMove()
{


		/*Checking whether winning or not*/
		for (int i = 1; i < 10; i++)
				if (part[i]==0)
				{
					part[i] = 2;

					/*display game result set exit flag and return to 
											 keyboard handler freom which control will return to 
											 constructor which will display continue option and will 
											 take accordingly decision*/

					switch (getResult(2))
					{
					case 1:
						part[i] = 0;
						return i;

					case 2:
						part[i] = 0;
						return i;
			
					case 0:
						/*if nothing happened try another cell*/
					part[i] = 0;
						break;
					}
				}



		/*trying to block user by using Player A's symbol and calculating result if user A is winning 
										   then put computer symbol in that place*/

		for (i = 1; i < 10; i++)
		{
				if (part[i]==0)/*if cell is empty*/
				{
					part[i] = 1;


					switch (getResult(1))
					{
					case 1:
						part[i] = 0;
						return i;

					case 2:

						part[i] = 0;
						return i;
					case 0:
						part[i] = 0;/*if nothing happened then reset cell and try another one*/
						break;
					}
				}			
		}


		/*if not able to block take a random move*/
		BOOL flag=false;
		
		int ra=1;

		if (!(getResult(1)==2)){
		while (!flag)
			{
				srand(GetTickCount());
	    		ra = (int)rand();
			    ra=(int)((ra/32767.0)*9.0)+1;

				if (part[ra]==0)
				{
					part[ra] = 0;
					flag = !flag;
				}
			}return ra;
		} return 0;




}

void getRssFeed()
{
	

	if ( URLDownloadToFile( NULL, rssfeed, npath,0, NULL ) != S_OK )
	{
		MessageBox(hwndBlackbox, "Error downloading file", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return;
	}
	readMessages();
}

void readMessages()
{
	static char szTemp[MAX_LINE_LENGTH];
	static char temp[8];
int c=0;
 char * pch;

  pch = strtok (read_file_into_buffer (npath),"\n\r");
  strcpy(mes,"");
  while ((pch != NULL))
  {
	  if (strstr(pch, "<item>")) {c++;}
	  if (strstr(pch, "<title>"))	{pch[(int)strstr(pch, "<\57title>")-(int)pch]=0;
					strncpy(szTemp,strstr(pch, "<title>")+7,50);
				//	strcat(mes, szTemp);
					if (c<10) strcpy(me[c],szTemp);
					
				//	strlen(strstr(pch, "<title>")+7)>50?strcat(mes,"...\n"):strcat(mes,"\n");
	  }  
	  if (strstr(pch, "<link>"))	{
		  pch[(int)strstr(pch, "<\57link>")-(int)pch]=0;
		  //char help[20];
		  //strcpy(help,"sss");
		  //itoa((int)strstr(pch, "<\57link>")-(int)pch,help,10);
		  //	MessageBox(hwndBlackbox, help, szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
	
		  //pch[(int)(strstr(pch, "<\57link>"))] = 0;//strlen(pch)-7]=0;
			int w=6;
			while (pch[w-1]==' ') w++;
			if (c<10) strcpy(li[c],strstr(pch, "<link>")+w);
	  }
		
		//pch[strlen(pch)-7]=0;
		//			strncpy(szTemp,strstr(pch, "<link>")+6,50);
				//	strcat(mes, szTemp);
	//				if (c<5) strcpy(li[c],szTemp);
	
	//			//	strlen(strstr(pch, "<title>")+7)>50?strcat(mes,"...\n"):strcat(mes,"\n");

	//  }//MessageBox(hwndBlackbox, strstr(pch, "<title>")+7, szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);}

	  pch = strtok (NULL, "\n\r");
  }
strcpy(mes,"");
for (int i=0;i<10;i++) {strcat(mes,me[i]);strlen(me[i])>=50?strcat(mes,"...\n "):strcat(mes,"\n ");}

	
	//		MessageBox(hwndBlackbox, mes, szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
	

}


char * read_file_into_buffer (const char *fname)
{
	FILE *fp; char *buf; int k;
	if (NULL == (fp=fopen(fname,"rb")))
		return NULL;

	fseek(fp,0,SEEK_END);
	k = ftell (fp);
	fseek (fp,0,SEEK_SET);

	buf=(char*)malloc(k+1);
	fread (buf, 1, k, fp);
	fclose(fp);

	buf[k]=0;
	return buf;
}

void setPos()
{

		if ( alwaysOnBottom) SetWindowPos( hwndBBRSS, hwndBlackbox, xpos, ypos, width, height, SWP_NOACTIVATE | SWP_NOZORDER);
		else if ( alwaysOnTop) SetWindowPos( hwndBBRSS, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
		else SetWindowPos( hwndBBRSS, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);


}

// the end ....