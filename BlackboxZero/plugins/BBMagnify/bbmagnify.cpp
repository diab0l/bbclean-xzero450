/*
 ============================================================================
 Blackbox for Windows: Plugin BBMagnify 1.0 by Miroslav Petrasko [Theo]
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

#include "bbmagnify.h"
#include "resource.h"

LPSTR szAppName = "BBMagnify";		// The name of our window class, etc.
LPSTR szVersion = "BBMagnify v1.0";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-08-04";
LPSTR szInfoLink = "theo.host.sk";
LPSTR szInfoEmail = "theo.devil@gmx.net";

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
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBMagnify();
	GetStyleSettings();
	
	// Create the window...
	hwndBBMagnify = CreateWindowEx(
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
	if (!hwndBBMagnify)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	//Start the plugin timer
	mySetTimer();
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMagnify);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBMagnify, (LPARAM)msgs);
  const long magicDWord = 0x49474541;
#if !defined _WIN64
  // Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
  SetWindowLong(hwndBBMagnify, GWL_USERDATA, magicDWord);
#else
  SetWindowLongPtr(hwndBBMagnify, GWLP_USERDATA, magicDWord);
#endif

	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBMagnify, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBMagnify, SW_SHOW);

	InvalidateRect(hwndBBMagnify, NULL, true);
	
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBMagnifyReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBMagnify()
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
	KillTimer(hwndBBMagnify, IDT_TIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...


	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBMagnify, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMagnify);
	// Destroy our window...
	DestroyWindow(hwndBBMagnify);
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

			HBRUSH hBrush;

			//retrieve the coordinates of the window's client area.
			GetClientRect(hwnd, &r);
			GetClientRect(hwnd, &updateRect);
			updateRect.top = updateRect.bottom - 2*fontSize - 6;
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
	
			
				
									
					
				POINT poloha;
				GetCursorPos(&poloha);
			if (drawColor) r.bottom -= fontSize +3;
					
			if ((drawColor)||(mag))	
			{
				HDC hdcdesk = GetDC(NULL);
				
				
			   if (mag)	StretchBlt(hdc, r.left, r.top, r.right-r.left, r.bottom-r.top, hdcdesk, poloha.x - (r.right-r.left)/(ratio*2), poloha.y - (r.bottom-r.top)/(ratio*2), (r.right-r.left)/ratio, (r.bottom-r.top)/ratio ,SRCCOPY);

				actColor = GetPixel(hdcdesk,poloha.x,poloha.y);
				
				//HPEN hPen;
				//hPen = CreatePen(PS_SOLID,1,0xff0000);
				
				ReleaseDC(NULL,hdcdesk);
				DeleteDC(hdcdesk);
			}
				

			//	SelectObject(hdc, hPen);
	
				
			//	MoveToEx(hdc,r.left+((r.right-r.left)/2),r.top + (r.bottom - r.top)/2,NULL);
			//	LineTo(hdc,);
					
				//RECT rec;
			//	r.bottom += fontSize +3;

			
			if ((drawCross)&&(mag))
			{
				
				hBrush = CreateSolidBrush(0x0000ff);

				rec.left = r.left+((r.right-r.left)/2)-1;
				rec.top = (r.top + (r.bottom - r.top)/2)-1;
				rec.right = rec.left + 2;
				rec.bottom = rec.top + 2;
				
				FillRect(hdc,&rec,hBrush);

				rec.top = rec.top - 10;
				rec.bottom = rec.top + 7;

				FillRect(hdc,&rec,hBrush);
				
				rec.top = rec.top + 15;
				rec.bottom = rec.top + 7;

				FillRect(hdc,&rec,hBrush);

				rec.top = (r.top + (r.bottom - r.top)/2)-1;
				rec.bottom = rec.top + 2;

				rec.left = rec.left - 10;
				rec.right = rec.left + 7;
				
				FillRect(hdc,&rec,hBrush);

				rec.left = rec.left + 15;
				rec.right = rec.left + 7;
				
				FillRect(hdc,&rec,hBrush);
				
			//	SelectObject(hdc, hPen);
				SelectObject(hdc, hBrush);
			//	DeleteObject(hPen);
				DeleteObject(hBrush);

			}	
				
			if (drawColor)
			{
				hBrush = CreateSolidBrush(actColor);

				rec.left = r.left;
				rec.right = r.right;
				rec.bottom = r.bottom + fontSize +3;
				rec.top = rec.bottom - fontSize - 3;
				FillRect(hdc,&rec,hBrush);

			//	SelectObject(hdc, hBrush);

				DeleteObject(hBrush);
			}
			
				
			
				if (!mag){
				// if a bitmap path is found the bitmap is painter
					
					if (!strcmp(bitmapFile,".none")==0) 
						{
					
					HANDLE image;
					image = LoadImage(NULL, bitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					
					HDC hdcMem = CreateCompatibleDC(hdc);
					
					HBITMAP old = (HBITMAP) SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);

	
					TransparentBlt(hdc, r.left, r.top, r.right-r.left, r.bottom-r.top, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
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
				
				}


			if (drawPos || drawColor || copied>0){	
				HGDIOBJ otherfont = CreateFont( fontSize, 
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace);

				SelectObject(hdc, otherfont);
				SetTextColor(hdc,fontColor);
			
			if (copied>0)
				
				DrawText(hdc,"COPIED",-1,&r,DT_TOP | DT_CENTER | DT_SINGLELINE);
				copied--;

			if (drawPos)
				{	
				sprintf(szTemp,"X:%i Y:%i",poloha.x,poloha.y);
				DrawText(hdc,szTemp,-1,&r,DT_BOTTOM | DT_CENTER | DT_SINGLELINE);
				}
			if (drawColor)
			{
				r.bottom = r.bottom + fontSize + 3;
				
				if ((GetRValue(actColor)>130) || (GetGValue(actColor)>130) || (GetBValue(actColor)>130)) SetTextColor(hdc,0x000000);
				else SetTextColor(hdc,0xffffff);

				sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(actColor),GetGValue(actColor),GetBValue(actColor));
				
				DrawText(hdc,_strupr(szTemp),-1,&r,DT_BOTTOM | DT_CENTER | DT_SINGLELINE);	
			}
				DeleteObject(otherfont);
				
				}
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
			InvalidateRect(hwndBBMagnify, NULL, true);
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
				ShowWindow( hwndBBMagnify, SW_SHOW);
				InvalidateRect( hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBMagnify, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyAbout"))
			{
				sprintf(szTemp, "%s\n\n%s ©2004 %s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBBMagnify,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
			}
			else if (!_stricmp(szTemp, "@BBMagnifyPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBMagnifyOnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;

				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBMagnify, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBMagnify, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBMagnifySlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMagnify);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMagnify);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBMagnifyFullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBMagnifySnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBMagnifyDrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyDrawCross"))
			{
				drawCross = !drawCross;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyDrawPossition"))
			{
				drawPos = !drawPos;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyDrawColor"))
			{
				drawColor = !drawColor;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyAtStart"))
			{
				magStart = !magStart;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyNoBitmap"))
			{
				noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyReloadSettings"))
			{
				
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMagnify);

					//Re-initialize
					ReadRCSettings();
					InitBBMagnify();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBMagnify, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBMagnify, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMagnify);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBMagnify, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBMagnifySaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBMagnifyFontSize", 18))
			{
				fontSize = atoi(szTemp + 19);
				
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMagnifyWidth", 15))
			{ //changing the clock size
				width = atoi(szTemp + 16);
				
				if ( alwaysOnTop) SetWindowPos( hwndBBMagnify, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBMagnify, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMagnifyHeight", 16))
			{ //changing the clock size
				height = atoi(szTemp + 17);
									
				if ( alwaysOnTop) SetWindowPos( hwndBBMagnify, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBMagnify, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMagnifySetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				if (transparency) setStatus();					
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMagnifySetBitmap", 18))
			{
				
				strcpy(bitmapFile,szTemp + 20);
				noBitmap = false;
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMagnifyRatio", 15))
			{
				ratio = atoi(szTemp + 16);
			
				InvalidateRect(hwndBBMagnify, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMagnifyLoadBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBMagnify;
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

			InvalidateRect(hwndBBMagnify, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BBMagnifyColorToClipboard"))
			{
				sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(actColor),GetGValue(actColor),GetBValue(actColor));
				
				ColorToClipboard(_strupr(szTemp));

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
				MoveWindow(hwndBBMagnify, xpos, ypos, width, height, true);
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

		case WM_LBUTTONUP: 
		{
				mag = !mag;
				InvalidateRect(hwndBBMagnify, NULL, false);
		}
		break;
		
		// ==========

		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			// Finally, we show the menu...
			createMenu();
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
					if ((mag)||(copied>=0)) InvalidateRect(hwndBBMagnify, NULL, false);
					else InvalidateRect(hwndBBMagnify, &updateRect, false);
				} break;
			}
		}
		break;

		case WM_MOUSEMOVE:
			{
			InvalidateRect(hwndBBMagnify, NULL, false);
			}break;

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
	parentstyle = false;

	
	
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
		parentstyle = true;
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
		else
		{
		parentstyle = true;
		}
	}
	else if(StrStrI(windowStyle, "toolbar") != NULL)
	{
			parentstyle = true;
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
		parentstyle = true;
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
			parentstyle = true;
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
		parentstyle = true;
		}
	}


	if (parentstyle)
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
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
	strcat(temp, "bbmagnify.rc");
	strcat(path, "bbmagnifyrc");
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
		strcat(temp, "bbmagnify.rc");
		strcat(path, "bbmagnifyrc");
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
			noBitmap = true;
			ratio = 1;
			drawCross = true;
			drawPos = true;
			drawColor = true;
			magStart = false;

			WriteRCSettings();
			
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbmagnify.x:", 10);
	ypos = ReadInt(rcpath, "bbmagnify.y:", 10);

	width = ReadInt(rcpath, "bbmagnify.width:", 100);
	height = ReadInt(rcpath, "bbmagnify.height:", 50);

	ratio = ReadInt(rcpath, "bbmagnify.ratio:", 1);
	if (ratio<=0) ratio = 1; 

	alpha = ReadInt(rcpath, "bbmagnify.alpha:", 160);
	if(alpha > 255) alpha = 255;
	
	if(ReadString(rcpath, "bbmagnify.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbmagnify.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "bbmagnify.alwaysOnTop:", true);
	
	drawBorder = ReadBool(rcpath, "bbmagnify.draw.border:", true);
	drawCross = ReadBool(rcpath, "bbmagnify.draw.cross:", true);
	drawPos = ReadBool(rcpath, "bbmagnify.draw.possition:", true);
	drawColor = ReadBool(rcpath, "bbmagnify.draw.color:", true);

	magStart = ReadBool(rcpath, "bbmagnify.magnifyAtStart:", false);
	if (magStart) mag = true;

	snapWindow = ReadBool(rcpath, "bbmagnify.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbmagnify.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbmagnify.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbmagnify.fontSize:", 6);
	alwaysOnTop = ReadBool(rcpath, "bbmagnify.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbmagnify.pluginToggle:", false);
	strcpy(windowStyle, ReadString(rcpath, "bbmagnify.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");
	strcpy(bitmapFile, ReadString(rcpath, "bbmagnify.bitmapFile:", ".none"));
	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 
	
}

//---------------------------------------------------------------------------
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

		sprintf(szTemp, "! BBMagnify %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbmagnify.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbmagnify.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbmagnify.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbmagnify.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbmagnify.ratio: %d\r\n", ratio, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbmagnify.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbmagnify.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.draw.border: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawCross) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.draw.cross: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawPos) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.draw.possition: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawColor) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.draw.color: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(magStart) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbmagnify.magnifyAtStart: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

    	sprintf(szTemp, "bbmagnify.fontSize: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbmagnify.bitmapFile: %s\r\n", bitmapFile);
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
		case PLUGIN_NAME:
			return szAppName;		// Plugin name
		case PLUGIN_VERSION:
			return szInfoVersion;	// Plugin version
		case PLUGIN_AUTHOR:
			return szInfoAuthor;	// Author
		case PLUGIN_RELEASEDATE:
			return szInfoRelDate;	// Release date, preferably in yyyy-mm-dd format
		case PLUGIN_LINK:
			return szInfoLink;		// Link to author's website
		case PLUGIN_EMAIL:
			return szInfoEmail;		// Author's email
		
		case PLUGIN_BROAMS:
		{
			strcpy(szTemp, "@BBMagnifyDrawBorder");
			strcat(szTemp, "@BBMagnifyDrawCross");
			strcat(szTemp, "@BBMagnifyDrawPossition");
			strcat(szTemp, "@BBMagnifyDrawColor");
			strcat(szTemp, "@BBMagnifyAtStart");
			strcat(szTemp, "@BBMagnifyWidth #");
			strcat(szTemp, "@BBMagnifyHeight #");
			strcat(szTemp, "@BBMagnifyFontSize #");
			strcat(szTemp, "@BBMagnifyStyleToolbar");
			strcat(szTemp, "@BBMagnifyStyleButton");
			strcat(szTemp, "@BBMagnifyStyleButtonPr");
			strcat(szTemp, "@BBMagnifyStyleLabel");
			strcat(szTemp, "@BBMagnifyStyleWindowLabel");
			strcat(szTemp, "@BBMagnifyStyleClock");
			strcat(szTemp, "@BBMagnifySlit");
			strcat(szTemp, "@BBMagnifyPluginToggle");
			strcat(szTemp, "@BBMagnifyOnTop");
			strcat(szTemp, "@BBMagnifyTransparent");
			strcat(szTemp, "@BBMagnifySetTransparent #");
			strcat(szTemp, "@BBMagnifyFullTrans");
			strcat(szTemp, "@BBMagnifySnapToEdge");
			strcat(szTemp, "@BBMagnifyLoadBitmap");
			strcat(szTemp, "@BBMagnifySetBitmap #");
			strcat(szTemp, "@BBMagnifyNoBitmap");
			strcat(szTemp, "@BBMagnifyEditRC");
			strcat(szTemp, "@BBMagnifyReloadSettings");
			strcat(szTemp, "@BBMagnifySaveSettings");
			strcat(szTemp, "@BBMagnifyRatio #");
			strcat(szTemp, "@BBMagnifyAbout");
			strcat(szTemp, "@BBMagnifyColorToClipboard");
			return szTemp;

		}
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
	
		//Start the 0.10 second plugin timer
		SetTimer(hwndBBMagnify,		// handle to main window 
				IDT_TIMER,			// timer identifier 
				100,				// second interval 
				(TIMERPROC) NULL);	// no timer callback 
	
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
//--------------------------------------------------------------

int beginPluginEx(HINSTANCE hPluginInstance, HWND hwndBBSlit) 
{ 
 inSlit = true; 
 hSlit = hwndBBSlit; 
 
 return beginPlugin(hPluginInstance); 
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
								SetWindowLong(hwndBBMagnify, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBMagnify, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBMagnify, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBMagnify, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBMagnify, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBMagnify, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBMagnify, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBMagnify, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBMagnify, NULL, false);		
}

//--------------------------------------------------------------------------

void ColorToClipboard(char* color)
{

	GLOBALHANDLE hGlobal;
	char* pGlobal;
	hGlobal = GlobalAlloc (GHND | GMEM_SHARE, 9) ;
	if (hGlobal == NULL)
	{
    MessageBox(hwndBBMagnify, "GlobalAlloc failed", "Error",MB_OK|MB_ICONERROR);
    return;
	};

	pGlobal = (char *) GlobalLock (hGlobal) ;

	if (pGlobal == NULL)
	{
    MessageBox(hwndBBMagnify, "GlobalLock failed", "Error",MB_OK|MB_ICONERROR);
    return;
	};

	for (int i = 0 ; i < 8 ; i++)
     *pGlobal++ = *color++ ;
	GlobalUnlock (hGlobal);


    
  if ( !OpenClipboard(hwndBBMagnify) )
  {
    MessageBox(hwndBBMagnify, "Cannot open the Clipboard", "Error",MB_OK|MB_ICONERROR);
    return;
  }
  // Remove the current Clipboard contents
  if( !EmptyClipboard() )
  {
    MessageBox(hwndBBMagnify, "Cannot empty the Clipboard", "Error",MB_OK|MB_ICONERROR );
    return;
  }

  // ...
  // Get the currently selected data
  // ...
  // For the appropriate data formats...
  if (SetClipboardData(CF_TEXT, hGlobal) == NULL )
  {
	
    MessageBox(hwndBBMagnify, "Unable to set data to Clipboard", "Error",MB_OK|MB_ICONERROR );
    CloseClipboard();
    return;
  }
  // ...
  CloseClipboard();
  copied = 20;

}


//--------------------------------------------------------------------------
void createMenu()
{
	bool tempBool = false;

	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
		
			//Now we define all menus and submenus
			
			otherSubmenu = MakeMenu("Other");

			MakeMenuItem(otherSubmenu, "Draw Border", "@BBMagnifyDrawBorder", drawBorder);
			MakeMenuItem(otherSubmenu, "Draw Crosshair", "@BBMagnifyDrawCross", drawCross);
			MakeMenuItem(otherSubmenu, "Draw Possition", "@BBMagnifyDrawPossition", drawPos);
			MakeMenuItem(otherSubmenu, "Draw Color", "@BBMagnifyDrawColor", drawColor);
			MakeMenuItem(otherSubmenu, "Magnify At Start", "@BBMagnifyAtStart", magStart);
			MakeMenuItemInt(otherSubmenu, "Width", "@BBMagnifyWidth", width, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Height", "@BBMagnifyHeight", height, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Font Size", "@BBMagnifyFontSize", fontSize, 6, width/3);
	
			windowStyleSubmenu = MakeMenu("Style");
			//MakeMenuNOP(windowStyleSubmenu, "___________________");
			if(StrStrI(windowStyle, "toolbar") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar:", "@BBMagnifyStyleToolbar", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonnp") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button:", "@BBMagnifyStyleButton", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonpr") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", "@BBMagnifyStyleButtonPr", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.label:", "@BBMagnifyStyleLabel", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "windowlabel") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", "@BBMagnifyStyleWindowLabel", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "clock") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", "@BBMagnifyStyleClock", tempBool);
					
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBMagnifySlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBMagnifyPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBMagnifyOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBMagnifyTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBMagnifySetTransparent",alpha,0,255);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBMagnifyFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBMagnifySnapToEdge", snapWindow);

			browseSubmenu = MakeMenu("Browse");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBMagnifyLoadBitmap", false);

			bitmapSubmenu = MakeMenu("Bitmap");
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
			MakeMenuItem(bitmapSubmenu, "Nothing", "@BBMagnifyNoBitmap", noBitmap);

			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBMagnifyEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBMagnifyReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBMagnifySaveSettings", false);
			
			ratioSubmenu = MakeMenu("Ratio");
			tempBool = false;
			if (ratio == 1) tempBool = true;
			MakeMenuItem(ratioSubmenu, "1x", "@BBMagnifyRatio 1", tempBool);
			tempBool = false;
			if (ratio == 2) tempBool = true;
			MakeMenuItem(ratioSubmenu, "2x", "@BBMagnifyRatio 2", tempBool);
			tempBool = false;
			if (ratio == 4) tempBool = true;
			MakeMenuItem(ratioSubmenu, "4x", "@BBMagnifyRatio 4", tempBool);
			tempBool = false;
			if (ratio == 8) tempBool = true;
			MakeMenuItem(ratioSubmenu, "8x", "@BBMagnifyRatio 8", tempBool);
			tempBool = false;
			if (ratio == 16) tempBool = true;
			MakeMenuItem(ratioSubmenu, "16x", "@BBMagnifyRatio 16", tempBool);
			tempBool = false;
			if (ratio == 32) tempBool = true;
			MakeMenuItem(ratioSubmenu, "32x", "@BBMagnifyRatio 32", tempBool);
			//attach defined menus together
			myMenu = MakeMenu("BBMagnify 1.0");
			
			MakeSubmenu(configSubmenu, ratioSubmenu, "Ratio");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, otherSubmenu, "Other");
			MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
			
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About", "@BBMagnifyAbout", false);
//		MakeMenuItem(myMenu, "Copy", "@BBMagnifyColorToClipboard", false);
			ShowMenu(myMenu);
}

// the end ....
