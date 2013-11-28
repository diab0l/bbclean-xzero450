/*
 ============================================================================
 bbFoomp // The foobar2000 plugin for Blackbox for Windows!
 Copyright © 2004-2005 isidoros.passadis@gmail.com

 Credits and thanks:
 qwilk, without his help and code I would have never started!
 azathoth, nc-17, tres`ni and other channel regulars, coder or not,
 thanks for bearing with my (often stupid) questions and helping me out!
 Lots of thanks also goes to MrJukes for his jAmp plugin and song scrolling
 which I relied heavily on for help, and Azathoth and Geekmaster
 for their geekAmp and other amp plugins, from which many of the ideas for the
 code came.

 As of version 1.7 this source-code is public. Follow the GPL if you wish to
 redistribute any changes. Enjoy.
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
 Foomp 1.7 (August 12 2006) / pkt-zer0
	» Shadow support
	» User-defined button actions and images
	» Wider variety of styles to choose from
	» Adjustable scroll speed
	» Major code rewrite
 
 Foomp 1.6 (June 2 2006) / pkt-zer0
	» Just a few changes to keep it compatible with the new foobar 9.1 and Columns UI

 Foomp 1.5 (June 21 2005) / freeb0rn
	» Fixed some bugs
	» Fixed support for foobar 8.3 and new UI Columns--assholes keep changing the class name.
	» First public code release.

 Foomp 1.4 (July 28 2004) / freeb0rn
	» Fixed a bug in FooMegaMode.
	» Added customizable default text.

 Foomp 1.3 (July 26 2004) / freeb0rn
	» Change what toolbar element GetStyleSettings() references to draw
		the outer rectangle so that it may fit in with other plugins better.
	» Added left/right alignment for FooMegaMode.

 Foomp 1.2 (July 24 2004) / freeb0rn 
	» Changed the the color that the buttons/controls are drawn with 
		(from button.PicColor to label.textcolor)
	» Changed... well, alot.
		Organized some code.
		Ability to choose which style the inner rectangle is drawn in.
			(Which subsequently effects the font color).

 Foomp 1.1 (July 15 2004) / freeb0rn 
	» Added adjustable height value.
	» Added adjustable border-thickness value.
	» Fixed the rendering of the buttons at different heights.
	» Fixed the rendering of the scrolling text at different heights.

 Foomp 1.0 (July 15 2004) / freeb0rn 
	- Original release

 ============================================================================
*/

#include "bbfoomp.h"

LPSTR szAppName = "bbFoomp";			// The name of our window class, etc.
LPSTR szVersion = "bbFoomp 1.7";		// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.7";
LPSTR szInfoAuthor = "freeb0rn";
LPSTR szInfoRelDate = "FILL IN LATER";
LPSTR szInfoLink = "http://freeb0rn.com";
LPSTR szInfoEmail = "isidoros.passadis@gmail.com";
LPSTR szInfoUpdateURL = "http://desktopian.org/bb/plugins/plugins.txt";

//====================

bool SlitExists = false;

//===========================================================================

int beginPlugin(HINSTANCE hPluginInstance)
{
	if (!hwndSlit) return 0;

	WNDCLASS wc;
	hwndBlackbox = GetBBWnd();
	hInstance = hPluginInstance;

	// Register our window class...
	ZeroMemory(&wc,sizeof(wc));
	wc.lpfnWndProc = WndProc;			// our window procedure
	wc.hInstance = hPluginInstance;		// hInstance of .dll
	wc.lpszClassName = szAppName;		// our window class name
	if (!RegisterClass(&wc)) 
	{
		MessageBox(hwndBlackbox, "Error registering window class", szVersion, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Get plugin and style settings...
	ReadRCSettings();
	GetStyleSettings();

	
	// Tap into the FooClass! (It's variables are going to be used
	// throughout the plugins. Window, handle etc.)
	if(FooClass) delete FooClass;	//Make sure FooClass is deleted from previous instances.
	FooClass = new Finfo;

	// Create our window...
	hwndPlugin = CreateWindowEx(
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
	if (!hwndPlugin)
	{						   
		MessageBox(0, "Error creating window", szVersion, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndPlugin, (LPARAM)msgs);
	// Set window to accept doubleclicks...
	SetClassLong(hwndPlugin, GCL_STYLE, CS_DBLCLKS | GetClassLong(hwndPlugin, GCL_STYLE));
	// Make the plugin window sticky (= pin the window)...
	MakeSticky(hwndPlugin); 	

	// Should we dock to the slit?
	// *** Thanks to qwilk for all things slit! ***
	if (SlitExists && FooDockedToSlit) SendMessage(hwndSlit, SLIT_ADD, NULL, (LPARAM)hwndPlugin);

	// Set the update timer to 250 milliseconds...
	int UpdateInterval = 250;
	if (!SetTimer(hwndPlugin, BBFOOMP_UPDATE_TIMER, UpdateInterval, (TIMERPROC)NULL))
	{
		MessageBox(0, "Error creating update timer", szVersion, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Show our window and force update...
	FirstUpdate = true;
	UpdateTitle();
	ShowWindow(hwndPlugin, SW_SHOW);

	// These are the buttons: reverse, pause, play, stop, forward, playlist, open/add, volul, voldown
	// Here we set them to false, and if OnTop is set in the settings we enable it.
	for (int i = 0; i < NUM_BUTTONS; ++i) buttons[i].pressed = false;
	if (FooOnTop==true)
	{
		SetWindowPos(hwndPlugin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
		SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)"BBFoomp -> Always On Top enabled");
	}
	// This is a check to see if transparency exists in the settings and if so enable it.
	Transparency();


	return 0;
}

//====================

int beginPluginEx(HINSTANCE hPluginInstance, HWND hwndBBSlit)
{
	return beginSlitPlugin(hPluginInstance, hwndBBSlit);
}

//====================

int beginSlitPlugin(HINSTANCE hPluginInstance, HWND hwndBBSlit)
{
	SlitExists = true;
	hwndSlit = hwndBBSlit;

	return beginPlugin(hPluginInstance);
}

//===========================================================================

void endPlugin(HINSTANCE hPluginInstance)
{	

	// Write current position to the config file if *not* docked to the slit...
	if (!FooDockedToSlit)
	{
		WriteInt(rcpath, "bbfoomp.xpos:", xpos);
		WriteInt(rcpath, "bbfoomp.ypos:", ypos);

		WriteInt(rcpath, "bbfoomp.borderwidth:", BorderWidth);
	}
	else SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);

	// Writing of following vars:
	// FooMode (Mouse Over Mode), FooWidth (Width) and Height (Height).
	WriteInt(rcpath, "bbfoomp.displaytype:", FooMode);
	WriteInt(rcpath, "bbfoomp.FooWidth:", FooWidth);
	WriteInt(rcpath, "bbfoomp.height:", height);

	// Write custom button commands.
	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		FoompButton &b = buttons[i];
		char picname[100], cmdname[100], altcmdname[100];
		sprintf(picname,"bbfoomp.button%d.image:",i+1);
		sprintf(cmdname,"bbfoomp.button%d.command:",i+1);
		sprintf(altcmdname,"bbfoomp.button%d.altcommand:",i+1);
		WriteInt(rcpath, picname, b.type);
		WriteString(rcpath, cmdname, b.cmdarg);
		WriteString(rcpath, altcmdname, b.altcmdarg);
	}


	if (hwndSlit)
	{
		// Remove from slit...
		SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);
		// Kill update timer...
		KillTimer(hwndPlugin, BBFOOMP_UPDATE_TIMER);
		// Delete the main plugin menu if it exists (PLEASE NOTE: This takes care of submenus as well!)
		if (scMenu) DelMenu(scMenu);
		// Unregister Blackbox messages...
		SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndPlugin, (LPARAM)msgs);
		// Destroy our window...
		DestroyWindow(hwndPlugin);
		// Unregister window class...
		UnregisterClass(szAppName, hPluginInstance);
		// Delete used FooInfo...
		if (FooClass) delete FooClass;
	}
}

//===========================================================================

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
		case PLUGIN_RELEASE:
			return szInfoRelDate;	// Release date, preferably in yyyy-mm-dd format
		case PLUGIN_LINK:
			return szInfoLink;		// Link to author's website
		case PLUGIN_EMAIL:
			return szInfoEmail;		// Author's email
		case PLUGIN_BROAMS:			// List of bro@ms available to the end users
		{
			return
			"@bbfoomp About "
			"@bbfoomp Play_Pause"
			"@bbfoomp Stop"
			"@bbfoomp Next"
			"@bbfoomp Previous"
			"@bbfoomp Random"
			"@bbfoomp VolUp"
			"@bbfoomp VolDown";
		}
		case PLUGIN_UPDATE_URL:		// AutoUpdate URL
			return szInfoUpdateURL;

		// ==========

		default:
			return szVersion;		// Fallback: Plugin name + version, e.g. "MyPlugin 1.0"
	}
}

