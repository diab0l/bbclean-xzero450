/*
 ============================================================================
 Blackbox for Windows: Plugin bbnoter 1.0 by Miroslav Petrasko [Theo] made
 from BBAnalogEx and Sdk Example
 ============================================================================
 Copyright © 2001-2002 The Blackbox for Windows Development Team
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

 ============================================================================
*/

#include "bbnoter.h"
#include "MessageBox.h"
#include "resource.h"


LPSTR szAppName = "BBNoter";		// The name of our window class, etc.
LPSTR szVersion = "BBNoter v1.0rc1";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0rc1";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-06-01";
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

/*	// Initialize GDI+.
	if(Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) != 0)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error starting GdiPlus.dll", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
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
						"EDIT",
//					szAppName,										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL|WS_POPUP,// | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,	// window parameters
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
		UnregisterClass(szAppName, hPluginInstance);
//		Gdiplus::GdiplusShutdown(gdiplusToken);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	wpEditProc = (WNDPROC)SetWindowLong(hwndBBNoter, GWL_WNDPROC,(long)WndProc);
	

	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
	else inSlit = false;
	
	setStatus();

//else
	//	SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | magicDWord);
	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBNoter, (LPARAM)msgs);
	MakeSticky(hwndBBNoter);
// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBNoter, GWL_USERDATA, magicDWord);
	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBNoter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
//	setRegion();
	//SetLayeredWindowAttributes(hwndBBNoter, 0x0000FF00, 125, LWA_ALPHA);
	ShowWindow(hwndBBNoter, SW_SHOW);
	InvalidateRect(hwndBBNoter, NULL, true);

	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBNoterReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBNoter()
{
	//set up center points and dimensions of clock
	//check for even size(0 is starting point from edges for x, y). fix for exact center point.
	//Have to do it this way since width is not a float and any operation would trim it.  darn types.
	if((width % 2) == 0) cntX = cntY = (float)((width / 2) - .5);
	else cntX = cntY = (float)((width - 1) / 2);
	
	//UINT uResult;               // SetTimer's return value 
	
    dwId = 0;
    dwMajorVer = 0;
    dwMinorVer = 0;

	//Init display mode counters
//	showDate = 0;
//	showAlarm = 0;
	circleCounter = 0;
//	oneMoreDraw = false;

	//getCurrentDate();
//	getCurrentTime();

	//Get Platform type
	_GetPlatformId (&dwId, &dwMajorVer, &dwMinorVer);
    
	//get screen dimensions
	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}

//===========================================================================

void endPlugin(HINSTANCE hPluginInstance)
{/*
	if(graphics) delete graphics;
	if(pen) delete pen;
	//if(font) delete font;
	if(fontFamily) delete fontFamily;
	if(format) delete format;
	if(brush) delete brush;
	if(layoutRect) delete layoutRect;
	if(plusFontColor) delete plusFontColor;
//	if(plusYellowColor) delete plusYellowColor;
//	if(plusBlueColor) delete plusBlueColor;
//	if(plusRedColor) delete plusRedColor;
//	if(origin) delete origin;
*/	
	//shutdown the gdi+ engine
//	Gdiplus::GdiplusShutdown(gdiplusToken);
	// Release our timer resources
