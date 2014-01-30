/*
 ============================================================================
 Blackbox for Windows: Plugin BBMessageBox 1.0 by Miroslav Petrasko [Theo] made
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

#include "BBMessageBox.h"
#include "MessageBox.h"
#include "resource.h"


LPSTR szAppName = "BBMessageBox";		// The name of our window class, etc.
LPSTR szVersion = "BBMessageBox v1.0";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.0";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-06-05";
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
//	if(!hSlit) inSlit = false;
	//else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBMessageBox();
	GetStyleSettings();
	//width = height = 32+2*(borderWidth+bevelWidth);
	
	// Create the window...
	hwndBBMessageBox = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						szAppName,										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP ,	// window parameters
						xpos,											// x position
						ypos,											// y position
						200,											// window width
						100,											// window height
						NULL,											// parent window
						NULL,											// no menu
						hInstance,								// hInstance of .dll
						NULL);
	if (!hwndBBMessageBox)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating 8 window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}
	
	
/*	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMessageBox);
	else inSlit = false;
*/	
	setStatus();

	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBMessageBox, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBMessageBox, GWL_USERDATA, magicDWord);
	// Make the window AlwaysOnTop?
	SetWindowPos(hwndBBMessageBox, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...

//	SetWindowPos(hwndMessage, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	
//	ShowWindow(hwndMessage, SW_SHOW);
	
//	if (hide) ShowWindow( hwndBBMessageBox, SW_HIDE);

//	InvalidateRect(hwndBBMessageBox, NULL, true);

///	BBMessageBox *m = new BBMessageBox(hwndBBMessageBox,hPluginInstance,"ss","ss");
//	m->DoModal();
//	delete m;
//	CreateMessageBox();
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBMessageBoxReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBMessageBox()
{
	
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
{	// Write the current plugin settings to the config file...
	WriteRCSettings();
//	DestroyMessageBox();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...
	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBMessageBox, (LPARAM)msgs);
/*	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMessageBox);
*/	// Destroy our window...
	DestroyWindow(hwndBBMessageBox);
	
	// Unregister window class...
	UnregisterClass(szAppName, hPluginInstance);
	
}

//===========================================================================
/*
LRESULT CALLBACK MesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

			HICON hIcon;
			hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_8BALL));
			DrawIcon (hdc,bevelWidth + borderWidth,bevelWidth + borderWidth,hIcon);
			
				//Paint to the screen 
			BitBlt(hdc_scrn, 0, 0, width, height, hdc, 0, 0, SRCCOPY);

			DeleteObject(hIcon);
			DeleteDC(hdc_scrn);   
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
			InvalidateRect(hwndBBMessageBox, NULL, true);
		}
		break;

		// ==========

		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);

			if (!_stricmp(szTemp, "@BBShowPlugins") &&  pluginToggle && !inSlit && !hide)
			{
				// Show window and force update...
				ShowWindow( hwndBBMessageBox, SW_SHOW);
				InvalidateRect( hwndBBMessageBox, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBMessageBox, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxHideMode"))
			{
				if (hide)
				{ hide = false; ShowWindow( hwndBBMessageBox, SW_SHOW);
				InvalidateRect( hwndBBMessageBox, NULL, true);}
				else
				{hide = true; ShowWindow( hwndBBMessageBox, SW_HIDE);}
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxAbout"))
			{
				
				
				sprintf(szTemp, "%s\n\n%s ©2004 \n%s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBBMessageBox,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
			}
	else if (!_stricmp(szTemp, "@BBMessageBoxPluginToggle"))
			{
				// Hide window...
				if (pluginToggle)
					 pluginToggle = false;
				else
					 pluginToggle = true;
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxOnTop"))
			{
				// Hide window...
				if (alwaysOnTop)
					 alwaysOnTop = false;
				else
					 alwaysOnTop = true;
				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBMessageBox, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBMessageBox, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxSlit"))
			{
				// Does user want it in the slit...
				if (wantInSlit)
					 wantInSlit = false;
				else
					 wantInSlit = true;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMessageBox);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMessageBox);
				else
					inSlit = false;	

				
				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxTransparent"))
			{
				// Set the transparent attributes to the window
				if (transparency)
					 transparency = false;
				else
					transparency = true; 
			
			setStatus();
			InvalidateRect(hwndBBMessageBox, NULL, true);
			}

			else if (!_stricmp(szTemp, "@BBMessageBoxFullTrans"))
			{
				// Set the transparent attributes to the window
				if (fullTrans)
					fullTrans = false;
				else
					fullTrans = true; 
				
			setStatus();
			InvalidateRect(hwndBBMessageBox, NULL, true);
			}



			else if (!_stricmp(szTemp, "@BBMessageBoxSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				if (snapWindow)
					 snapWindow = false;
				else
					 snapWindow = true;
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxPredict")) 
			{
				showResult();
			}

			else if (!_stricmp(szTemp, "@BBMessageBoxStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxReloadSettings"))
			{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBMessageBox);

					//Re-initialize
					ReadRCSettings();
					InitBBMessageBox();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBMessageBox, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBMessageBox, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBMessageBox);
					else inSlit = false;

					InvalidateRect(hwndBBMessageBox, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BBMessageBoxSaveSettings"))
			{
				WriteRCSettings();
			}
	

	/*		else if (!_strnicmp(szTemp, "@BBMessageBoxSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				
				if (transparency) setStatus();
							
				
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
*//*
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
				MoveWindow(hwndBBMessageBox, xpos, ypos, width, height, true);
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

		// ==========
		case WM_LBUTTONUP: 
		{
			showResult();
		}
		break;
		
		// Right mouse button clicked?
		case WM_RBUTTONUP:
		{	
			// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

			//Now we define all menus and submenus
			
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			MakeMenuItem(generalConfigSubmenu, "Hide mode", "@BBMessageBoxHideMode", hide);
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBMessageBoxSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBMessageBoxPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBMessageBoxOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBMessageBoxTransparent", transparency);
	/*		if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBMessageBoxSetTransparent",alpha,0,255);
	*//*		if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBMessageBoxFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBMessageBoxSnapToEdge", snapWindow);
	
			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBMessageBoxEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBMessageBoxReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBMessageBoxSaveSettings", false);
			
			//attach defined menus together
			myMenu = MakeMenu("BBMessageBox 1.0");
			MakeMenuItem(myMenu, "Predict...", "@BBMessageBoxPredict", false);
			MakeSubmenu(myMenu, generalConfigSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About...", "@BBMessageBoxAbout", false);
			

			// Finally, we show the menu...
			ShowMenu(myMenu);
		}
		break;
	
		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}*/

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
//	bool parentstyle = false;

	
	
 //if(StrStrI(windowStyle, "label") != NULL  && strlen(windowStyle) < 6)
//	{
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
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
		
	
	{
		// ...gradient type, bevel etc. from toolbar.windowLabel:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.button:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (button) delete button;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			button = new StyleItem;			
			ParseItem(tempstyle2, button);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.button.color:", "")))
				buttonColor = ReadColor(stylepath, "toolbar.button.color:", "#000000");
			else
    			buttonColor = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.button.colorTo:", "")))
				buttonColorTo = ReadColor(stylepath, "toolbar.button.colorTo:", "#000000");
			else
				buttonColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			buttonfontColor = ReadColor(stylepath, "toolbar.button.picColor:", "#FFFFFF");
		}
		else
		{
			if (button) delete button;	//else use the the toolbar: settings
			button = new StyleItem;
			ParseItem(tempstyle, button);	//use original tempstyle if "parentrelative"
			buttonColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			buttonColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			buttonfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}

	{
		// ...gradient type, bevel etc. from toolbar.clock:(using a StyleItem)...
		char tempstyle2[MAX_LINE_LENGTH];
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.button.pressed:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (buttonpr) delete buttonpr;	//if everything is found in toolbar.clock: then make a new StyleItem
			buttonpr = new StyleItem;			
			ParseItem(tempstyle2, buttonpr);
			
				if (!IsInString("", ReadString(stylepath, "toolbar.button.pressed.color:", "")))
				buttonprColor = ReadColor(stylepath, "toolbar.button.pressed.color:", "#000000");
			else
    			buttonprColor = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.button.pressed.colorTo:", "")))
				buttonprColorTo = ReadColor(stylepath, "toolbar.button.pressed.colorTo:", "#000000");
			else
				buttonprColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			buttonprfontColor = ReadColor(stylepath, "toolbar.clock.textColor:", "#FFFFFF");
		}
		else
		{
			if (button) delete button;	//else use the the toolbar: settings
			button = new StyleItem;
			ParseItem(tempstyle, button);	//use original tempstyle if "parentrelative"
			buttonprColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			buttonprColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			buttonprfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	


	}
	
	{
			if (toolbar) delete toolbar;	//else use the the toolbar: settings
			toolbar = new StyleItem;
			ParseItem(tempstyle, toolbar);	//use original tempstyle if "parentrelative"
			toolbarColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			toolbarColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			toolbarfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
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
//	magicHourFreq = false;
	strcpy(dcaption,"BBMessageBox");
	// First we look for the config file in the same folder as the plugin...
	GetModuleFileName(hInstance, rcpath, sizeof(rcpath));
	nLen = strlen(rcpath) - 1;
	while (nLen >0 && rcpath[nLen] != '\\') nLen--;
	rcpath[nLen + 1] = 0;
	strcpy(temp, rcpath);
	strcpy(path, rcpath);
	strcat(temp, "BBMessageBox.rc");
	strcat(path, "BBMessageBoxrc");
//	strcpy(windowStyle, "windowlabel");
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
		strcat(temp, "BBMessageBox.rc");
		strcat(path, "BBMessageBoxrc");
		if (FileExists(temp)) strcpy(rcpath, temp);
		else if (FileExists(path)) strcpy(rcpath, path);
		else // If no config file was found, we use the default path and settings, and return
		{
			strcpy(rcpath, defaultpath);
			xpos = 10;
			ypos = 10;
			alpha = 160;
//			wantInSlit = true;
		//	alwaysOnTop = true;
			snapWindow = true;
			transparency = false;
		//	fullTrans = false;
		//	pluginToggle = false;
		//	hide = false;
			WriteRCSettings();
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "BBMessageBox.x:", 10);
	ypos = ReadInt(rcpath, "BBMessageBox.y:", 10);
	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 10;
	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 10;

	alpha = ReadInt(rcpath, "BBMessageBox.alpha:", 160);
	if(alpha > 255) alpha = 255;
//	if(ReadString(rcpath, "BBMessageBox.inSlit:", NULL) == NULL) wantInSlit = true;
//	else wantInSlit = ReadBool(rcpath, "BBMessageBox.inSlit:", true);
	
	//alwaysOnTop = ReadBool(rcpath, "BBMessageBox.alwaysOnTop:", true);
	snapWindow = ReadBool(rcpath, "BBMessageBox.snapWindow:", true);
	transparency = ReadBool(rcpath, "BBMessageBox.transparency:", false);
//	fullTrans = ReadBool(rcpath, "BBMessageBox.fullTrans:", false);
//	alwaysOnTop = ReadBool(rcpath, "BBMessageBox.alwaysontop:", true);
//	pluginToggle = ReadBool(rcpath, "BBMessageBox.pluginToggle:", false);
//	hide = ReadBool(rcpath, "BBMessageBox.hideMode:", false);
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

		sprintf(szTemp, "! BBMessageBox %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "BBMessageBox.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "BBMessageBox.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "BBMessageBox.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
/*
		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

/*		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(hide) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BBMessageBox.hideMode: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
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
/*
int beginSlitPlugin(HINSTANCE hMainInstance, HWND hBBSlit)
{
	/* Since we were loaded in the slit we need to remember the Slit
	 * HWND and make sure we remember that we are in the slit ;)
	 
	inSlit = true;
	hSlit = hBBSlit;

	// Start the plugin like normal now..
	return beginPlugin(hMainInstance);
}
*/
//=======================
void setStatus()
{

	//check for windows 2000 or higher before using transparency
//		if(!inSlit)
//					{
						if (transparency && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
						/*	if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBMessageBox, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBMessageBox, 0x202020, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
						*///	{
							SetWindowLong(hwndBBMessageBox, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBMessageBox, NULL, (unsigned char)alpha, LWA_ALPHA);
							//}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
						//	if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						//	{
						//		SetWindowLong(hwndBBMessageBox, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
						//		BBSetLayeredWindowAttributes(hwndBBMessageBox, 0x202020, (unsigned char)alpha, LWA_COLORKEY);
						//	}
						//	else
							SetWindowLong(hwndBBMessageBox, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					//}
					//else if((transparency)||(fullTrans)) SetWindowLong(hwndBBMessageBox, GWL_EXSTYLE, WS_EX_TOOLWINDOW);



}
/*
void showResult()

{
	
	int ra;
	srand(GetTickCount());
	ra = (int)rand();
	ra=(int)((ra/32767.0)*20.0);
	//strcpy(resmsg, "f");

	switch (ra)
	{
	case 0: {strcpy(resmsg, "Signs point to yes.");break;}
	case 1: {strcpy(resmsg, "Yes.");break;}
	case 2: {strcpy(resmsg, "Reply hazy, try again.");break;}
	case 3: {strcpy(resmsg, "Without a doubt.");break;}
	case 4: {strcpy(resmsg, "My sources say no.");break;}
	case 5: {strcpy(resmsg, "As I see it, yes. ");break;}
	case 6: {strcpy(resmsg, "You may rely on it.");break;}
	case 7: {strcpy(resmsg, "Concentrate and ask again.");break;}
	case 8: {strcpy(resmsg, "Outlook not so good.");break;}
	case 9: {strcpy(resmsg, "It is decidedly so.");break;}
	case 10: {strcpy(resmsg, "Better not tell you now. ");break;}
	case 11: {strcpy(resmsg, "Very doubtful.");break;}
	case 12: {strcpy(resmsg, "Yes - definitely.");break;}
	case 13: {strcpy(resmsg, "It is certain.");break;}
	case 14: {strcpy(resmsg, "Cannot predict now.");break;}
	case 15: {strcpy(resmsg, "Most likely.");break;}
	case 16: {strcpy(resmsg, "Ask again later. ");break;}
	case 17: {strcpy(resmsg, "My reply is no.");break;}
	case 18: {strcpy(resmsg, "Outlook good.");break;}
	case 19: {strcpy(resmsg, "Don't count on it.");break;}
	}

//	MessageBox(hwndBlackbox, resmsg, "BBMessageBox Result", MB_OK | MB_ICONINFORMATION);
//	CreateMessageBox();
/*	CMessageBox box(hwndBBMessageBox,				
								_T(resmsg),					
								_T("BBMessageBox Result"),			
								MB_OK | MB_SETFOREGROUND);

				box.SetIcon(IDI_8BALL, hInstance);
				box.DoModal();
}*/

/*int CreateMessageBox()
{
	DestroyMessageBox();
	WNDCLASS mc;
	ZeroMemory(&mc,sizeof(mc));
	mc.lpfnWndProc = MesProc;			// our window procedure
	mc.hInstance = hInstance;		// hInstance of .dll
	mc.lpszClassName = "Message";		// our window class name
	if (!RegisterClass(&mc)) 
	{
		MessageBox(hwndBlackbox, "Error registering window class", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	hwndMessage = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						"Message",										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP ,	// window parameters
						ScreenWidth/2 - 100,											// x position
						ScreenHeight/2 - 50,											// y position
						200,											// window width
						100,											// window height
						hwndBBMessageBox,											// parent window
						NULL,											// no menu
						hInstance,								// hInstance of .dll
						NULL);
	if (!hwndMessage)
	{
		UnregisterClass("Message", hInstance);
		MessageBox(0, "Error creating a window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}


	SetWindowPos(hwndMessage, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	
	ShowWindow(hwndMessage, SW_SHOW);

	InvalidateRect(hwndMessage, NULL, true);
	return 0;

}

void DestroyMessageBox()
{
	DestroyWindow(hwndMessage);
	UnregisterClass("Message", hInstance);
}
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{		
		// Window update process...
		case WM_PAINT:
		{
			// Create buffer hdc's, bitmaps etc.
			PAINTSTRUCT ps;  RECT r,rec;

			//get screen buffer
            HDC hdc_scrn = BeginPaint(hwnd, &ps);

			//retrieve the coordinates of the window's client area.
			GetClientRect(hwnd, &r);
			rec = r;
			

			//to prevent flicker of the display, we draw to memory first,
            //then put it on screen in one single operation. This is like this:

            //first get a new 'device context'
			HDC hdc = CreateCompatibleDC(NULL);

			
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc_scrn, r.right, r.bottom);
            SelectObject(hdc, bufbmp);
			


		//	if(!inSlit)
		//	{
				//Make background gradient
		//		if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit))
		//		{

				MakeGradient(hdc, r, myStyleItem->type,
							backColor, backColorTo,
							myStyleItem->interlaced,
							myStyleItem->bevelstyle,
							myStyleItem->bevelposition,
							bevelWidth, borderColor,
							borderWidth);
		//		}
		//	}
			

			SetBkMode(hdc, TRANSPARENT);

		//Paint second background according to toolbar.label.color: and toolbar.label.colorto:
		//	if(!labelIsPR || inSlit)
		//	{
		//		if(!inSlit)
		//		{
					r.left = r.left + (bevelWidth + borderWidth);
					r.top = r.top + (bevelWidth + borderWidth);
					r.bottom = (r.bottom - (bevelWidth + borderWidth));
					r.right = (r.right - (bevelWidth + borderWidth));
		//		}
						
				//draw second background according to toolbar.label.color: and toolbar.label.colorto:
			rec = r;
			r.bottom = r.top + 18;

//			if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit))
//				{
				
/*				MakeGradient(hdc, r, toolbar->type,
							toolbarColor, toolbarColorTo,
							toolbar->interlaced,
							toolbar->bevelstyle,
							toolbar->bevelposition,
							bevelWidth, borderColor, 0); 
//*///				}
		//	}

			r.bottom = rec.bottom;
			r.top = r.top + bevelWidth + 18;

			MakeGradient(hdc, r, myStyleItem2->type,
							backColor2, backColorTo2,
							myStyleItem2->interlaced,
							myStyleItem2->bevelstyle,
							myStyleItem2->bevelposition,
							bevelWidth, borderColor, 0); 

			r.top = rec.top;

			
			HGDIOBJ otherfont = CreateFont( 16,   //pBBCalendar->text.fontSize,
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace);
		    SelectObject(hdc, otherfont);
			SetTextColor(hdc, toolbarfontColor);
			DrawText(hdc, dcaption, -1, &r, DT_RIGHT | DT_TOP | DT_SINGLELINE);
					
			
	
			//Paint to the screen 
			r.top += 18;
			r.bottom -=23;
			SetTextColor(hdc, fontColor);
			DrawText(hdc, dtext, -1, &r, DT_CENTER | DT_VCENTER);
			r.bottom +=23;
			r.left = r.right - 53;
			r.right = r.right - 3;
			r.top = r.bottom - 23;
			r.bottom = r.bottom - 3;

			okRect = r;

				MakeGradient(hdc, r, myStyleItem->type,
							backColor, backColorTo,
							myStyleItem->interlaced,
							myStyleItem->bevelstyle,
							myStyleItem->bevelposition,
							bevelWidth, borderColor,
							borderWidth);

			
				r.left = r.left + (bevelWidth + borderWidth);
					r.top = r.top + (bevelWidth + borderWidth);
					r.bottom = (r.bottom - (bevelWidth + borderWidth));
					r.right = (r.right - (bevelWidth + borderWidth));
			
			if (!pressed){	MakeGradient(hdc, r, button->type,
							buttonColor, buttonColorTo,
							button->interlaced,
							button->bevelstyle,
							button->bevelposition,
							bevelWidth, borderColor, 0); 
							SetTextColor(hdc, buttonfontColor);
			}
			else       {	MakeGradient(hdc, r, buttonpr->type,
							buttonprColor, buttonprColorTo,
							buttonpr->interlaced,
							buttonpr->bevelstyle,
							buttonpr->bevelposition,
							bevelWidth, borderColor, 0); 
							SetTextColor(hdc, buttonprfontColor);
			}
			DrawText(hdc, "OK", -1, &r, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
		

			DeleteObject(otherfont);
			
			GetClientRect(hwnd, &r);
			


			BitBlt(hdc_scrn, 0, 0, r.right, r.bottom, hdc, 0, 0, SRCCOPY);

//			DeleteObject(hIcon);
		//	DeleteObject(hPen);
			DeleteDC(hdc_scrn);   
			DeleteDC(hdc);         //gdi: first delete the dc
			DeleteObject(bufbmp);  //gdi: now the bmp is free
			
			//takes care of hdc_scrn
			EndPaint(hwnd, &ps);
					
			return 0;
		}
		break;

		// If Blackbox sends a reconfigure message, load the new style settings and update the window...
		case BB_RECONFIGURE:
		{
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
			GetStyleSettings();
			InvalidateRect(hwndBBMessageBox, NULL, true);
		}
		break;

		// ==========

		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);

        	if (!_stricmp(szTemp, "@BBMessageBoxAbout"))
			{
				
				sprintf(dcaption,szAppName);
				sprintf(dtext, "%s\n\n%s ©2004 \n%s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);
				ShowWindow(hwndBBMessageBox, SW_SHOW);
				InvalidateRect(hwndBBMessageBox, NULL, true);
				/*CMessageBox box(hwndBBMessageBox,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				*/
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency; 
			
			setStatus();
			InvalidateRect(hwndBBMessageBox, NULL, true);
			}

			else if (!_stricmp(szTemp, "@BBMessageBoxSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBMessageBoxReloadSettings"))
			{
					//Re-initialize
					ReadRCSettings();
					InitBBMessageBox();
					GetStyleSettings();
					
					setStatus();

					SetWindowPos( hwndBBMessageBox, HWND_TOPMOST, xpos, ypos, 200, 100, SWP_NOACTIVATE);
					InvalidateRect(hwndBBMessageBox, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BBMessageBoxSaveSettings"))
			{
				WriteRCSettings();
			}
	

			else if (!_strnicmp(szTemp, "@BBMessageBoxSetTransparent", 27))
			{
				alpha = atoi(szTemp + 28);
			
				
				if (transparency) setStatus();
							
				
				InvalidateRect(hwndBBMessageBox, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBMessageBoxCaption", 19))
			{
				strcpy(dcaption,szTemp + 21);
				
			}
			else if (!_strnicmp(szTemp, "@BBMessageBox", 12))
			{
				strcpy(dtext,szTemp + 14);
				ShowWindow(hwndBBMessageBox, SW_SHOW);
				InvalidateRect(hwndBBMessageBox, NULL, true);
				
			}

		}
		break;

		// ==========
	
		case WM_WINDOWPOSCHANGING:
		{
				// Snap window to screen edges (if the last bool is false it uses the current DesktopArea)
				if(IsWindowVisible(hwnd)) SnapWindowToEdge((WINDOWPOS*)lParam, 10, true);
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;
				xpos = windowpos->x;
				ypos = windowpos->y;
		}
		break;

		// ==========
		// Allow window to move if the cntrl key is pressed...
		case WM_NCHITTEST:
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
				return HTCAPTION;
			else
				return HTCLIENT;
		}
		break;
		
		case WM_LBUTTONDOWN: 
		{
			POINT poloha;
			RECT windowRect;
			GetCursorPos(&poloha);
			GetWindowRect(hwndBBMessageBox,&windowRect);
			poloha.x -= windowRect.left;
			poloha.y -= windowRect.top;
			
			
			if (PtInRect(&okRect,poloha)) {
				pressed = true;
				InvalidateRect(hwndBBMessageBox, NULL, true);
				//DestroyMessageBox();
			//	pressed = false;
			}
		//	showResult();
		}
		break;
		// ==========
		case WM_LBUTTONUP: 
		{
			POINT poloha;
			RECT windowRect;
			GetCursorPos(&poloha);
			GetWindowRect(hwndBBMessageBox,&windowRect);
			poloha.x -= windowRect.left;
			poloha.y -= windowRect.top;
			
			
			if (PtInRect(&okRect,poloha)) {
				//pressed = true;
				//InvalidateRect(hwndMessage, NULL, true);
			//	DestroyMessageBox();
				ShowWindow( hwndBBMessageBox, SW_HIDE);
				pressed = false;
			}
		//	showResult();
		}
		break;
		
		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

			//Now we define all menus and submenus
	
			generalConfigSubmenu = MakeMenu("General");
		//	MakeMenuItem(generalConfigSubmenu, "Hide mode", "@BBMessageBoxHideMode", hide);
		//	if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBMessageBoxSlit", wantInSlit);
		//	MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBMessageBoxPluginToggle", pluginToggle);
		//	MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBMessageBoxOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBMessageBoxTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBMessageBoxSetTransparent",alpha,0,255);
	    //	if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
		//		MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBMessageBoxFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBMessageBoxSnapToEdge", snapWindow);
	
			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBMessageBoxEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBMessageBoxReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBMessageBoxSaveSettings", false);
			
			//attach defined menus together
			myMenu = MakeMenu("BBMessageBox 1.0");
		//	MakeMenuItem(myMenu, "Predict...", "@BBMessageBoxPredict", false);
			MakeSubmenu(myMenu, generalConfigSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About...", "@BBMessageBoxAbout", false);
			

			// Finally, we show the menu...
			ShowMenu(myMenu);
		}
		break;
	
		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}