void show_foomp_menu()
{
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (scMenu) DelMenu(scMenu);

	scMenu = MakeMenu("bbFoomp");

	//	<<	BEGIN	-- Controls Submenu
	scSubMenu = MakeMenu("Controls");
	MakeMenuItem(scSubMenu, "Previous", "@bbfoomp Previous", false);
	MakeMenuItem(scSubMenu, "Play/Pause", "@bbfoomp Play_Pause", false);
	MakeMenuItem(scSubMenu, "Stop", "@bbfoomp Stop", false);
	MakeMenuItem(scSubMenu, "Next", "@bbfoomp Next", false);
	MakeMenuItem(scSubMenu, "Random", "@bbfoomp Random", false);
	MakeMenuItem(scSubMenu, "Open file", "@bbfoomp Open", false);
	MakeMenuItem(scSubMenu, "Add files", "@bbfoomp Add", false);
	if (FooClass->FooHandle) 
	{
		MakeMenuNOP(scSubMenu, ""); // Separator
		MakeMenuItem(scSubMenu, "Foobar > Off", "@bbfoomp FooOff", false);
	}
	MakeSubmenu(scMenu, scSubMenu, "Controls");
	//	>>	END		-- Controls Submenu

	//	<<	BEGIN	-- Playback Order Submenu
	scSubMenu = MakeMenu("Playback Order");
	MakeMenuItem(scSubMenu, "Default", "@bbfoomp Order_Default", false);
	MakeMenuItem(scSubMenu, "Random", "@bbfoomp Order_Random", false);
	MakeMenuItem(scSubMenu, "Repeat", "@bbfoomp Order_Repeat", false);
	MakeMenuItem(scSubMenu, "Repeat One", "@bbfoomp Order_RepeatOne", false);
	MakeSubmenu(scMenu, scSubMenu, "Playback Order");
	//	>>	END		-- Playback Order Submenu
	
	//	<<	BEGIN	-- Options Submenu
	scSubMenu = MakeMenu("Options");
	if (FooMode != 3) MakeMenuItem(scSubMenu, "Toggle Display Mode", "@bbfoomp ToggDispMode", false);
	if (FooMode != 3) MakeMenuItem(scSubMenu, "FooMouseOver Mode", "@bbfoomp ToggFooMode", (FooMode == 1));
	MakeMenuItem(scSubMenu, "FooMega Mode", "@bbfoomp ToggFooMega", (FooMode == 3));
	MakeMenuNOP(scSubMenu, ""); // Septarator
	MakeMenuItem(scSubMenu, "Edit RC Settings", "@bbfoomp EditSettings", false);
	MakeMenuItem(scSubMenu, "Read RC Settings", "@bbfoomp ReadSettings", false);
	MakeSubmenu(scMenu, scSubMenu, "Options");
	//	>>	END		-- Options Submenu

	MakeMenuNOP(scMenu, "");// Separator

	//	<<	BEGIN	-- Preferences Submenu
	scSubMenu = MakeMenu("Preferences");
		scSubMenu2 = MakeMenu("Inset Rectangle Style");
			MakeMenuItem(scSubMenu2, "Label", "@bbfoomp ChangeInnerStyle 1", (InnerStyleIndex == 1));
			MakeMenuItem(scSubMenu2, "Window Label", "@bbfoomp ChangeInnerStyle 2", (InnerStyleIndex == 2));
			MakeMenuItem(scSubMenu2, "Clock", "@bbfoomp ChangeInnerStyle 3", (InnerStyleIndex == 3));
			MakeMenuItem(scSubMenu2, "Toolbar", "@bbfoomp ChangeInnerStyle 4", (InnerStyleIndex == 4));
			MakeMenuItem(scSubMenu2, "Button", "@bbfoomp ChangeInnerStyle 5", (InnerStyleIndex == 5));
			MakeMenuItem(scSubMenu2, "Button.Pressed", "@bbfoomp ChangeInnerStyle 6", (InnerStyleIndex == 6));
			MakeSubmenu(scSubMenu, scSubMenu2, "Inset Rectangle Style");			
		scSubMenu2 = MakeMenu("Outer Rectangle Style");
			MakeMenuItem(scSubMenu2, "Label", "@bbfoomp ChangeOuterStyle 1", (OuterStyleIndex == 1));
			MakeMenuItem(scSubMenu2, "Window Label", "@bbfoomp ChangeOuterStyle 2", (OuterStyleIndex == 2));
			MakeMenuItem(scSubMenu2, "Clock", "@bbfoomp ChangeOuterStyle 3", (OuterStyleIndex == 3));
			MakeMenuItem(scSubMenu2, "Toolbar", "@bbfoomp ChangeOuterStyle 4", (OuterStyleIndex == 4));
			MakeMenuItem(scSubMenu2, "Button", "@bbfoomp ChangeOuterStyle 5", (OuterStyleIndex == 5));
			MakeMenuItem(scSubMenu2, "Button.Pressed", "@bbfoomp ChangeOuterStyle 6", (OuterStyleIndex == 6));
			MakeSubmenu(scSubMenu, scSubMenu2, "Outer Rectangle Style");			
		scSubMenu2 = MakeMenu("FooMegaMode Alignment");
			MakeMenuItem(scSubMenu2, "Buttons on Right", "@bbfoomp ChangeMegaAlign 1", (FooAlign == false));
			MakeMenuItem(scSubMenu2, "Buttons on Left", "@bbfoomp ChangeMegaAlign 2", (FooAlign == true));
		MakeSubmenu(scSubMenu, scSubMenu2, "FooMegaMode Alignment");
	MakeMenuItem(scSubMenu, "Shadows enabled", "@bbfoomp ToggleShadows", FooShadowsEnabled);
	MakeMenuItemInt(scSubMenu,"Scroll speed", "@bbfoomp ScrollSpeed", FooScrollSpeed, 1,10);
	if (SlitExists) MakeMenuItem(scSubMenu, "Docked to slit", "@bbfoomp ToggleDockedToSlit", FooDockedToSlit);
	if (!FooDockedToSlit) MakeMenuItem(scSubMenu, "Always on top", "@bbfoomp ToggleOnTop", FooOnTop);
	if (!FooDockedToSlit) MakeMenuItem(scSubMenu, "Transparency", "@bbfoomp ToggleTrans", FooTrans);
	MakeSubmenu(scMenu, scSubMenu, "Preferences");
	//	>>	END		-- Preferences Submenu
	MakeMenuItem(scMenu, "Readme", "@bbfoomp Readme", false);
	MakeMenuItem(scMenu, "About bbFoomp", "@bbfoomp About", false);
	ShowMenu(scMenu);
}