//	KillTimer(hwndBBNoter, IDT_TIMER);
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{		
		// Window update process...
		case WM_PAINT:
		{
			// Create buffer hdc's, bitmaps etc.
			PAINTSTRUCT ps;  RECT r;
//			PAINTSTRUCT ps1;
			//get screen buffer
            HDC hdc_scrn = BeginPaint(hwnd, &ps);
		//	HDC hdc1 = BeginPaint(hEdit, &ps1);

			//retrieve the coordinates of the window's client area.
			GetClientRect(hwnd, &r);

			//to prevent flicker of the display, we draw to memory first,
            //then put it on screen in one single operation. This is like this:

            //first get a new 'device context'
			HDC hdc = CreateCompatibleDC(NULL);

		
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc_scrn, r.right, r.bottom);
            SelectObject(hdc, bufbmp);
			



			if(!inSlit)
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
			}
			

			SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, numbColor);

		//Paint second background according to toolbar.label.color: and toolbar.label.colorto:
			if(!labelIsPR || inSlit)
			{
				if(!inSlit)
				{
					r.left = r.left + (bevelWidth + borderWidth);
					r.top = r.top + (bevelWidth + borderWidth);
					r.bottom = (r.bottom - (bevelWidth + borderWidth));
					r.right = (r.right - (bevelWidth + borderWidth));
				}
						
				//draw second background according to toolbar.label.color: and toolbar.label.colorto:
		
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

			
			if(!inSlit)
			{	
				if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
					{
					HBRUSH hbOrig, hBrush;
					GetClientRect(hwnd, &rect);
					hBrush = CreateSolidBrush(0x202020);
					hbOrig = (HBRUSH)SelectObject(hdc, hBrush);
					Rectangle(hdc, -1, -1, rect.right+1 , rect.bottom+1);
					DeleteObject(hBrush);
					DeleteObject(hbOrig);
					}
			}
			

				//Paint to the screen 
				BitBlt(hdc_scrn, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
     		DeleteDC(hdc_scrn);   
			DeleteDC(hdc);         //gdi: first delete the dc
			DeleteObject(bufbmp);  //gdi: now the bmp is free
			
			//takes care of hdc_scrn
//			EndPaint(hEdit, &ps1);
			EndPaint(hwnd, &ps);

			return 0;
		}
		break;

	

		// If Blackbox sends a reconfigure message, load the new style settings and update the window...
		case BB_RECONFIGURE:
		{
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
			GetStyleSettings();
			//radius = cntX - (float)(((2 * bevelWidth) + borderWidth) - 1);
//			getCurrentTime();
			InvalidateRect(hwndBBNoter, NULL, true);
		}
		break;

		case WM_CTLCOLOREDIT:
		{
			//gr
			SetTextColor((HDC)wParam, 0x000000);
			SetBkMode((HDC)wParam, TRANSPARENT);
			return (LRESULT)GetStockObject(NULL_BRUSH);
		}
		break;

		

		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);

			if (!_stricmp(szTemp, "@BBShowPlugins") &&  pluginToggle && !inSlit)
			{
				// Show window and force update...
				ShowWindow( hwndBBNoter, SW_SHOW);
//				getCurrentTime();
				showAlarm = 0;
//				if(showSeconds) mySetTimer(0);
//				else mySetTimer(1);
				InvalidateRect( hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBNoter, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBNoterAbout"))
			{
				//char temp[MAX_LINE_LENGTH];
				
				sprintf(szTemp, "%s\n\n%s ©2004 \n%s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBBNoter,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
				//sprintf(szTemp, "%s\n\n© 2003 Mortar - Brad Bartolucci\n%s\n", szVersion, szInfoEmail);
				//MessageBox(hwndBBNoter, szTemp, szAppName, MB_OK | MB_SETFOREGROUND | MB_ICONINFORMATION);
			}

			else if (!_stricmp(szTemp, "@BBNoterPluginToggle"))
			{
				// Hide window...
				if (pluginToggle)
					 pluginToggle = false;
				else
					 pluginToggle = true;
			}
			else if (!_stricmp(szTemp, "@BBNoterOnTop"))
			{
				// Hide window...
				if (alwaysOnTop)
					 alwaysOnTop = false;
				else
					 alwaysOnTop = true;
				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBNoter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBNoter, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBNoterSlit") && showAlarm == 0)
			{
				// Does user want it in the slit...
				if (wantInSlit)
					 wantInSlit = false;
				else
					 wantInSlit = true;

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
//				getCurrentTime();
//				setRegion();
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBNoterTransparent"))
			{
				// Set the transparent attributes to the window
				if (transparency)
					 transparency = false;
				else
					transparency = true; 
				
		
			setStatus();
			InvalidateRect(hwndBBNoter, NULL, true);
			}

			else if (!_stricmp(szTemp, "@BBNoterFullTrans"))
			{
				// Set the transparent attributes to the window
				if (fullTrans)
					fullTrans = false;
				else
					fullTrans = true; 
		
			setStatus();
			InvalidateRect(hwndBBNoter, NULL, true);
			}



			else if (!_stricmp(szTemp, "@BBNoterSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				if (snapWindow)
					 snapWindow = false;
				else
					 snapWindow = true;
			}
			else if (!_stricmp(szTemp, "@BBNoterLockPosition"))
			{
				// Set the snapWindow attributes to the window
				if (lockp)
					 lockp = false;
				else
					 lockp = true;
			}
			else if (!_stricmp(szTemp, "@BBNoterStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
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
				if(showAlarm <= 0)
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBNoter);

					//Re-initialize
					ReadRCSettings();
					InitBBNoter();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					//reset timer
//					if(showSeconds) mySetTimer(0);
//					else mySetTimer(1);

					//check for windows 2000 or higher before using transparency
		
					setStatus();

				//	else if(fullTrans) SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBNoter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBNoter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBNoter);
					else inSlit = false;

					//update window
//					getCurrentTime();
//					setRegion();
					InvalidateRect(hwndBBNoter, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBNoterSaveSettings"))
			{
				WriteRCSettings();
			}
	    	else if (!_strnicmp(szTemp, "@BBNoterTextSize", 15))
			{
				fontSizeC = atoi(szTemp + 16);
				InvalidateRect(hwndBBNoter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBNoterSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				
				if (transparency)
				{
						setStatus();
				}
							
				InvalidateRect(hwndBBNoter, NULL, true);
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
				MoveWindow(hwndBBNoter, xpos, ypos, width, height, true);
			}
		}
		break;

		// ==========
		// Allow window to move if the cntrl key is not pressed...
		case WM_NCHITTEST:
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				return HTCAPTION;
		//	else
		//		return HTCLIENT;
		}
		break;
		
		case WM_NCRBUTTONUP:
		{	
		
			 createMenu();

		}
		break;
		

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
			if(!inSlit) radius = cntX - (float)(((2 * bevelWidth) + borderWidth));
			else radius = cntX - (float)((bevelWidth));
			labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.label
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
			if(!inSlit) radius = cntX - (float)((bevelWidth + borderWidth));
			else radius = cntX - (float)(bevelWidth);
			labelIsPR = true;
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
			if(!inSlit) radius = cntX - (float)(((2 * bevelWidth) + borderWidth));
			else radius = cntX - (float)((bevelWidth));
			labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.windowLabel
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
			if(!inSlit) radius = cntX - (float)((bevelWidth + borderWidth));
			else radius = cntX - (float)(bevelWidth);
			labelIsPR = true;
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
			if(!inSlit) radius = cntX - (float)(((2 * bevelWidth) + borderWidth));
			else radius = cntX - (float)((bevelWidth));
			labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
			if(!inSlit) radius = cntX - (float)((bevelWidth + borderWidth));
			else radius = cntX - (float)(bevelWidth);
			labelIsPR = true;
		}
	}
	
	
	// ...font settings...
	strcpy(fontFace, ReadString(stylepath, "toolbar.font:", ""));
	if (!_stricmp(fontFace, "")) strcpy(fontFace, ReadString(stylepath, "*font:", "Tahoma"));
	fontSize = (width / 3);
	//fontColor = ReadColor(stylepath, "toolbar.label.textColor:", "#FFFFFF");
}

//===========================================================================

void ReadRCSettings()
{
	char temp[MAX_LINE_LENGTH], path[MAX_LINE_LENGTH], defaultpath[MAX_LINE_LENGTH];
	int nLen;
	magicHourFreq = false;

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
			xpos = 10;
			ypos = 10;
			width = 100;
			height = 100;
			alpha = 160;
			wantInSlit = true;
			alwaysOnTop = true;
			snapWindow = true;
			transparency = false;
			fullTrans = false;
			lockp = false;
			showGrid = false;
	    	pluginToggle = false;
        	fontSizeC = 8;
			strcpy(windowStyle, "windowlabel");
			numbColor = 0x00FFFFFF;
			gridColor = 0x00FFFFFF;
	    	WriteRCSettings();
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbnoter.x:", 10);
	ypos = ReadInt(rcpath, "bbnoter.y:", 10);
	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 10;
	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 10;

	width  = ReadInt(rcpath, "bbnoter.width:", 100);
	height = ReadInt(rcpath, "bbnoter.height:", 100);
	if(width < 5 || width > 300 || height < 5 || height > 300) width = height = 100;

	alpha = ReadInt(rcpath, "bbnoter.alpha:", 160);
	if(alpha > 255) alpha = 255;
	if(ReadString(rcpath, "bbnoter.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbnoter.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "bbnoter.alwaysOnTop:", true);
	snapWindow = ReadBool(rcpath, "bbnoter.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbnoter.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbnoter.fullTrans:", false);
	lockp = ReadBool(rcpath, "bbnoter.lockPosition:", false);
	fontSizeC = ReadInt(rcpath, "bbnoter.fontSizeC:", 8);
	alwaysOnTop = ReadBool(rcpath, "bbnoter.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbnoter.pluginToggle:", false);
	showGrid = ReadBool(rcpath, "bbnoter.showGrid:", false);

	strcpy(windowStyle, ReadString(rcpath, "bbnoter.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL))
		strcpy(windowStyle, "windowLabel");
	numbColor = ReadColor(rcpath, "bbnoter.numberColor:", "#FFFFFF");
	gridColor = ReadColor(rcpath, "bbnoter.gridColor:", "#FFFFFF");
}
//===========================================================================

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
		
		sprintf(szTemp, "bbnoter.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.fontSizeC: %d\r\n", fontSizeC, temp);
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

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(lockp) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.lockPosition: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(showGrid) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.showGrid: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(sundayFirst) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbnoter.sundayFirst: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbnoter.numberColor: #%.2x%.2x%.2x\r\n", GetRValue(numbColor),GetGValue(numbColor),GetBValue(numbColor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbnoter.gridColor: #%.2x%.2x%.2x\r\n", GetRValue(gridColor),GetGValue(gridColor),GetBValue(gridColor));
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

//get current local time of the users machine
void getCurrentDate()
{
	time(&systemTime);
	localTime		= localtime(&systemTime);
	strftime(szTemp, 10, "%m", localTime);
	month = (int)atoi(szTemp);
	strftime(szTemp, 10, "%Y", localTime);
	year = (int)atoi(szTemp);
	if (div(year,4).rem==0) daysInMonth[1]=29;
	else daysInMonth[1]=28;
	strftime(szTemp, 10, "%d", localTime);
	day = (int)atoi(szTemp);
	strftime(szTemp, 10, "%w", localTime);
	move = (int)atoi(szTemp);
	if (!sundayFirst) move--;
	move = move - div(day,7).rem;
	if (move<0) move+=7;
//	currentSecond	= localTime->tm_sec;
//	currentMinute	= localTime->tm_min;
//	currentHour		= localTime->tm_hour;
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

//=======================
void setStatus()
{

	//check for windows 2000 or higher before using transparency
		if(!inSlit)
					{
						if (transparency && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBNoter, 0x202020, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
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
								BBSetLayeredWindowAttributes(hwndBBNoter, 0x202020, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBNoter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);



}



void createMenu()
{

	bool tempBool = false;
			// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

			//Now we define all menus and submenus

			windowStyleSubmenu = MakeMenu("Window Style");
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
			MakeMenuItem(generalConfigSubmenu, "Lock Position", "@BBNoterLockPosition", lockp);
			

	
			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBNoterEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBNoterReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBNoterSaveSettings", false);
	
				//attach defined menus together
			myMenu = MakeMenu("BBNoter 1.0rc1");
			MakeMenuItem(myMenu, "About", "@BBNoterAbout", false);
		//	MakeSubmenu(myMenu, setAlarmsConfigSubmenu, "Set Alarms");
			MakeSubmenu(myMenu, windowStyleSubmenu, "Window Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
		MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			
	
			// Finally, we show the menu...
			ShowMenu(myMenu);

}

