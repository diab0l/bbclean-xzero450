/*
 ============================================================================
 Blackbox for Windows: Plugin BBNoter 1.0 by Miroslav Petrasko [Theo]
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

#include "bbnoter.h"
#include "resource.h"

LPSTR szAppName = "BBNoter";		// The name of our window class, etc.
LPSTR szVersion = "BBNoter v1.0";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-08-04";
LPSTR szInfoLink = "theo.host.sk";
LPSTR szInfoEmail = "theo.devil@gmx.net";

//===========================================================================

int beginPlugin(HINSTANCE hPluginInstance)
{
//	WNDCLASS wc;
	hwndBlackbox = GetBBWnd();
//	hInstance = hPluginInstance;

	// Register the window class...
//	ZeroMemory(&wc,sizeof(wc));
//	wc.lpfnWndProc = WndProc;			// our window procedure
//	wc.hInstance = hPluginInstance;		// hInstance of .dll
//	wc.lpszClassName = szAppName;		// our window class name
/*	if (!RegisterClass(&wc)) 
	{
		MessageBox(hwndBlackbox, "Error registering window class", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}
*/
	// Get plugin and style settings...
	ReadRCSettings();
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBNoter();
	GetStyleSettings();
	
	// Create the window...
	hwndBBNoter = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						"EDIT",										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP|WS_VISIBLE |ES_MULTILINE|ES_AUTOVSCROLL,										// window parameters
						xpos,											// x position
						ypos,											// y position
						width,											// window width
						height,											// window height
						NULL,											// parent window
						NULL,											// no menu
						hPluginInstance,								// hInstance of .dll
						NULL);
	if (!hwndBBNoter)
	{
//		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}


	wpEditProc = (WNDPROC)SetWindowLong(hwndBBNoter, GWL_WNDPROC,(long)EditProc);
	MakeSticky(hwndBBNoter);
	//Start the plugin timer
	mySetTimer();
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBNoter, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	//SetWindowLong(hwndBBNoter, GWL_USERDATA, magicDWord);
	
	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBNoter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBNoter, SW_SHOW);
	
	
	GetClientRect(hwndBBNoter, &r);

	if(drawBorder)
			{
				r.left = r.left + (bevelWidth + borderWidth);
				r.top = r.top + (bevelWidth + borderWidth);
				r.bottom = (r.bottom - (bevelWidth + borderWidth));
				r.right = (r.right - (bevelWidth + borderWidth));
			}
	SendMessage(hwndBBNoter, EM_SETRECT, 0, (LPARAM)&r);


	InvalidateRect(hwndBBNoter, NULL, true);
	
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBNoterReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBNoter()
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
	KillTimer(hwndBBNoter, IDT_TIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...


	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBNoter, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBNoter);
	// Destroy our window...
	DestroyWindow(hwndBBNoter);
	// Unregister window class...
	UnregisterClass(szAppName, hPluginInstance);
}

