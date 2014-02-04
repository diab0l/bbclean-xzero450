/*
 ============================================================================
 Blackbox for Windows: Plugin BB8Ball 1.0 by Miroslav Petrasko [Theo] made
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

#include "BB8Ball.h"
#include "MessageBox.h"
#include "resource.h"


LPSTR szAppName = "BB8Ball";		// The name of our window class, etc.
LPSTR szVersion = "BB8Ball v1.0";	// Used in MessageBox titlebars

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
		MessageBox(hwndBlackbox, "Error registering window 8 class", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	

	// Get plugin and style settings...
	ReadRCSettings();
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBB8Ball();
	GetStyleSettings();
	width = height = 32+2*(borderWidth+bevelWidth);
	
	// Create the window...
	hwndBB8Ball = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						szAppName,										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP,	// window parameters
						xpos,											// x position
						ypos,											// y position
						width,											// window width
						height,											// window height
						NULL,											// parent window
						NULL,											// no menu
						hPluginInstance,								// hInstance of .dll
						NULL);
	if (!hwndBB8Ball)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating 8 window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}
	
	
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBB8Ball);
	else inSlit = false;
	
	setStatus();

	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBB8Ball, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...

  const long magicDWord = 0x49474541;
#if !defined _WIN64
  // Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
  SetWindowLong(hwndBB8Ball, GWL_USERDATA, magicDWord);
#else
  SetWindowLongPtr(hwndBB8Ball, GWLP_USERDATA, magicDWord);
#endif


	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBB8Ball, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...

	ShowWindow(hwndBB8Ball, SW_SHOW);

//	SetWindowPos(hwndMessage, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	
//	ShowWindow(hwndMessage, SW_SHOW);
	
	if (hide) ShowWindow( hwndBB8Ball, SW_HIDE);

	InvalidateRect(hwndBB8Ball, NULL, true);

///	BBMessageBox *m = new BBMessageBox(hwndBB8Ball,hPluginInstance,"ss","ss");
//	m->DoModal();
//	delete m;
//	CreateMessageBox();
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BB8BallReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBB8Ball()
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
	DestroyMessageBox();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...
	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBB8Ball, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBB8Ball);
	// Destroy our window...
	DestroyWindow(hwndBB8Ball);
	
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
			//if(!labelIsPR || inSlit)
			//{
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
			//}

			
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
			InvalidateRect(hwndBB8Ball, NULL, true);
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
				ShowWindow( hwndBB8Ball, SW_SHOW);
				InvalidateRect( hwndBB8Ball, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBB8Ball, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BB8BallHideMode"))
			{
				if (hide)
				{ hide = false; ShowWindow( hwndBB8Ball, SW_SHOW);
				InvalidateRect( hwndBB8Ball, NULL, true);}
				else
				{hide = true; ShowWindow( hwndBB8Ball, SW_HIDE);}
			}
			else if (!_stricmp(szTemp, "@BB8BallAbout"))
			{
				
				
				sprintf(resmsg, "%s\n\n%s ©2004 \n%s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);		
				strcpy(dcaption,szAppName);
				fortunes = false;
				CreateMessageBox();
/*
				CMessageBox box(hwndBB8Ball,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
*/				
			}
			else if (!_stricmp(szTemp, "@BB8BallPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BB8BallOnTop"))
			{
				// Hide window...
				alwaysOnTop = !alwaysOnTop;
				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBB8Ball, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBB8Ball, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BB8BallSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBB8Ball);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBB8Ball);
				else
					inSlit = false;	

				
				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBB8Ball, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BB8BallTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency; 
			
			setStatus();
			InvalidateRect(hwndBB8Ball, NULL, true);
			}

			else if (!_stricmp(szTemp, "@BB8BallAPossition"))
			{
				// Set the transparent attributes to the window
			apossition = !apossition; 
			}

			else if (!_stricmp(szTemp, "@BB8BallFullTrans"))
			{
				// Set the transparent attributes to the window
				fullTrans = !fullTrans; 
				
			setStatus();
			InvalidateRect(hwndBB8Ball, NULL, true);
			}



			else if (!_stricmp(szTemp, "@BB8BallSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BB8BallPredict")) 
			{
				showResult();
			}
			else if (!_stricmp(szTemp, "@BB8BallFortune")) 
			{
				showFortune();
			}

		/*	else if (!_stricmp(szTemp, "@BB8BallStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBB8Ball, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BB8BallStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBB8Ball, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BB8BallStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBB8Ball, NULL, true);
			}
		*/	else if (!_stricmp(szTemp, "@BB8BallEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BB8BallReloadSettings"))
			{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBB8Ball);

					//Re-initialize
					ReadRCSettings();
					InitBB8Ball();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBB8Ball, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBB8Ball, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBB8Ball);
					else inSlit = false;

					InvalidateRect(hwndBB8Ball, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BB8BallSaveSettings"))
			{
				WriteRCSettings();
			}
	

	/*		else if (!_strnicmp(szTemp, "@BB8BallSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
			
				
				if (transparency) setStatus();
							
				
				InvalidateRect(hwndBB8Ball, NULL, true);
			}
*/
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
				MoveWindow(hwndBB8Ball, xpos, ypos, width, height, true);
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

		case WM_RBUTTONUP: 
		{
			showFortune();
		}
		break;
		
		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

			//Now we define all menus and submenus
			
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			MakeMenuItem(generalConfigSubmenu, "Hide mode", "@BB8BallHideMode", hide);
			MakeMenuItem(generalConfigSubmenu, "Auto Message Pos.", "@BB8BallAPossition", apossition);
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BB8BallSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BB8BallPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BB8BallOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BB8BallTransparent", transparency);
	/*		if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BB8BallSetTransparent",alpha,0,255);
	*/		if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BB8BallFullTrans", fullTrans);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BB8BallSnapToEdge", snapWindow);
	
			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BB8BallEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BB8BallReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BB8BallSaveSettings", false);
			
			//attach defined menus together
			myMenu = MakeMenu("BB8Ball 1.0");
			MakeMenuItem(myMenu, "Predict...", "@BB8BallPredict", false);
			MakeMenuItem(myMenu, "Fortune...", "@BB8BallFortune", false);
			MakeSubmenu(myMenu, generalConfigSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About...", "@BB8BallAbout", false);
			

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
	
	char tempstyle2[MAX_LINE_LENGTH];
	
	
 //if(StrStrI(windowStyle, "label") != NULL  && strlen(windowStyle) < 6)
//	{
		// ...gradient type, bevel etc. from toolbar.label:(using a StyleItem)...
	//	char tempstyle2[MAX_LINE_LENGTH];
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
		
	
	//clock

		strcpy(tempstyle2, ReadString(stylepath, "toolbar.clock:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (clock) delete clock;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			clock = new StyleItem;			
			ParseItem(tempstyle2, clock);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.clock.color:", "")))
				clockColor = ReadColor(stylepath, "toolbar.clock.color:", "#000000");
			else
    			clockColor = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.clock.colorTo:", "")))
				clockColorTo = ReadColor(stylepath, "toolbar.clock.colorTo:", "#000000");
			else
				clockColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			clockfontColor = ReadColor(stylepath, "toolbar.clock.textColor:", "#FFFFFF");
		}
		else
		{
			if (clock) delete clock;	//else use the the toolbar: settings
			clock = new StyleItem;
			ParseItem(tempstyle, clock);	//use original tempstyle if "parentrelative"
			clockColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			clockColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			clockfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}

	//label
	
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.label:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (label) delete label;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			label = new StyleItem;			
			ParseItem(tempstyle2, label);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.label.color:", "")))
				labelColor = ReadColor(stylepath, "toolbar.label.color:", "#000000");
			else
    			labelColor = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.label.colorTo:", "")))
				labelColorTo = ReadColor(stylepath, "toolbar.label.colorTo:", "#000000");
			else
				labelColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			labelfontColor = ReadColor(stylepath, "toolbar.label.textColor:", "#FFFFFF");
		}
		else
		{
			if (label) delete label;	//else use the the toolbar: settings
			label = new StyleItem;
			ParseItem(tempstyle, label);	//use original tempstyle if "parentrelative"
			labelColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			labelColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			labelfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}

//winlabel
			
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.windowLabel:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (winlabel) delete winlabel;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			winlabel = new StyleItem;			
			ParseItem(tempstyle2, winlabel);
			
			if (!IsInString("", ReadString(stylepath, "toolbar.windowLabel.color:", "")))
				winlabelColor = ReadColor(stylepath, "toolbar.windowLabel.color:", "#000000");
			else
    			winlabelColor = ReadColor(stylepath, "toolbar.color:", "#FFFFFF");

			if (!IsInString("", ReadString(stylepath, "toolbar.windowLabel.colorTo:", "")))
				winlabelColorTo = ReadColor(stylepath, "toolbar.windowLabel.colorTo:", "#000000");
			else
				winlabelColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#000000");
			
			winlabelfontColor = ReadColor(stylepath, "toolbar.windowLabel.textColor:", "#FFFFFF");
		}
		else
		{
			if (winlabel) delete winlabel;	//else use the the toolbar: settings
			winlabel = new StyleItem;
			ParseItem(tempstyle, winlabel);	//use original tempstyle if "parentrelative"
			winlabelColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			winlabelColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			winlabelfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}

	  //button	
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

		//buttonpr
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
			
			buttonprfontColor = ReadColor(stylepath, "toolbar.button.pressed.textColor:", "#FFFFFF");
		}
		else
		{
			if (buttonpr) delete buttonpr;	//else use the the toolbar: settings
			buttonpr = new StyleItem;
			ParseItem(tempstyle, buttonpr);	//use original tempstyle if "parentrelative"
			buttonprColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			buttonprColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			buttonprfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		}
	
		//toolbar
			if (toolbar) delete toolbar;	//else use the the toolbar: settings
			toolbar = new StyleItem;
			ParseItem(tempstyle, toolbar);	//use original tempstyle if "parentrelative"
			toolbarColor = backColor;			//have to do this if parent relative found, it seems bb4win uses
			toolbarColorTo = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			toolbarfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		
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

	// First we look for the config file in the same folder as the plugin...
	GetModuleFileName(hInstance, rcpath, sizeof(rcpath));
	nLen = strlen(rcpath) - 1;
	while (nLen >0 && rcpath[nLen] != '\\') nLen--;
	rcpath[nLen + 1] = 0;
	strcpy(temp, rcpath);
	strcpy(path, rcpath);
	strcpy(fpath, rcpath);

	strcat(fpath,"fortune.rc");

	strcat(temp, "BB8Ball.rc");
	strcat(path, "BB8Ballrc");
	strcpy(windowStyle, "windowlabel");
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
		strcat(temp, "BB8Ball.rc");
		strcat(path, "BB8Ballrc");
		if (FileExists(temp)) strcpy(rcpath, temp);
		else if (FileExists(path)) strcpy(rcpath, path);
		else // If no config file was found, we use the default path and settings, and return
		{
			strcpy(rcpath, defaultpath);
			xpos = 10;
			ypos = 10;
			mxpos = ScreenWidth/2 - 100;
			mypos = ScreenHeight/2 - 50;
			alpha = 160;
			wantInSlit = true;
			alwaysOnTop = true;
			snapWindow = true;
			transparency = false;
			fullTrans = false;
			pluginToggle = false;
			hide = false;
			WriteRCSettings();
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "BB8Ball.x:", 10);
	ypos = ReadInt(rcpath, "BB8Ball.y:", 10);
	mxpos = ReadInt(rcpath, "BB8Ball.message.x:", 10);
	mypos = ReadInt(rcpath, "BB8Ball.message.y:", 10);