//===========================================================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{		
		// Window update process...
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HDC buf = CreateCompatibleDC(NULL);
			HDC src = CreateCompatibleDC(NULL);
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc, width, height);
			HBITMAP srcbmp = CreateCompatibleBitmap(hdc, 20, 15);
			HBITMAP oldbuf = (HBITMAP)SelectObject(buf, bufbmp);
			RECT r;

			//====================

			// Paint border+background according to the current style...
			GetClientRect(hwnd, &r);
			MakeStyleGradient(buf, &r, &OuterStyle, OuterStyle.bordered);
			
			int offset = OuterStyle.borderWidth + BorderWidth;
			r.left += offset;
			r.top += offset;
			r.right -= offset;
			r.bottom -= offset;
			 

		//******* Begin control button calculations
		// Paint pressed button-control buttons...
		if (DisplayMode == 2)
		{
			CalculateButtonPositions(r); //Now we know how where the buttons are in this rect.
			DispModeControls(r, buf);
		}
		// Here follows the DisplayMode = Title stuff. (Scrolling title, etc.)
		if	(!DisplayMode) {
			DisplayMode = 1;
		}

		if	(DisplayMode == 1 || DisplayMode == 3) 
		{
			// DisplayMode 3 (side-by-side support)
			if (FooMode == 3)
			{
				if (FooAlign == false) r.left = FooWidth;

				DisplayMode = 3;
				r.right = r.left + 140 + 18;
				DispModeControls(r, buf);
				int offset3 = 150 + 18;
				r.left = r.left + offset3;
				r.right = width - 4;

				if (FooAlign == false) 
				{
					r.left = BorderWidth+1;
					r.right = FooWidth - offset3;
				}
			}

			MakeStyleGradient(buf, &r, &InnerStyle, false);
			int offset2 = OuterStyle.borderWidth + 3;
			r.left += offset2;
			r.top += offset2;
			r.right -= offset2;
			r.bottom -= offset2;

			//====================
			FooClass->update(); // make sure the current data is updated
			strcpy(CurrentSong,FooClass->song_title);
			// ===== End of grabbing name and handle, time to draw the text.

			// Song title scrolling if songtitle > X characters as it will be clipped.
			if (FooMode == 3) 
			{
				if (FooAlign == true) r.left = r.left + 150 + 18;
				if (FooAlign == false) r.left = r.left;
			}
			static int txtRefX = r.left;
			char temp[512] = "";
			int textWidth;
			int ret;
			textWidth = r.right;
			r.right = r.right+2;
			RECT r2;
			SetRectEmpty(&r2);
			r.top = r.top + 1;
			r.top = height / 2 - 5;
			r.bottom = r.bottom+2;

			strcat(temp, FooClass->song_title);
			strcat(temp, jAmpScrollFiller);

			HFONT font =  CreateStyleFont((StyleItem *)GetSettingPtr(SN_TOOLBAR));
			HGDIOBJ oldfont = SelectObject(buf, font);
			SetBkMode(buf, TRANSPARENT);

			ret = DrawText(buf, temp, strlen(temp), &r2, DT_SINGLELINE | DT_CALCRECT);
			
			// Scroll the text if needed
			if ( (r2.right - r2.left + 118) > textWidth && strlen(FooClass->song_title) > 30) 
			{	
				SetTextAlign(buf,0);
				// Title text repeater...
					strcat(temp, FooClass->song_title);
					strcat(temp, jAmpScrollFiller);
				
				// The actual scroller and the sliding scroll pointer!
				if (DisplayMode == 1 && txtRefX <= (r2.left-(r2.right-10))) txtRefX = r.left;
				if (FooMode == 3 && txtRefX <= (r2.left-(r2.right-150))) txtRefX = r.left;
				txtRefX -= FooScrollSpeed;  // Scroll speed.
				r.left += 2;
				r.right -= 2; //NOTE: I added this
				r.bottom=r.bottom + 1;

				if (FooShadowsEnabled)
				{
					RECT srect;
					srect.bottom	= r.bottom + 1; 
					srect.left		= r.left;
					srect.right		= r.right; // No weird shadow artifacting on the right hand side.
					srect.top		= r.top + 1;
					SetTextColor(buf, GetShadowColor(InnerStyle));
					ExtTextOut(buf, (txtRefX+1), (srect.top), ETO_CLIPPED, &srect, temp, strlen(temp), NULL);
				}
				SetTextColor(buf, InnerStyle.TextColor);
				ExtTextOut(buf, (txtRefX), (r.top), ETO_CLIPPED, &r, temp, strlen(temp), NULL);
			}

			
			else // Normally draw the text since it doesn't need to scroll/get clipped.
			{
				r.top = height / 2 - 5;
				r.left = r.left + 1;
				
				if (FooShadowsEnabled)
				{
					RECT srect;
					srect.bottom	= r.bottom + 1; 
					srect.left		= r.left + 1;
					srect.right		= r.right + 1;
					srect.top		= r.top + 1;
					SetTextColor(buf, GetShadowColor(InnerStyle));
					DrawText(buf, FooClass->song_title, strlen(FooClass->song_title), &srect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
				}
				SetTextColor(buf, InnerStyle.TextColor);
				DrawText(buf, FooClass->song_title, strlen(FooClass->song_title), &r, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);
	
			}

			DeleteObject(SelectObject(buf, oldfont));
		}
			//====================

			// Copy from the paint buffer to the window...
			BitBlt(hdc, 0, 0, width, height, buf, 0, 0, SRCCOPY);

			DeleteDC(src);
			SelectObject(buf, oldbuf);
			DeleteDC(buf);
			DeleteObject(bufbmp);
			DeleteObject(srcbmp);
			DeleteObject(oldbuf);
			EndPaint(hwnd, &ps);
			return 0;
		}
		break;
		// ==========
		case WM_CLOSE:
			return 0;
		// ==========
		case BB_RECONFIGURE:
		{
			UpdatePosition(); // Get new settings and resize window if needed...
			if (FooDockedToSlit) SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
			InvalidateRect(hwndPlugin, NULL, false);
		}
		break;
		// ==========
		case WM_TIMER:
		{
			UpdateTitle();
			return 0;
		}
		break;
		// ==========
		case BB_BROADCAST:
		{
			char temp[MAX_LINE_LENGTH];
			strcpy(temp, (LPCSTR)lParam);

			//====================

			// Global bro@ms...

			if (!FooDockedToSlit)
			{
				if (!_stricmp(temp, "@BBShowPlugins"))
				{
					// Show window and force update...
					ShowWindow(hwndPlugin, SW_SHOW);
					InvalidateRect(hwndPlugin, NULL, true);
					break;
				}
				else if (!_stricmp(temp, "@BBHidePlugins"))
				{
					// Hide window...
					ShowWindow(hwndPlugin, SW_HIDE);
					break;
				}
			}

			//====================

			// bbfoomp bro@ms...

			if (IsInString(temp, "@bbfoomp"))
			{
				// bbFoomp internal commands (e.g. for the menu)...
				static char msg[MAX_LINE_LENGTH];
				char token1[4096], token2[4096], extra[4096];
				LPSTR tokens[2];
				tokens[0] = token1;
				tokens[1] = token2;

				token1[0] = token2[0] = extra[0] = '\0';
				BBTokenize (temp, tokens, 2, extra);

				// ==========
				if (!_stricmp(token2, "Readme"))
				{
					char path[MAX_LINE_LENGTH], directory[MAX_LINE_LENGTH];
					int nLen;
					// First we look for the readme file in the same folder as the plugin...
					GetModuleFileName(hInstance, path, sizeof(path));
					nLen = strlen(path) - 1;
					while (nLen >0 && path[nLen] != '\\') nLen--;
					path[nLen + 1] = 0;
					strcpy(directory, path);
					strcat(path, "bbFoomp.htm");
					if (FileExists(path)) BBExecute(GetDesktopWindow(), NULL, path, "", directory, SW_SHOWNORMAL, true);
				}

				// ==========
				else if (!_stricmp(token2, "About"))
				{
					char temp[MAX_LINE_LENGTH];
					strcpy(temp, szVersion);
					strcat(temp, "\n\n© 2005 isidoros.passadis@gmail.com\n\nhttp://freeb0rn.com/\n#bb4win on irc.freenode.net   ");
					MessageBox(0, temp, "About this plugin...", MB_OK | MB_ICONINFORMATION);
					break;
				}

				// ==========
				if (!_stricmp(token2, "EditSettings"))
				{
					char temp[MAX_LINE_LENGTH];
					GetBlackboxEditor(temp);				
					if (FileExists(rcpath)) BBExecute(GetDesktopWindow(), NULL, temp, rcpath, NULL, SW_SHOWNORMAL, true);
				}

				// ==========
				if (!_stricmp(token2, "ReadSettings"))
				{
					ReadRCSettings();
					UpdatePosition(); // Get new settings and resize window if needed...
					SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
					InvalidateRect(hwndPlugin, NULL, false);
				}

				// ==========
				else if (!_stricmp(token2, "Show_Hide"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Activate or hide\"" : "/command:\"foobar2000/Activate or hide\"", NULL, SW_SHOWNORMAL, false);
					SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========
				else if (!_stricmp(token2, "ToggDispMode"))
				{
					if (DisplayMode == 1) {
						DisplayMode = 2;
					}
					else {
						DisplayMode = 1;
					}
						UpdatePosition(); // Get new settings and resize window if needed...
						SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
						InvalidateRect(hwndPlugin, NULL, false);
					break;
				}

				// ==========
				else if (!_stricmp(token2, "ChangeInnerStyle"))
				{
					int val = atoi(extra);
					if (val > 0 && val <= 6)
					{
						WriteInt(rcpath, "bbfoomp.InnerStyle:", val);
						ReadRCSettings();
						UpdatePosition(); // Get new settings and resize window if needed...
						SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
						InvalidateRect(hwndPlugin, NULL, false);
					}

					break;
				}
				else if (!_stricmp(token2, "ChangeOuterStyle"))
				{
					int val = atoi(extra);
					if (val > 0 && val <= 6)
					{
						WriteInt(rcpath, "bbfoomp.OuterStyle:", val);
						ReadRCSettings();
						UpdatePosition(); // Get new settings and resize window if needed...
						SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
						InvalidateRect(hwndPlugin, NULL, false);
					}

					break;
				}

				// ==========
				else if (!_stricmp(token2, "ChangeMegaAlign"))
				{
					if(!_stricmp(extra, "1"))
					{
						FooAlign = false;
						WriteBool(rcpath, "bbfoomp.MegaLeftAlign:", FooAlign);
					}
					else if(!_stricmp(extra, "2"))
					{
						FooAlign = true;
						WriteBool(rcpath, "bbfoomp.MegaLeftAlign:", FooAlign);
					}
					ReadRCSettings();
					UpdatePosition(); // Get new settings and resize window if needed...
					SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
					InvalidateRect(hwndPlugin, NULL, false);

					break;
				}

				// ==========
				else if (!_stricmp(token2, "ToggFooMode"))
				{
					if (FooMode == 1) {
						FooMode = 2;
						UpdateTitle();
					}
					else {
						FooMode = 1;
						UpdateTitle();
					}
						UpdatePosition(); // Get new settings and resize window if needed...
						SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
						InvalidateRect(hwndPlugin, NULL, false);
					break;
				}

				// ==========
				else if (!_stricmp(token2, "ToggFooMega"))
				{
					if (width < 300) MessageBox(0, "Please assign a Width greater than 300\nin the bbfoomp.rc for this feature to work.", "ERROR: Feature Unusable at this Width", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
					else 
					{
						if (FooMode != 3) 
						{
							FooModePrev = FooMode;
						}
						if (FooMode == 1 || FooMode == 2) {
							FooMode = 3;
							UpdateTitle();
						}
						else {
							FooMode = FooModePrev;
							if (FooMode == 0) FooMode = 1;
							if (FooMode == 2) DisplayMode = 1;
							UpdateTitle();
						}		
							UpdatePosition(); // Get new settings and resize window if needed...
							SendMessage(hwndSlit, SLIT_UPDATE, NULL, NULL);
							InvalidateRect(hwndPlugin, NULL, false);
					}
					break;
				}

				// ==========

				else if (!_stricmp(token2, "ToggleDockedToSlit"))
				{
					ToggleDockedToSlit();
					break;
				}

				else if (!_stricmp(token2, "ToggleShadows"))
				{
					FooShadowsEnabled = !FooShadowsEnabled;
					WriteBool(rcpath, "bbfoomp.Shadows:", FooShadowsEnabled);
					break;
				}

				else if (!_stricmp(token2, "ScrollSpeed"))
				{
					int val = atoi(extra);
					if (val > 0 && val <= 10)
					{
						FooScrollSpeed = val;
						WriteInt(rcpath, "bbfoomp.ScrollSpeed:", FooScrollSpeed);
					}
					break;
				}

				else if (!_stricmp(token2, "ToggleOnTop"))
				{
					if (FooOnTop == true)
					{
						FooOnTop = false;
						SetWindowPos(hwndPlugin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
						SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)"BBFoomp -> Always On Top disabled");						
						WriteBool(rcpath, "bbfoomp.OnTop:", FooOnTop);

					}
					else
					{
						FooOnTop = true;
						SetWindowPos(hwndPlugin, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
						SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)"BBFoomp -> Always On Top enabled");
						WriteBool(rcpath, "bbfoomp.OnTop:", FooOnTop);

					}
					break;
				}
				
				else if (!_stricmp(token2, "ToggleTrans"))
				{
					if (usingWin2kXP)
					{
						if (FooTrans)
						{
							FooTrans = false;
							SetTransparency(hwndPlugin, 255);
							SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)"BBFoomp -> Transparency disabled");
							WriteBool(rcpath, "bbfoomp.transparency:", FooTrans);
						}
						else if (!FooDockedToSlit)
						{
							FooTrans = true;
							SetTransparency(hwndPlugin, (unsigned char)transparencyAlpha);
							SendMessage(hwndBlackbox, BB_SETTOOLBARLABEL, 0, (LPARAM)"BBFoomp -> Transparency enabled");
							WriteBool(rcpath, "bbfoomp.transparency:", FooTrans);
						}
					}
				}
				// ========== BEGIN CONTROLS BROAMS
				// You will note a few commented lines under each broam,
				// should you wish to uncomment them they will focus Foobar2000
				// whenever a command is given to it.
				else if (!_stricmp(token2, "VolUp"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Volume up\"" : "/command:\"Playback/Volume up\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				else if (!_stricmp(token2, "VolDown"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Volume down\"" : "/command:\"Playback/Volume down\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				else if (!_stricmp(token2, "Play_Pause"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/playpause", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ========== 
				else if (!_stricmp(token2, "Play"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/play", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========		
				else if (!_stricmp(token2, "Stop"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/stop", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========
				else if (!_stricmp(token2, "Previous"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/prev", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========
				else if (!_stricmp(token2, "Next"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/next", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========											
				else if (!_stricmp(token2, "Random"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/rand", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ==========											
				else if (!_stricmp(token2, "Add"))
				{
					SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)"@bbfoomp Show_Hide");
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Add files...\"" : "/command:\"Playlist/Add files...\"", NULL, SW_SHOWNORMAL, false);
					break;
				}

				// ==========											
				else if (!_stricmp(token2, "Open"))
				{
					SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)"@bbfoomp Show_Hide");
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Open...\"" : "/command:\"Playlist/Open...\"", NULL, SW_SHOWNORMAL, false);
					break;
				}

				// ==========					
				else if (!_stricmp(token2, "FooOff"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, "/exit", NULL, SW_SHOWNORMAL, false);
					break;
				}

				// ========== END CONTROLS BROAMS // BEGIN PLAYBACK ORDER BROAMS
													
				else if (!_stricmp(token2, "Order_Default"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Default\"" : "/command:\"Playback/Order/Default\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ========== 																	
				else if (!_stricmp(token2, "Order_Random"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Shuffle (tracks)\"" : "/command:\"Playback/Order/Random\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ========== 																				
				else if (!_stricmp(token2, "Order_Repeat"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Repeat (playlist)\"" : "/command:\"Playback/Order/Repeat\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ========== 
				else if (!_stricmp(token2, "Order_RepeatOne"))
				{
					BBExecute(GetDesktopWindow(), NULL, FooPath, foobar_v9 ? "/command:\"Repeat (track)\"" : "/command:\"Playback/Order/Repeat One\"", NULL, SW_SHOWNORMAL, false);
				//	SendMessage(GetBBWnd(), BB_BRINGTOFRONT, 0, (LPARAM)FooClass->FooHandle);
				//	SetForegroundWindow(FooClass->FooHandle);
					break;
				}

				// ========== END PLAYBACK ORDER BROAMS

				// ========== CUSTOM COMMAND BROAMS
				else if (!strnicmp(token2, "Press", 5))
				{
					int button_idx = atoi(token2+5);
					if (button_idx > 0 && button_idx < NUM_BUTTONS && buttons[button_idx-1].cmdarg[0])
					{
						BBExecute(GetDesktopWindow(), NULL, FooPath, buttons[button_idx-1].cmdarg, NULL, SW_SHOWNORMAL, false);
					}
					break;
				}
				else if (!strnicmp(token2, "AltPress", 8))
				{
					int button_idx = atoi(token2+8);
					if (button_idx > 0 && button_idx < NUM_BUTTONS && buttons[button_idx-1].altcmdarg[0])
					{
						BBExecute(GetDesktopWindow(), NULL, FooPath, buttons[button_idx-1].altcmdarg, NULL, SW_SHOWNORMAL, false);
					}
					break;
				}
			}
		}
		break;
		// ==========
		case WM_NCHITTEST:
		{
			if (!FooDockedToSlit && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) return HTCAPTION;
			else return HTCLIENT;
		}
		break;
		// ==========
		case WM_RBUTTONUP:
		case WM_NCRBUTTONUP:
		{	
			show_foomp_menu();
		}
		break;
		case WM_RBUTTONDOWN:
		case WM_NCRBUTTONDOWN: {} break;
		// ==========
		case WM_LBUTTONDBLCLK:
		{
			if (DisplayMode == 1 || DisplayMode == 3)
			{
				SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)"@bbfoomp Show_Hide");
			}
		}
		break;
		// ==========
		case WM_LBUTTONDOWN: 
		{
			TrackMouse();
            ClickMouse(LOWORD(lParam), HIWORD(lParam));
		}
		break;
		// ==========
		case WM_LBUTTONUP:
			{
				int i;
				for (i = 0; i < NUM_BUTTONS; ++i)
					if (buttons[i].pressed)
					{
						char buffer[128];
						if (GetAsyncKeyState(VK_MENU) & 0x8000)	sprintf(buffer,"@bbfoomp AltPress%d",i+1);
						else sprintf(buffer,"@bbfoomp Press%d",i+1);
						SendMessage(hwndPlugin, BB_BROADCAST, 0, (LPARAM)buffer);
					}
				
				for (i = 0; i < NUM_BUTTONS; ++i) buttons[i].pressed = false;
                InvalidateRect(hwndPlugin, NULL, false);
			}
		break;

		//====================

		case WM_XBUTTONUP:
		case WM_NCXBUTTONUP:
		{
			if (SlitExists) ToggleDockedToSlit();
		}
		break;

		case WM_XBUTTONDOWN:
		case WM_NCXBUTTONDOWN: {} break;

		// ==========

		case WM_MOUSELEAVE:
		{
			if (DisplayMode == 2 || DisplayMode == 3)
			{

				for (int i = 0; i < NUM_BUTTONS; ++i) buttons[i].pressed = false;
                InvalidateRect(hwndPlugin, NULL, false);
				if (FooMode == 1) // If 'mouseover' mode is on...
				{
					DisplayMode = 1;
					UpdateTitle();			
				}

			}

        }
        break;

		// ==========
		case WM_MOUSEMOVE:
		{	
			TrackMouse();
			if (FooMode == 1) // If 'mouseover' mode is on...
			{
				if (DisplayMode == 1)
				{
					DisplayMode = 2;
					UpdateTitle();			
				}
			}
		}
		break;

		//====================

		// Snap window to screen edges (or the currently defined DesktopArea)...
		case WM_WINDOWPOSCHANGING:
		{
			if (!FooDockedToSlit)
			{
				if (IsWindowVisible(hwnd)) SnapWindowToEdge((WINDOWPOS*)lParam, 10, true);
			}
		}
		break;

		//====================

		// Save window position if it changes...
		case WM_WINDOWPOSCHANGED:
		{
			if (!FooDockedToSlit)
			{
				WINDOWPOS* windowpos = (WINDOWPOS*)lParam;
				xpos = windowpos->x;
				ypos = windowpos->y;
			}
		}
		break;

		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

//===========================================================================

void UpdateTitle()
{
	bool UpdateDisplay = false;

	if (FirstUpdate)
	{
		UpdateDisplay = true;
		FirstUpdate = false;
	}

	// Only update toolbar if the windowname has changed.
	if (CurrentSong != DisplayedSong || UpdateDisplay)
	{
		// Setting the CurrentSong as the Displayed song for the next Update.
		strcpy(DisplayedSong,CurrentSong);
		UpdateDisplay = true;
	}

	if (UpdateDisplay) { InvalidateRect(hwndPlugin, NULL, false); }
}

//===========================================================================

void UpdatePosition()
{
	GetStyleSettings();
	MoveWindow(hwndPlugin, xpos, ypos, width, height, true);
	SetWindowPos(hwndPlugin, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	return;
}

//===========================================================================

void ToggleDockedToSlit()
{
	if (FooDockedToSlit)
	{
		SendMessage(hwndSlit, SLIT_REMOVE, NULL, (LPARAM)hwndPlugin);
		FooDockedToSlit = false;
		// Since position in slit != position as plugin we need to reload the position...
		xpos = ReadInt(rcpath, "bbfoomp.xpos:", 0);
		ypos = ReadInt(rcpath, "bbfoomp.ypos:", 0);
		if (FooTrans) SetTransparency(hwndPlugin, (unsigned char)transparencyAlpha);
		UpdatePosition();
	}
	else
	{
		// Since position in slit != position as plugin we need to
		// save the current position before docking to the slit...
		WriteInt(rcpath, "bbfoomp.xpos:", xpos);
		WriteInt(rcpath, "bbfoomp.ypos:", ypos);
		FooDockedToSlit = true;
		if (FooTrans) SetTransparency(hwndPlugin, (BYTE)255);
		SendMessage(hwndSlit, SLIT_ADD, NULL, (LPARAM)hwndPlugin);
	}
	WriteBool(rcpath, "bbfoomp.dockedtoslit:", FooDockedToSlit);

	static char msg[MAX_LINE_LENGTH];
	static char status[9];
	if (FooDockedToSlit) sprintf(msg, "bbfoomp -> Docked! (slit mode)", status);
	else sprintf(msg, "bbfoomp -> Undocked! (plugin mode)", status);
	SendMessage(GetBBWnd(), BB_SETTOOLBARLABEL, 0, (LPARAM)msg);
}

//===========================================================================

void GetStyleSettings()
{
	int style_assoc[] =
	{ 
		SN_TOOLBARLABEL,
		SN_TOOLBARWINDOWLABEL,
		SN_TOOLBARCLOCK,
	    SN_TOOLBAR,
		SN_TOOLBARBUTTON,
		SN_TOOLBARBUTTONP
	};
 
	StyleItem * os = (StyleItem *)GetSettingPtr(style_assoc[OuterStyleIndex-1]);
	StyleItem * is = (StyleItem *)GetSettingPtr(style_assoc[InnerStyleIndex-1]);
	StyleItem * toolbar = (StyleItem *)GetSettingPtr(SN_TOOLBAR);
	StyleItem * toolbarButtonPressed = (StyleItem *)GetSettingPtr(SN_TOOLBARBUTTONP);

	OuterStyle = os->parentRelative ? *toolbar : *os;
	InnerStyle = is->parentRelative ? *toolbar : *is;
	ButtonStyle = *toolbarButtonPressed;



	// Check if the title needs updating, and if so, update it.
	UpdateTitle();

//	SIZE size;
	HDC fonthdc = CreateDC("DISPLAY", NULL, NULL, NULL);
	HFONT font = CreateStyleFont(toolbar);
	HGDIOBJ oldfont = SelectObject(fonthdc, font);
//	GetTextExtentPoint32(fonthdc, foobarWnd, 32, &size);
	width = FooWidth;
	DeleteObject(SelectObject(fonthdc, oldfont));
	DeleteDC(fonthdc);
}

//===========================================================================
void TrackMouse()
{
	TRACKMOUSEEVENT track;
    ZeroMemory(&track,sizeof(track));
    track.cbSize = sizeof(track);
    track.dwFlags = TME_LEAVE;
    track.dwHoverTime = HOVER_DEFAULT;
    track.hwndTrack = hwndPlugin;
    TrackMouseEvent(&track);
}


// ==================
void CalculateButtonPositions(RECT r)
{
	// NOTE: merge into DispMode controls, perhaps?
	int i;
	// Generate X, Y coordinates for the shapes
	int xpos;
	int ypos;

	int sumWidth = 0;
	for (i = 0; i< NUM_BUTTONS; ++i)
		sumWidth += buttons[i].width();
	sumWidth += (NUM_BUTTONS-1) * button_spacing;
	
	// Position within rect.
	xpos= (r.right - r.left)/2 - (sumWidth/2);
	ypos= (r.bottom - r.top)/2 - 3;

	//Adjust position for global rect.
	xpos+= r.left;
	ypos+= r.top;

	
	for (i = 0; i < NUM_BUTTONS; ++i)
	{
		buttons[i].x = xpos;
		buttons[i].y = ypos;
		xpos += buttons[i].width() + button_spacing;
	}

	// Generate hit rectangles
	int rtop = r.top + 1;
	int rbottom = r.bottom - 1;

	for (i = 0; i < NUM_BUTTONS; ++i)
	{
		FoompButton &b = buttons[i];
		b.hitrect.top = rtop;
		b.hitrect.bottom = rbottom;
		int padding = (12 - b.width())/2;
		b.hitrect.left = b.x - padding;
		b.hitrect.right = b.x + b.width() + padding;
	}
}
// ==================
void DispModeControls(RECT r, HDC buf)
{
	int i;
	for (i = 0; i < NUM_BUTTONS; ++i)
		buttons[i].draw(buf);
}


//===========================================================================

void ClickMouse(int mouseX, int mouseY) 
{
	if (DisplayMode == 2 || DisplayMode == 3)
		for (int i = 0; i < NUM_BUTTONS; ++i)
			if (buttons[i].clicked(mouseX,mouseY)) 
			{
				buttons[i].pressed = true;
				InvalidateRect(hwndPlugin, NULL, false);
				return;
			}
}


//===========================================================================

void ReadRCSettings()
{
	// NOTE: make a 0.8.3 compatible RC file
	const char *default_commands[] =
	{
		"/prev",
		"/play",
		"/playpause",
		"/stop",
		"/next",
		"/command:\"Activate or hide\"",
		"/command:\"Volume up\"",
		"/command:\"Volume down\"",
		"/command:\"Open...\""
	};
	const char *default_altcommands[] =
	{
		"",
		"/rand",
		"",
		"",
		"",
		"",
		"",
		"",
		"/command:\"Add files...\""
	};


	char pluginDir[MAX_LINE_LENGTH];
	int nLen;

	// First we extract the plugin directory...
	GetModuleFileName(hInstance, pluginDir, sizeof(pluginDir));
	nLen = strlen(pluginDir) - 1;
	while (nLen >0 && pluginDir[nLen] != '\\') nLen--;
	pluginDir[nLen + 1] = 0;

	// ...then we search for the bbfoomp.rc config file...
	// (-> $UserAppData$\Blackbox -> plugin directory -> Blackbox directory)
	strcpy(rcpath, ConfigFileExists("bbfoomp.rc", pluginDir));
	if (!strlen(rcpath)) strcpy(rcpath, ConfigFileExists("bbfoomprc", pluginDir));
	if (!strlen(rcpath))
	{
		// If bbfoomp.rc could not be found we create a new
		// config file in the same folder as the plugin...
		strcpy(rcpath, pluginDir);
		strcat(rcpath, "bbfoomp.rc");
		WriteDefaultRCSettings();
	}

	//====================

	// Read bbfoomp settings from config file...
	strcpy(FooPath, ReadString(rcpath, "bbfoomp.foobar.path:", "C:\\Progra~1\\foobar2000\\foobar2000.exe"));
	strcpy(NoInfoText, ReadString(rcpath, "bbfoomp.DefaultText:", "Nothing is playing"));
	FooWidth = ReadInt(rcpath, "bbfoomp.foowidth:" , 200);
	height = ReadInt(rcpath, "bbfoomp.height:", 20);
	FooMode = ReadInt(rcpath, "bbfoomp.displaytype:", 2);
	InnerStyleIndex = ReadInt(rcpath, "bbfoomp.InnerStyle:", 2);
	OuterStyleIndex = ReadInt(rcpath, "bbfoomp.OuterStyle:", 4);
	FooOnTop = ReadBool(rcpath, "bbfoomp.OnTop:", false);
	transparencyAlpha = ReadInt(rcpath, "bbfoomp.transparencyAlpha:", 220);
	BorderWidth = ReadInt(rcpath, "bbfoomp.borderwidth:", 3);
	FooTrans = ReadBool(rcpath, "bbfoomp.transparency:", false);
	FooAlign = ReadBool(rcpath, "bbfoomp.MegaLeftAlign:", true);
	FooShadowsEnabled = ReadBool(rcpath, "bbfoomp.Shadows:", false);
	FooScrollSpeed = ReadInt(rcpath, "bbfoomp.ScrollSpeed:", 5);

	for (int i = 0; i < NUM_BUTTONS; ++i)
	{
		FoompButton &b = buttons[i];
		char picname[100], cmdname[100], altcmdname[100];
		sprintf(picname,"bbfoomp.button%d.image:",i+1);
		sprintf(cmdname,"bbfoomp.button%d.command:",i+1);
		sprintf(altcmdname,"bbfoomp.button%d.altcommand:",i+1);
		b.type = ButtonType(ReadInt(rcpath, picname, i));
		strcpy(b.cmdarg, ReadString(rcpath, cmdname, default_commands[i]));
		strcpy(b.altcmdarg, ReadString(rcpath, altcmdname, default_altcommands[i]));
	}

	xpos = ReadInt(rcpath, "bbfoomp.xpos:", 10);
	if (xpos >= GetSystemMetrics(SM_CXSCREEN)) xpos = 0;
	ypos = ReadInt(rcpath, "bbfoomp.ypos:", 10);
	if (ypos >= GetSystemMetrics(SM_CYSCREEN)) ypos = 0;
	

	if (SlitExists) FooDockedToSlit = ReadBool(rcpath, "bbfoomp.dockedtoslit:", false);
	else FooDockedToSlit = false;

	// Minimum settings checks.
	if (height < (15 + BorderWidth) || width < 0 || BorderWidth < 0)
	{
		MessageBox(0, "The value you have inputted for either: \nheight, width or border-width is below the minimum.\nThe values will default. Please consult the Readme for the minimums.", "ERROR: Illegal value set.", MB_OK | MB_TOPMOST | MB_SETFOREGROUND);
		FooWidth = 200;
		height = 22;
		BorderWidth = 3;
	}
}

//===========================================================================

void WriteDefaultRCSettings()
{
	static char szTemp[MAX_LINE_LENGTH];
	DWORD retLength = 0;

	// Create a new bbfoomp.rc configuration file with default settings...
	HANDLE file = CreateFile(rcpath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		strcpy(szTemp,
			"bbfoomp.foobar.path: C:\\Progra~1\\foobar2000\\foobar2000.exe\r\n" // Foo Directory [FooPath]
			"bbfoomp.displaytype: 2\r\n"										// FooMode (Mouse Over Mode) [FooMode]
			"bbfoomp.foowidth: 200\r\n"											// FooWidth [self-explanatory]
			"bbfoomp.height: 22\r\n"											// Height [self-explanatory]
			"bbfoomp.borderwidth: 3\r\n"										// Border width [self-explanatory]
			"bbfoomp.xpos: 0\r\n"												// Xpos [x-coordinate]
			"bbfoomp.ypos: 0\r\n"												// Ypos [y-coordinate]
			"bbfoomp.dockedtoslit: false\r\n"									// FooDockedToSlit [self-explanatory]
			"bbfoomp.OnTop: false\r\n"											// FooOnTop [Always On Top]
			"bbfoomp.transparency: false\r\n"									// FooTrans [Transparency]
			"bbfoomp.transparencyAlpha: 220\r\n"								// transparencyAlpha [Amount of Transparency]
			"bbfoomp.RectangleStyle: 2\r\n"										// Inset Rectangle Style [Style in which its drawn]
			"bbfoomp.MegaLeftAlign: true\r\n"									// MegaMode alignment [FooAlign]
			"bbfoomp.DefaultText: Nothing is playing\r\n");						// Text to show when nothing is playing [NoInfoText]

		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	}
	CloseHandle(file);
}

//===========================================================================

void Transparency()
{
	// Transparency is only supported under Windows 2000/XP...
	OSVERSIONINFO osInfo;
	ZeroMemory(&osInfo, sizeof(osInfo));
	osInfo.dwOSVersionInfoSize = sizeof(osInfo);
	GetVersionEx(&osInfo);

	if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && osInfo.dwMajorVersion == 5) 
		usingWin2kXP = true;
	else 
		usingWin2kXP = false;
	if (usingWin2kXP)
	{
		if (FooTrans && !FooDockedToSlit)
		{	
			SetTransparency(hwndPlugin, (unsigned char)255);
			SetTransparency(hwndPlugin, (unsigned char)transparencyAlpha);
		}
	}

}

//===========================================================================
COLORREF MakeShadowColor(StyleItem &style)
{
	int rav, gav, bav;
	if (style.type != B_SOLID)
	{
		rav = (GetRValue(style.Color)+GetRValue(style.ColorTo)) / 2;
		gav = (GetGValue(style.Color)+GetGValue(style.ColorTo)) / 2;
		bav = (GetBValue(style.Color)+GetBValue(style.ColorTo)) / 2;
	}
	else
	{
		rav = GetRValue(style.Color);
		gav = GetGValue(style.Color);
		bav = GetBValue(style.Color);
	}

	if (rav < 0x30) rav = 0;
	else rav -= 0x10;
	if (gav < 0x30) gav = 0;
	else gav -= 0x10;
	if (bav < 0x30) bav = 0;
	else bav -= 0x10;

	return RGB((BYTE)rav, (BYTE)gav, (BYTE)bav);
}
//===========================================================================
COLORREF GetShadowColor(StyleItem &style)
{
	return	(style.validated & VALID_SHADOWCOLOR) ?
				style.ShadowColor :
				MakeShadowColor(style);
}


//===========================================================================
void Finfo::update()
{
	// ===== Gets the Handle for FooBar and then uses that to get the windowname.
	// Here is where stuff gets tricky. Foobar by default has a really ugly handle,
	// and plugins such as Foo_UI Columns change the handle name! So we have to figure out
	// if UI Columns is being used and then re-grab the handle. Here goes!
	// *** NOTE: Current support for foobar 9.1 and Columns UI 0.1.3 beta 1v5
	strcpy(song_title,"");
	foobar_v9 = false;
	if (FooHandle = FindWindow("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}", NULL)) // Foobar 8.3
	{
		GetWindowText(FooHandle, song_title, sizeof(song_title));
		if (stricmp(song_title, "uninteresting")==0) // It seems Columns UI 1.2 is loaded for 8.3
		{
			FooHandle = FindWindow("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}", NULL);
			GetWindowText(FooHandle, song_title, sizeof(song_title));
		}
	}
	else if (FooHandle = FindWindow("{DA7CD0DE-1602-45e6-89A1-C2CA151E008E}/1", NULL)) // Plain foobar 9.1
	{
		foobar_v9 = true;
		GetWindowText(FooHandle, song_title, sizeof(song_title));
		if (char *c = strstr(song_title,"   [foobar2000 v0.9.") ) *c = 0; // Cut off trailing text
	}
	else if (FooHandle = FindWindow("{E7076D1C-A7BF-4f39-B771-BCBE88F2A2A8}", NULL)) // Foobar 9.1 with Coloumns UI
	{
		foobar_v9 = true;
		GetWindowText(FooHandle, song_title, sizeof(song_title));
	}
	else // If there is no handle (foobar is not on), then display the NoInfoText var.
		strcpy(song_title, NoInfoText);
}

//===========================================================================
// Implementation of the FoompButton class
//===========================================================================
void FoompButton::draw(HDC buf)
{
	//Create drawing tools
	HPEN hDefPen = CreatePen(PS_SOLID, 1, OuterStyle.TextColor);
	HPEN hPressedPen = CreatePen(PS_SOLID, 1, ButtonStyle.TextColor);
	HPEN hShadowPen = CreatePen(PS_SOLID, 1, GetShadowColor(OuterStyle));
	HPEN hPressedShadowPen = CreatePen(PS_SOLID, 1, GetShadowColor(ButtonStyle));
	//Save current object
	HGDIOBJ prev = SelectObject(buf,hDefPen);

	if (pressed)
		MakeStyleGradient(buf, &hitrect, &ButtonStyle, false);

	if (FooShadowsEnabled)
	{
		SelectObject(buf,pressed ? hPressedShadowPen : hShadowPen);
		drawShape(buf,x+1,y+1);
	}
	SelectObject(buf,pressed ? hPressedPen : hDefPen);
	drawShape(buf,x,y);


	//Revert old object
	SelectObject(buf, prev);
	//Destroy drawing tools
	DeleteObject(hDefPen);
	DeleteObject(hPressedPen);
	DeleteObject(hShadowPen);
	DeleteObject(hPressedShadowPen);

}
void FoompButton::drawShape(HDC buf, int Penx, int Peny)
{
	switch (type)
	{
		case REWIND_BUTTON:
			//-------		First arrow
			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx, Peny+2);
			
			//-------		2nd arrow
			MoveToEx(buf, Penx+7, Peny+5, NULL);
			LineTo(buf, Penx+7, Peny);
			MoveToEx(buf, Penx+6, Peny+4, NULL);
			LineTo(buf, Penx+6, Peny+1);
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+2);

			break;
		case PLAY_BUTTON:
			//=======	Begin Draw Play
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx+2, Peny+3, NULL);
			LineTo(buf, Penx+2, Peny+2);
			
			//=======	End Draw Play
			break;
		case PAUSE_BUTTON:
			//=======	Begin Draw Pause
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+5, NULL);
			LineTo(buf, Penx+1, Peny);
			//MoveToEx(buf, Penx+2, Peny+5, NULL);
			//LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);
			MoveToEx(buf, Penx+4, Peny+5, NULL);
			LineTo(buf, Penx+4, Peny);

			//=======	End Draw Pause
			break;
		case STOP_BUTTON:
			//=======	Begin Draw Stop
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+5, NULL);
			LineTo(buf, Penx+1, Peny);
			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);
			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);
			MoveToEx(buf, Penx+4, Peny+5, NULL);
			LineTo(buf, Penx+4, Peny);

				// *** ATTN: No need for "rStop" because rPause == "rStop" == "rPls"
			//=======	End Draw Stop
			break;
		case FORWARD_BUTTON:
			
			//=======	Begin Draw Forward
			//-------		First arrow
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx, Peny);
			MoveToEx(buf, Penx+1, Peny+4, NULL);
			LineTo(buf, Penx+1, Peny+1);
			MoveToEx(buf, Penx+2, Peny+3, NULL);
			LineTo(buf, Penx+2, Peny+2);

			//-------		2nd arrow
			MoveToEx(buf, Penx+5, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny);
			MoveToEx(buf, Penx+6, Peny+4, NULL);
			LineTo(buf, Penx+6, Peny+1);
			MoveToEx(buf, Penx+7, Peny+3, NULL);
			LineTo(buf, Penx+7, Peny+2);
			
				// *** ATTN: No need for "rFwd" because rRew == "rFwd"
			//=======	End Draw Forward
			break;
		case PLAYLIST_BUTTON:
			//=======	Begin Draw Playlist
			MoveToEx(buf, Penx, Peny+1, NULL);
			LineTo(buf, Penx+5, Peny+1);
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+3);
			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny+5);

				// *** ATTN: No need for "rStop" because rPause == "rStop" == "rPls"
			//=======	End Draw Playlist
			break;
		case OPEN_BUTTON:
			//=======	Begin Draw Addfiles
			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+5, Peny+3);

			MoveToEx(buf, Penx+1, Peny+2, NULL);
			LineTo(buf, Penx+4, Peny+2);

			MoveToEx(buf, Penx+2, Peny+1, NULL);
			LineTo(buf, Penx+3, Peny+1);

			MoveToEx(buf, Penx, Peny+5, NULL);
			LineTo(buf, Penx+5, Peny+5);

			//=======	End Draw Add files		
			break;
		case UPARROW_BUTTON:
			//=======	Begin Draw Volume Up
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+1, Peny);

			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+3, Peny);

			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny+1);

			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny+1);

			//=======	End Draw Volume Up		
			break;
		case DOWNARROW_BUTTON:
			//=======	Begin Draw Volume Down
			MoveToEx(buf, Penx+5, Peny+3, NULL);
			LineTo(buf, Penx+1, Peny+6);

			MoveToEx(buf, Penx, Peny+3, NULL);
			LineTo(buf, Penx+3, Peny+6);

			MoveToEx(buf, Penx+2, Peny+5, NULL);
			LineTo(buf, Penx+2, Peny);

			MoveToEx(buf, Penx+3, Peny+5, NULL);
			LineTo(buf, Penx+3, Peny);

			//=======	End Draw Volume Down
			break;
	}
}

int FoompButton::width()
{
	switch (type)
	{
		case REWIND_BUTTON:		return 8;
		case PLAY_BUTTON:		return 3;
		case PAUSE_BUTTON:		return 5;
		case STOP_BUTTON:		return 5;
		case FORWARD_BUTTON:	return 8;
		case PLAYLIST_BUTTON:	return 5;
		case OPEN_BUTTON:		return 5;
		case UPARROW_BUTTON:	return 6;
		case DOWNARROW_BUTTON:	return 6;
		default:				return 0;
	}
}
int FoompButton::height()
{
	return 5;
}

bool FoompButton::clicked(int mouseX, int mouseY)
{
	return	(	(mouseY >= hitrect.top)
			&&	(mouseY <= hitrect.bottom)
			&&	(mouseX >= hitrect.left)
			&&	(mouseX <= hitrect.right)
			);
}
// EOF