//===========================================================================
LRESULT CALLBACK EditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_SETFOCUS:
		{
			SendMessage(hwndBBNoter,EM_SETSEL,0,-1);
			mySetTimer();
//			pBBox->ToggleSlitAutoHide(true);
		}
		break;

		case WM_KILLFOCUS:
		{
			KillTimer(hwndBBNoter, IDT_TIMER);
//			pBBox->ResetHistoryIndex();
		//	pBBox->ToggleSlitAutoHide(false);
		}
		break;

		case WM_NCHITTEST:
		{
			if((GetAsyncKeyState(VK_CONTROL) & 0x8000))
				return HTCAPTION;
		}
		break;

		case WM_NCRBUTTONUP:
		//	return pBBox->RButtonDown(hText, msg, wParam, lParam);
			createMenu();
		
		case WM_KEYDOWN:
		{
		/*	if(wParam==VK_DOWN)
			{
	//			pBBox->HistoryPrev();
				return 0;
			}
			if(wParam==VK_UP)
			{
		//		pBBox->HistoryNext();
				return 0;
			}
			if(KEY_CHECK(wParam))
		*/		InvalidateRect(hwndBBNoter, NULL, true);
		}
		break;

		case WM_CHAR:
		{
			/*if(wParam==VK_RETURN)
			{
			//	pBBox->Execute();
			//	pBBox->ToggleSlitAutoHide(false);
				
				InvalidateRect(hwndBBNoter, NULL, true);

			//	return 0;
			}*/
			
		}
		break;

		case WM_CLOSE: { return 0; } break; // no ALT+F4

		//gr

		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_TIMER:
				{
					//redraw the window
					line = SendMessage(hwndBBNoter,EM_GETFIRSTVISIBLELINE,0,0);
					if (lastLine != line) InvalidateRect(hwndBBNoter, NULL, false);
					lastLine = line;
				} break;
			}
		}
		break;
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
					GetClientRect(hwnd, &rect);
					hBrush = CreateSolidBrush(0xFF00FF);
					hbOrig = (HBRUSH)SelectObject(hdc, hBrush);
					Rectangle(hdc, -1,-1,rect.right+1, rect.bottom+1);
					DeleteObject(hBrush);
					DeleteObject(hbOrig);
				}
	
			
					
				/*	HGDIOBJ otherfont = CreateFont( 13, 
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace);
					SendMessage(hwnd, WM_SETFONT, (WPARAM)otherfont, FALSE);
					SelectObject(hdc, otherfont);
					SetTextColor(hdc,fontColor);*/
									
					
				POINT poloha;
				GetCursorPos(&poloha);

				

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
				

		//		sprintf(szTemp,"X:%i Y:%i",poloha.x,poloha.y);
		//		DrawText(hdc,szTemp,-1,&r,DT_BOTTOM | DT_CENTER | DT_SINGLELINE);


				//Paint to the screen
			//	DeleteObject(otherfont);

				CallWindowProc(wpEditProc, hwnd, message, (WPARAM)hdc, lParam);

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

		case WM_CTLCOLOREDIT:
		{
			//grSetTextColor((HDC)wParam, fontColor);
			SetTextColor((HDC)wParam, 0xffffff);
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
		}
		break;

		case WM_ERASEBKGND:
			return TRUE;

		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);
		
			if (!_stricmp(szTemp, "@BBShowPlugins") &&  pluginToggle && !inSlit)
			{
				// Show window and force update...
				ShowWindow( hwndBBNoter, SW_SHOW);
				InvalidateRect( hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBNoter, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBNoterAbout"))
			{
				sprintf(szTemp, "%s\n\n%s ©2004 %s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBBNoter,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
			}
			else if (!_stricmp(szTemp, "@BBNoterPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBNoterOnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;

				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBNoter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBNoter, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBNoterSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBNoter);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBNoterFullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBNoterSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBNoterDrawBorder"))
			{
				drawBorder = !drawBorder;
				GetClientRect(hwnd, &r);

				if(drawBorder)
				{
				r.left = r.left + (bevelWidth + borderWidth);
				r.top = r.top + (bevelWidth + borderWidth);
				r.bottom = (r.bottom - (bevelWidth + borderWidth));
				r.right = (r.right - (bevelWidth + borderWidth));
				}
				SendMessage(hwndBBNoter, EM_SETRECT, 0, (LPARAM)&r);
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterNoBitmap"))
			{
				noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBNoterReloadSettings"))
			{
				
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBNoter);

					//Re-initialize
					ReadRCSettings();
					InitBBNoter();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBNoter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBNoter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBNoter, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBNoterSaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBNoterFontSize", 18))
			{
				fontSize = atoi(szTemp + 19);
				
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterWidth", 15))
			{ //changing the clock size
				width = atoi(szTemp + 16);
				
				if ( alwaysOnTop) SetWindowPos( hwndBBNoter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBNoter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterHeight", 16))
			{ //changing the clock size
				height = atoi(szTemp + 17);
									
				if ( alwaysOnTop) SetWindowPos( hwndBBNoter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBNoter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				if (transparency) setStatus();					
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterSetBitmap", 18))
			{
				
				strcpy(bitmapFile,szTemp + 20);
				noBitmap = false;
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterRatio", 15))
			{
				ratio = atoi(szTemp + 16);
			
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterLoadBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBNoter;
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

			InvalidateRect(hwndBBNoter, NULL, true);

			}
		}
		return 0;

		case BB_RECONFIGURE:
		{
			
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBNoter);

					//Re-initialize
					ReadRCSettings();
					InitBBNoter();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBNoter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBNoter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBNoter, NULL, true);
				
		}
		return 0;
	}
	return CallWindowProc(wpEditProc,hwndBBNoter,message,wParam,lParam);
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
	strcat(temp, "bbnoter.rc");
	strcat(path, "bbnoterrc");
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
		strcat(temp, "bbnoter.rc");
		strcat(path, "bbnoterrc");
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

			WriteRCSettings();
			
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbnoter.x:", 10);
	ypos = ReadInt(rcpath, "bbnoter.y:", 10);

	width = ReadInt(rcpath, "bbnoter.width:", 100);
	height = ReadInt(rcpath, "bbnoter.height:", 50);

	ratio = ReadInt(rcpath, "bbnoter.ratio:", 1);
	if (ratio<=0) ratio = 1; 
	alpha = ReadInt(rcpath, "bbnoter.alpha:", 160);
	if(alpha > 255) alpha = 255;
	if(ReadString(rcpath, "bbnoter.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbnoter.inSlit:", true);
	alwaysOnTop = ReadBool(rcpath, "bbnoter.alwaysOnTop:", true);
	drawBorder = ReadBool(rcpath, "bbnoter.drawBorder:", true);
	snapWindow = ReadBool(rcpath, "bbnoter.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbnoter.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbnoter.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbnoter.fontSize:", 6);
	alwaysOnTop = ReadBool(rcpath, "bbnoter.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbnoter.pluginToggle:", false);
	strcpy(windowStyle, ReadString(rcpath, "bbnoter.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");
	strcpy(bitmapFile, ReadString(rcpath, "bbnoter.bitmapFile:", ".none"));
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

		sprintf(szTemp, "! BBNoter %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbnoter.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbnoter.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.ratio: %d\r\n", ratio, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbnoter.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.drawBorder: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

    	sprintf(szTemp, "bbnoter.fontSize: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.bitmapFile: %s\r\n", bitmapFile);
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
	
		//Start the 0.10 second plugin timer
		SetTimer(hwndBBNoter,		// handle to main window 
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
								SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBNoter, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBNoter, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBNoter, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBNoter, NULL, false);		
}

void createMenu()
{
	bool tempBool = false;

	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
		
			//Now we define all menus and submenus
			
			otherSubmenu = MakeMenu("Other");

			MakeMenuItem(otherSubmenu, "Draw Border", "@BBNoterDrawBorder", drawBorder);
			MakeMenuItemInt(otherSubmenu, "Width", "@BBNoterWidth", width, 20, 400);
			MakeMenuItemInt(otherSubmenu, "Height", "@BBNoterHeight", height, 20, 400);
		//	MakeMenuItemInt(otherSubmenu, "Font Size", "@BBNoterFontSize", fontSize, 6, width/3);
	
			windowStyleSubmenu = MakeMenu("Style");
			//MakeMenuNOP(windowStyleSubmenu, "___________________");
			if(StrStrI(windowStyle, "toolbar") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar:", "@BBNoterStyleToolbar", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonnp") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button:", "@BBNoterStyleButton", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonpr") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", "@BBNoterStyleButtonPr", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.label:", "@BBNoterStyleLabel", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "windowlabel") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", "@BBNoterStyleWindowLabel", tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "clock") != NULL) tempBool = true;
			MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", "@BBNoterStyleClock", tempBool);
					
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBNoterSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBNoterPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBNoterOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBNoterTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBNoterSetTransparent",alpha,0,255);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBNoterFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBNoterSnapToEdge", snapWindow);

			browseSubmenu = MakeMenu("Browse");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBNoterLoadBitmap", false);

			bitmapSubmenu = MakeMenu("Bitmap");
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
			MakeMenuItem(bitmapSubmenu, "Nothing", "@BBNoterNoBitmap", noBitmap);

			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBNoterEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBNoterReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBNoterSaveSettings", false);
			
		/*	ratioSubmenu = MakeMenu("Ratio");
			tempBool = false;
			if (ratio == 1) tempBool = true;
			MakeMenuItem(ratioSubmenu, "1x", "@BBNoterRatio 1", tempBool);
			tempBool = false;
			if (ratio == 2) tempBool = true;
			MakeMenuItem(ratioSubmenu, "2x", "@BBNoterRatio 2", tempBool);
			tempBool = false;
			if (ratio == 4) tempBool = true;
			MakeMenuItem(ratioSubmenu, "4x", "@BBNoterRatio 4", tempBool);
			tempBool = false;
			if (ratio == 8) tempBool = true;
			MakeMenuItem(ratioSubmenu, "8x", "@BBNoterRatio 8", tempBool);
			tempBool = false;
			if (ratio == 16) tempBool = true;
			MakeMenuItem(ratioSubmenu, "16x", "@BBNoterRatio 16", tempBool);
			tempBool = false;
			if (ratio == 32) tempBool = true;
			MakeMenuItem(ratioSubmenu, "32x", "@BBNoterRatio 32", tempBool);
		*/	//attach defined menus together
			myMenu = MakeMenu("BBNoter 1.0");
			
		//	MakeSubmenu(configSubmenu, ratioSubmenu, "Ratio");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, otherSubmenu, "Other");
			MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
			
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About", "@BBNoterAbout", false);
			ShowMenu(myMenu);
}

// the end ....