//	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 10;
//	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 10;

	alpha = ReadInt(rcpath, "BB8Ball.alpha:", 160);
	if(alpha > 255) alpha = 255;
	if(ReadString(rcpath, "BB8Ball.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "BB8Ball.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "BB8Ball.alwaysOnTop:", true);
	snapWindow = ReadBool(rcpath, "BB8Ball.snapWindow:", true);
	transparency = ReadBool(rcpath, "BB8Ball.transparency:", false);
	fullTrans = ReadBool(rcpath, "BB8Ball.fullTrans:", false);
	alwaysOnTop = ReadBool(rcpath, "BB8Ball.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "BB8Ball.pluginToggle:", false);
	hide = ReadBool(rcpath, "BB8Ball.hideMode:", false);
	apossition = ReadBool(rcpath, "BB8Ball.message.apossition:", true);
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

		sprintf(szTemp, "! BB8Ball %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "BB8Ball.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "BB8Ball.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "BB8Ball.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(hide) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.hideMode: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "BB8Ball.message.x: %d\r\n", mxpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "BB8Ball.message.y: %d\r\n", mypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(apossition) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "BB8Ball.message.apossition: %s\r\n", temp);
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
								SetWindowLong(hwndBB8Ball, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBB8Ball, 0x202020, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBB8Ball, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBB8Ball, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBB8Ball, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBB8Ball, 0x202020, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBB8Ball, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBB8Ball, GWL_EXSTYLE, WS_EX_TOOLWINDOW);



}

void showFortune()
{

	int ra;
	srand(GetTickCount());
	ra = (int)rand();
	ra=(int)((ra/32767.0)*999.0);

	sprintf(szTemp,"%.3d:",ra);
	sprintf(dcaption,"BB8Ball Fortune n:%.3d",ra);
	strcpy(resmsg, ReadString(fpath, szTemp, "Nothing foud, maybe\nthe file isnt there?"));
	for (UINT i=0;i < strlen(resmsg); i++)
				if ((resmsg[i]=='\\') && ((resmsg[i+1]=='n')||(resmsg[i+1]=='N'))) {resmsg[i]=' ';resmsg[i+1]='\n';}
			
	fortunes = true;
	CreateMessageBox();
}

void showResult()

{
	
	int ra;
	srand(GetTickCount());
	ra = (int)rand();
	ra=(int)((ra/32767.0)*20.0);
	//strcpy(resmsg, "f");
	strcpy(dcaption,"BB8Ball Prediction");
	strcpy(resmsg,advice[ra]);
	fortunes = false;
//	MessageBox(hwndBlackbox, resmsg, "BB8Ball Result", MB_OK | MB_ICONINFORMATION);
	CreateMessageBox();
/*	CMessageBox box(hwndBB8Ball,				
								_T(resmsg),					
								_T("BB8Ball Result"),			
								MB_OK | MB_SETFOREGROUND);

				box.SetIcon(IDI_8BALL, hInstance);
				box.DoModal();
*/}

int CreateMessageBox()
{
	DestroyMessageBox();
	pressed = ompressed = false;
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


    //*************
	HDC hdc = CreateCompatibleDC(NULL);
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
	RECT sss;
	sss.right = 10;  sss.left = 0; sss.bottom = 10; sss.top = 0;
	DrawText(hdc, resmsg, -1, &sss, DT_CENTER | DT_VCENTER | DT_CALCRECT);
	DeleteObject(otherfont);
	DeleteDC(hdc);
	//*************

	hwndMessage = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						"Message",										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP ,	// window parameters
						(!apossition)?mxpos:ScreenWidth/2 - ((((sss.right - sss.left)+20)<200)?100:((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2)/2),
						// x position
						(!apossition)?mypos:ScreenHeight/2 - ((sss.bottom - sss.top)+60+(bevelWidth + borderWidth)*2)/2,											// y position
						(((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2)<200)?200:((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2),											// window width
						/*(((sss.bottom - sss.top)+40)<100)?100:*/((sss.bottom - sss.top)+60) + (bevelWidth + borderWidth)*2,											// window height
						hwndBB8Ball,											// parent window
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

LRESULT CALLBACK MesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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

			if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit))
				{
				
				MakeGradient(hdc, r, toolbar->type,
							toolbarColor, toolbarColorTo,
							toolbar->interlaced,
							toolbar->bevelstyle,
							toolbar->bevelposition,
							bevelWidth, borderColor, 0); 
				}
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
			r.top += bevelWidth + 20;
			r.bottom -=23;
			SetTextColor(hdc, fontColor);
			DrawText(hdc, resmsg, -1, &r, DT_CENTER | DT_VCENTER);
			r.bottom +=23;
			r.left = r.right - 53;
			r.right = r.right - 3;
			r.top = r.bottom - 23;
			r.bottom = r.bottom - 3;

			okRect = r;

		/*		MakeGradient(hdc, r, myStyleItem->type,
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
		*/	
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
		
			if (fortunes)
			{
			r = okRect;

			r.right = r.left - (bevelWidth + borderWidth);
			r.left = r.right - 75;
			omRect = r;

		/*	MakeGradient(hdc, r, myStyleItem->type,
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
*/
			if (!ompressed){	MakeGradient(hdc, r, button->type,
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
			DrawText(hdc, "One More", -1, &r, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			}
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


		// ==========

		case WM_WINDOWPOSCHANGING:
		{
			// Is SnapWindowToEdge enabled?
	//		if (!inSlit && snapWindow)
			//{
				// Snap window to screen edges (if the last bool is false it uses the current DesktopArea)
				if(IsWindowVisible(hwnd)) SnapWindowToEdge((WINDOWPOS*)lParam, 10, true);
			//}
		}
		break;

		case WM_WINDOWPOSCHANGED:
		{				
				if (!apossition)
				{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;
				mxpos = windowpos->x;
				mypos = windowpos->y;
				}
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
			GetWindowRect(hwndMessage,&windowRect);
			poloha.x -= windowRect.left;
			poloha.y -= windowRect.top;
			
			
			if (PtInRect(&okRect,poloha)) {
				pressed = true;
				InvalidateRect(hwndMessage, &okRect, true);
				//DestroyMessageBox();
			//	pressed = false;
			}
			if ((fortunes)&&(PtInRect(&omRect,poloha))) {
				ompressed = true;
				InvalidateRect(hwndMessage, &omRect, true);
				
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
			GetWindowRect(hwndMessage,&windowRect);
			poloha.x -= windowRect.left;
			poloha.y -= windowRect.top;
			
			
			if (PtInRect(&okRect,poloha)) {
				//pressed = true;
				//InvalidateRect(hwndMessage, NULL, true);
				pressed = false;
				DestroyMessageBox();
			}
			
			if ((fortunes)&&(PtInRect(&omRect,poloha))) {
				//pressed = true;
				//InvalidateRect(hwndMessage, NULL, true);
			//	DestroyMessageBox();
				ompressed = false;
				showFortune();
			}
		//	showResult();
		}
		break;
		
		// Right mouse button clicked?
		case WM_RBUTTONUP:
		{	
			
		}
		break;
		
	
		case WM_CHAR:
		{
			if(wParam==VK_RETURN)
			{	
			//	pressed = true;
			//	InvalidateRect(hwndMessage, &okRect, true);
				pressed = false;
				DestroyMessageBox();
				
			}
			if((fortunes)&&(wParam==VK_SPACE))
			{
			//	ompressed = true;
			//	InvalidateRect(hwndMessage, &omRect, true);
				ompressed = false;
				showFortune();
			}
		}
		break;

	
		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}
