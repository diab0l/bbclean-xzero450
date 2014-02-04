/*
 ============================================================================
 Blackbox for Windows: Plugin BBSysMeter 1.0 by Miroslav Petrasko [Theo]
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

#include "bbsysmeter.h"
#include "resource.h"

LPSTR szAppName = "BBSysMeter";		// The name of our window class, etc.
LPSTR szVersion = "BBSysMeter v1.0";	// Used in MessageBox titlebars

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

 
    // Initialize GDI+.
	if(GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL) != 0)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error starting GdiPlus.dll", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	// Get plugin and style settings...
	ReadRCSettings();
	NetTable();


	
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBSysMeter();

	if (dwId == VER_PLATFORM_WIN32_WINDOWS)
    {
      //  m_eOpSys = OPSYS_WIN_9X;
        m_pStatsObj = new CGet9XStats();
    }
    else if (dwId == VER_PLATFORM_WIN32_NT)
    {
        if (dwMajorVer <= 4)
        {
          //  m_eOpSys = OPSYS_WIN_NT4;
            m_pStatsObj = new CGetNTStats();
        }
        else if (dwMajorVer > 4)
        {
          // m_eOpSys = OPSYS_WIN_NT5;
            m_pStatsObj = new CGetNTStats();
        }
    }

	GetStyleSettings();
	
	// Create the window...
	hwndBBSysMeter = CreateWindowEx(
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
	if (!hwndBBSysMeter)
	{
		UnregisterClass(szAppName, hPluginInstance);
//		Gdiplus::GdiplusShutdown(gdiplusToken);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	//Start the plugin timer
	mySetTimer(refresh*1000);
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBSysMeter);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBSysMeter, (LPARAM)msgs);
	const long magicDWord = 0x49474541;
#if !defined _WIN64
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBSysMeter, GWL_USERDATA, magicDWord);
#else
	SetWindowLongPtr(hwndBBSysMeter, GWLP_USERDATA, magicDWord);
#endif

	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBSysMeter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	getStats(monitor);
	//digiFill = true;
	for(int i=0;i<300;i++) mon[i]=0;

	

/*	hfont = CreateFont(fontSizeC,
								fontSizeC, 0, 0, FW_NORMAL,
								false, false, false,
								DEFAULT_CHARSET,
								OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS,
								DEFAULT_QUALITY,
								DEFAULT_PITCH, fontFace);
*/	ShowWindow(hwndBBSysMeter, SW_SHOW);

	InvalidateRect(hwndBBSysMeter, NULL, true);
	
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBSysMeterReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBSysMeter()
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
{
	//shutdown the gdi+ engine
	GdiplusShutdown(g_gdiplusToken);
	
	// Release our timer resources
	KillTimer(hwndBBSysMeter, IDT_TIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...
	if (m_pStatsObj) delete m_pStatsObj;

	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBSysMeter, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBSysMeter);
	// Destroy our window...
	DestroyWindow(hwndBBSysMeter);
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
			
			// the second background is painted

			if ((!(fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4)))||(inSlit)||(!noBitmap))
				{
				
				MakeGradient(hdc, r, myStyleItem2->type,
							backColor2, backColorTo2,
							myStyleItem2->interlaced,
							myStyleItem2->bevelstyle,
							myStyleItem2->bevelposition,
							bevelWidth, borderColor, 0); 
				}
			

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
					
				
					HGDIOBJ otherfont = CreateFont( fontSizeC, 
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

					if ((text_pos == TOP) || (text_pos == BOTTOM)) strcpy(new_line,"");
					else strcpy(new_line,"\r\n");
					
					
					switch (monitor){

					case CPU: 
							sprintf(szTemp,"CPU %s%d %%",new_line,per_stat);
							if (per_stat > 100) per_stat = 0;
							temp = per_stat;
							break;
					case RAM:
							sprintf(szTemp,"RAM %s%d %%",new_line,per_stat);
							if (per_stat > 100) per_stat = 0;
							temp = per_stat;
							break;
					case SWAP:
							sprintf(szTemp,"SWAP %s%d %%",new_line,per_stat);
							if (per_stat > 100) per_stat = 0;
							temp = per_stat;
							break;
					case HDD:
							sprintf(szTemp,"%s: %s%d MB",drive_letter,new_line,per_stat);
							temp = (int)((per_stat * 100.0) / help_stat);
							break;
					case NETT:
							sprintf(szTemp,"NET TOTAL: %s%d",new_line,per_stat);
							temp = 50;
							break;
					case NETIN:
							sprintf(szTemp,"NET IN: %s%d",new_line,per_stat);
							temp = 50;
							break;
					case NETOUT:
							sprintf(szTemp,"NET OUT: %s%d",new_line,per_stat);
							temp = 50;
							break;
							
					}
					 
					theta = (100 - temp) * (PI / 100);
				


				Graphics *graphics;
				Pen *p;
				SolidBrush *br;
				SolidBrush *nr;

				graphics = new Graphics(hdc);
				p = new Pen(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)),2);
				br = new SolidBrush(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));
				nr = new SolidBrush(Color(160,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));
			
				if (anti) graphics->SetSmoothingMode(SmoothingModeAntiAlias);
				rect = r;
				r.bottom = r.bottom - 1;
				r.right = r.right - 1;

			if (draw_type<5)
			{
				switch (text_pos){
					case TOP:	
						DrawText(hdc,szTemp,-1,&rect,DT_TOP | DT_CENTER | DT_SINGLELINE);
						r.top = r.top + fontSizeC + 1;
						break;
					case BOTTOM:
						DrawText(hdc,szTemp,-1,&rect,DT_BOTTOM | DT_CENTER | DT_SINGLELINE);
						r.bottom = r.bottom - fontSizeC;
						break;
					case LEFT:	
						DrawText(hdc,szTemp,-1,&rect,DT_LEFT );
						r.left = r.left + text_width;
						break;
					case RIGHT: 
						DrawText(hdc,szTemp,-1,&rect,DT_RIGHT );
						r.right = r.right - text_width;
						break;
				}
			}		
				
			else {
			    sprintf(szTemp, "CPU %s\n",GetSysNfo(10));
				sprintf(sz, "RAM: %s\n",GetSysNfo(11));
				strcat(szTemp,sz);
				sprintf(sz, "SWAP: %s\n",GetSysNfo(12));
				strcat(szTemp,sz);
				sprintf(sz, "HDD: %s\n",GetSysNfo(13));
				strcat(szTemp,sz);
				sprintf(sz, "PC: %s\n",GetSysNfo(14));
				strcat(szTemp,sz);
 				sprintf(sz, "USER: %s\n",GetSysNfo(15));
				strcat(szTemp,sz);
				sprintf(sz, "WORK AREA: %s\n",GetSysNfo(16));
				strcat(szTemp,sz);
				sprintf(sz, "SCREEN SIZE: %s\n",GetSysNfo(17));
				strcat(szTemp,sz);
				sprintf(sz, "OS VERSION: %s\n",GetSysNfo(18));
				strcat(szTemp,sz);
				sprintf(sz, "NET ADAPTERS: \n%s\n",GetSysNfo(19));
				strcat(szTemp,sz);


				//strcat(szTemp,GetSysNfo(10));
			
				DrawText(hdc,szTemp,-1,&rect,DT_TOP);
			}

				

				
			switch(draw_type)
			{
			case ELLIPSE:
					if (fill)
						graphics->FillPie(nr,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top-2,180,(float)(3.6*(100-temp)));
					else
					    graphics->FillPie(nr,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top-2,180,(float)(3.6*temp));
					
					graphics->DrawEllipse(p,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top-2);
					break;
			
			case HAND:
					graphics->DrawArc(p, r.left+1,r.top+1,r.right-r.left-2,(r.bottom-r.top-2)*2,0.0,-180.0);
					graphics->DrawLine(p, r.left+1,r.bottom-1,r.right-1,r.bottom-1);

					double x,y;
					double a,b;

					a =(r.right-r.left)/2 - 5;
					b =(r.bottom - r.top) - 5;

					x = sqrt((a*a*b*b)/(b*b + tan(theta)*tan(theta)*a*a)) ;
					y = tan(theta)*x ;

				
				if (temp<50)
				graphics->DrawLine(p, r.left + (r.right-r.left)/2,
					    r.bottom - 1,
						(int)(r.left + (r.right-r.left)/2 - 1 - x),
						(int)(r.bottom - 1 + y));

				else if (temp>50)
					graphics->DrawLine(p, r.left + (r.right-r.left)/2,
					    r.bottom - 1,
						(int)(r.left + (r.right-r.left)/2 - 1 + x),
						(int)(r.bottom - 1 - y));
				else if (temp == 50)
					graphics->DrawLine(p, r.left + (r.right-r.left)/2,
					    r.bottom - 1,
						(int)(r.left + (r.right-r.left)/2 - 1),
						(int)(r.bottom - 1 - b));


							
				if (fill) graphics->FillPie(nr, r.left+1,r.top+1,r.right-r.left-2,(r.bottom-r.top-2)*2,180.0,(float)((temp)*1.8));

				graphics->FillPie(br,r.left + (r.right-r.left)/2 - 5,r.bottom-1-5,10,10,0.0,-180.0);
				break;	
			
			case CHART:
				graphics->DrawRectangle(p,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top - 2);
				
				p->SetWidth(1);

				mon[c_cpu] = temp*(r.bottom-r.top - 2)/100; 
				c_cpu++;
				if (c_cpu>=(r.right-r.left-2)) c_cpu=0;
				int ii;
				for(ii=0;ii<(r.right-r.left-2);ii++)
			
        if (fill) graphics->DrawLine(p,r.left+1+ii,r.bottom - 1,r.left+1+ii,r.bottom-1-mon[div(static_cast<LONG>(ii+c_cpu),r.right-r.left-2).rem]);
        else        graphics->DrawLine(p,r.left+1+ii-1,r.bottom-1-mon[div(static_cast<LONG>(ii-1+c_cpu),r.right-r.left-2).rem],r.left+1+ii,r.bottom-1-mon[div(static_cast<LONG>(ii+c_cpu),r.right-r.left-2).rem]);
				break;
			
			case RECTLR:
				p->SetWidth(2);
				graphics->DrawRectangle(p,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top - 2);
				if (fill) graphics->FillRectangle(nr,r.left+1,r.top+1,(int)((r.right-r.left-2)*temp/100),r.bottom-r.top - 2);
				graphics->DrawLine(p,(int)(r.left+1+(r.right-r.left-2)*temp/100),r.top + 1,(int)(r.left+1+(r.right-r.left-2)*temp/100),r.bottom -1);
				break;
			
			case RECTBT:	
			
				p->SetWidth(2);
				graphics->DrawRectangle(p,r.left+1,r.top+1,r.right-r.left-2,r.bottom-r.top - 2);
				if (fill) graphics->FillRectangle(nr,r.left+1,(int)(r.top + (r.bottom-r.top)-(r.bottom-r.top - 2)*temp/100),r.right-r.left-2,(int)((r.bottom-r.top - 2)*temp/100));
				graphics->DrawLine(p,r.left+1,(int)(r.top + (r.bottom-r.top)-(r.bottom-r.top - 2)*temp/100),r.right-1,(int)(r.top + (r.bottom-r.top)-(r.bottom-r.top - 2)*temp/100));
				break;
			
			case 5:
				break;

			}

				delete nr;
				delete br;
				delete p;
				delete graphics;


			
				//Paint to the screen
				DeleteObject(otherfont);
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
			InvalidateRect(hwndBBSysMeter, NULL, true);
		}
		break;

		// ==========

		// Broadcast messages (bro@m -> the bang killah! :D <vbg>)
		case BB_BROADCAST:
		{
			strcpy(szTemp, (LPCSTR)lParam);
			if (saveMessages) writeMessages(szTemp);

			if (!_stricmp(szTemp, "@BBShowPlugins") &&  pluginToggle && !inSlit)
			{
				// Show window and force update...
				ShowWindow( hwndBBSysMeter, SW_SHOW);
				InvalidateRect( hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBSysMeter, SW_HIDE);
			}
			
			
			else if (!_strnicmp(szTemp, command,strlen(command)))
			{ 
				strcpy(szTemp,szTemp + strlen(command));




			if (!_stricmp(szTemp, "About"))
			{
				sprintf(szTemp, "%s\n\n%s ©2004 %s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);

				CMessageBox box(hwndBBSysMeter,				// hWnd
								_T(szTemp),					// Text
								_T(szAppName),				// Caption
								MB_OK | MB_SETFOREGROUND);	// type

				box.SetIcon(IDI_ICON1, hInstance);
				box.DoModal();
				
			}
			else if (!_stricmp(szTemp, "PluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "OnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;

				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBSysMeter, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBSysMeter, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "Slit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBSysMeter);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBSysMeter);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "Transparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "FullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "SnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "LockPosition"))
			{
				lockp = !lockp;
			}
			else if (!_stricmp(szTemp, "DrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "Fill"))
			{
				fill = !fill;

				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "NoBitmap"))
			{
				noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "StyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "EditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "ViewBroams"))
			{
				BBExecute(GetDesktopWindow(), NULL, mpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "SaveBroams"))
			{
			  saveMessages = !saveMessages; 
			}
			else if (!_stricmp(szTemp, "ReloadSettings"))
			{
				
				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBSysMeter);

					//Re-initialize
					ReadRCSettings();
					InitBBSysMeter();
					inSlit = wantInSlit;
					GetStyleSettings();
					
					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBSysMeter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBSysMeter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBSysMeter);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBSysMeter, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "SaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "Mon",2))
			{ 
				if (draw_type == 2) for(int i=0;i<300;i++) mon[i]=0;

				strcpy(szTemp,szTemp + 4);
				
				if (!_stricmp(szTemp, "CPU")) monitor = 0;
				else if (!_stricmp(szTemp, "RAM")) monitor = 1;
				else if (!_stricmp(szTemp, "SWAP")) monitor = 2;
				else if (!_stricmp(szTemp, "HDD")) monitor = 3;
				else if (!_stricmp(szTemp, "NETT")) monitor = 4;
				else if (!_stricmp(szTemp, "NETIN")) monitor = 5;
				else if (!_stricmp(szTemp, "NETOUT")) monitor = 6;
				getStats(monitor);
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
	
			else if (!_strnicmp(szTemp, "Draw", 3))
			{//changing to a hole setting
				draw_type = atoi(szTemp + 4);
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}

			else if (!_strnicmp(szTemp, "DriveLetter", 10))
			{//changing to a hole setting
				strncpy(drive_letter,szTemp + 12,1);
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}

			else if (!_strnicmp(szTemp, "TextPosition", 12))
			{//changing to a hole setting
				text_pos = atoi(szTemp + 13);
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
		
		/*	else if (!_stricmp(szTemp, "@BBSysMeterColor"))
			{
				CHOOSECOLOR cc;
			    COLORREF custCol[16];
				ZeroMemory(&cc, sizeof(CHOOSECOLOR));
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = NULL;
				cc.lpCustColors = (LPDWORD) custCol;
				cc.rgbResult = drawColor;
				cc.Flags = CC_RGBINIT;
				
				if (ChooseColor(&cc)) drawColor = cc.rgbResult;
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}*/
			else if (!_strnicmp(szTemp, "FontSize", 8))
			{
				fontSizeC = atoi(szTemp + 9);
				
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "TextWidth", 9))
			{
				text_width = atoi(szTemp + 10);
				
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "Width", 5))
			{ //changing the clock size
				width = atoi(szTemp + 6);
				
				if ( alwaysOnTop) SetWindowPos( hwndBBSysMeter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBSysMeter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "Height", 6))
			{ //changing the clock size
				height = atoi(szTemp + 7);
									
				if ( alwaysOnTop) SetWindowPos( hwndBBSysMeter, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBSysMeter, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "SetTransparent", 14))
			{
				alpha = atoi(szTemp + 15);
			
				if (transparency) setStatus();					
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "AntiAlias"))
			{
				anti = !anti;
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "HandLength", 10))
			{
				handLength = atoi(szTemp + 11);
			
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "SetBitmap", 9))
			{
				
				strcpy(bitmapFile,szTemp + 11);
				noBitmap = false;
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_strnicmp(szTemp, "Refresh", 7))
			{
				refresh = atoi(szTemp + 8);
			
				KillTimer(hwndBBSysMeter, IDT_TIMER);
				
				mySetTimer(refresh*1000);
				
				InvalidateRect(hwndBBSysMeter, NULL, true);
			}
			else if (!_stricmp(szTemp, "LoadBitmap"))
			{
				
			OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwndBBSysMeter;
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

			InvalidateRect(hwndBBSysMeter, NULL, true);

			}

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
				MoveWindow(hwndBBSysMeter, xpos, ypos, width, height, true);
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
			
			if (!inSlit)
				return DefWindowProc(hwnd,message,wParam,lParam);
		}
		break;
/*
		case WM_NCLBUTTONUP: {} break;
		
		// ==========
		//Show the date when the mouse is clicked and the cntrl button is press
		case WM_LBUTTONUP: 
		{
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
			{
					//set timer to 1 second interval for date
					InvalidateRect(hwndBBSysMeter, NULL, false);
				
			}
		}
		break;*/
		
		// ==========

	/*	case WM_LBUTTONDOWN: {} break;
		
		// ==========

		case WM_NCMBUTTONUP: {} break;
		
		// ==========

		case WM_NCMBUTTONDOWN: {} break;*/
		
		// ==========

		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			// Finally, we show the menu...
			createMenu();
		}
		break;
	
		// ==========

//		case WM_NCRBUTTONDOWN: {} break;

		// ==========
/*
		case WM_NCLBUTTONDBLCLK: 
		{
			//open control panel with:  control timedate.cpl,system,0
			//BBExecute(GetDesktopWindow(), NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
		}
		break;*/

		// ==========

		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_TIMER:
				{
					getStats(monitor);
					//redraw the window
					InvalidateRect(hwndBBSysMeter, NULL, false);
				} break;
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
//			labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.label
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
	 //		labelIsPR = true;
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
		//	labelIsPR = false;
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
		//	labelIsPR = true;
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
		//	labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		//	labelIsPR = true;
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
		//	labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		//	labelIsPR = true;
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
		//	labelIsPR = false;
		}
		else
		{
			if (myStyleItem2) delete myStyleItem2;	//else use the the toolbar: settings
			myStyleItem2 = new StyleItem;
			ParseItem(tempstyle, myStyleItem2);	//use original tempstyle if "parentrelative"
			backColor2 = backColor;			//have to do this if parent relative found, it seems bb4win uses
			backColorTo2 = backColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			fontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		//	labelIsPR = true;
		}
	}
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
	
	strcpy(mpath, temp);
	strcat(mpath,"messages.rc");

	GetModuleFileName(hInstance, szTemp, sizeof(szTemp));
	strcpy(szTemp, szTemp + nLen + 1);
	nLen = strlen(szTemp) - 1;
	while (nLen >0 && szTemp[nLen] != '.') nLen--;
	strncpy(dllname,szTemp,nLen);
	
//	MessageBox(hwndBBSysMeter,dllname,NULL,MB_OK);
	strcpy(command,"@");
	strcat(command,dllname);
	strcat(temp, dllname);
	strcat(path, dllname);
	strcat(temp, ".rc");
	strcat(path, "rc");
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
		strcat(temp, "bbsysmeter.rc");
		strcat(path, "bbsysmeterrc");
		if (FileExists(temp)) strcpy(rcpath, temp);
		else if (FileExists(path)) strcpy(rcpath, path);
		else // If no config file was found, we use the default path and settings, and return
		{
			strcpy(rcpath, defaultpath);
			xpos = ypos = 10;
			width = 100; 
			height = 70;
			monitor = 0;
			alpha = 160;
			wantInSlit = true;
			alwaysOnTop = true;
			snapWindow = true;
			transparency = false;
			fullTrans = false;
			lockp = false;
			drawBorder = false;
			pluginToggle = false;
			fontSizeC = 6;
			refresh = 1;
			strcpy(windowStyle, "windowlabel");
			strcpy(bitmapFile, ".none");
			noBitmap = true;
			//dotRadius = 40; 
			//handLength = 40;
			fill = true;
			strcpy(drive_letter,"c");
			draw_type = 0;
		//	drawColor = fontColor;
			saveMessages = false;
			text_pos = 1;
			text_width = 20;
			anti = true;
			WriteRCSettings();
			
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbsysmeter.x:", 10);
	ypos = ReadInt(rcpath, "bbsysmeter.y:", 10);

	width = ReadInt(rcpath, "bbsysmeter.width:", 100);
	height = ReadInt(rcpath, "bbsysmeter.height:", 50);
	monitor = ReadInt(rcpath, "bbsysmeter.monitor:", 0);
	if ((monitor<0) || (monitor>6)) monitor = 0; 
	draw_type = ReadInt(rcpath, "bbsysmeter.draw.type:", 0);
	if ((draw_type <0) || (draw_type>5)) draw_type = 0;
	refresh = ReadInt(rcpath, "bbsysmeter.refresh.time:", 1);
	if (refresh<=0) refresh = 1; 
	alpha = ReadInt(rcpath, "bbsysmeter.alpha:", 160);
	if(alpha > 255) alpha = 255;
	if(ReadString(rcpath, "bbsysmeter.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbsysmeter.inSlit:", true);
//	drawColor = ReadColor(rcpath, "bbsysmeter.draw.color:", "0xffffff");
	alwaysOnTop = ReadBool(rcpath, "bbsysmeter.alwaysOnTop:", true);
	drawBorder = ReadBool(rcpath, "bbsysmeter.drawBorder:", true);
	snapWindow = ReadBool(rcpath, "bbsysmeter.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbsysmeter.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbsysmeter.fullTrans:", false);
	lockp = ReadBool(rcpath, "bbsysmeter.lockPosition:", false);
	fontSizeC = ReadInt(rcpath, "bbsysmeter.fontSizeC:", 6);
	alwaysOnTop = ReadBool(rcpath, "bbsysmeter.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbsysmeter.pluginToggle:", false);
	fill = ReadBool(rcpath, "bbsysmeter.fill:", true);
	anti = ReadBool(rcpath, "bbsysmeter.anti.alias:", true);
	//dotRadius = ReadInt(rcpath, "bbsysmeter.dotRadius:", 40);
	//if (dotRadius > width/2) dotRadius = width/2;
	//handLength = ReadInt(rcpath, "bbsysmeter.handLength:", 40);
	//if (handLength > width/2) handLength = width/2;
	strcpy(drive_letter, ReadString(rcpath, "bbsysmeter.drive.letter:", "c"));
	strcpy(windowStyle, ReadString(rcpath, "bbsysmeter.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");
	strcpy(bitmapFile, ReadString(rcpath, "bbsysmeter.bitmapFile:", ".none"));
	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 
	saveMessages = ReadBool(rcpath, "bbsysmeter.save.broams:", false);
	text_pos = ReadInt(rcpath, "bbsysmeter.text.position:", 0);
	if ((text_pos<0) || (text_pos>3)) text_pos = 0;
	text_width = ReadInt(rcpath, "bbsysmeter.text.width:", 20);
	
}

//===========================================================================


void writeMessages(char message[MAX_LINE_LENGTH])
{
	static char szTemp[MAX_LINE_LENGTH];
	static char temp[8];
	DWORD retLength = 0;

	HANDLE file = CreateFile(mpath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file)
	{
		SetFilePointer(file,0,NULL,FILE_END);
		sprintf(szTemp, "%s\r\n", message);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	}
	CloseHandle(file);
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

		sprintf(szTemp, "! BBSysMeter %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbsysmeter.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbsysmeter.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbsysmeter.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(lockp) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.lockPosition: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.drawBorder: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.monitor: %d\r\n", monitor, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.draw.type: %d\r\n", draw_type, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	/*	sprintf(szTemp, "bbsysmeter.dotRadius: %d\r\n", dotRadius, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.handLength: %d\r\n", handLength, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
		(fill) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.fill: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(anti) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.anti.alias: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.fontSizeC: %d\r\n", fontSizeC, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.text.width: %d\r\n", text_width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.text.position: %d\r\n", text_pos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbsysmeter.drive.letter: %s\r\n", drive_letter);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbsysmeter.refresh.time: %d\r\n", refresh, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	/*	sprintf(szTemp, "bbsysmeter.draw.color: #%.2x%.2x%.2x\r\n", GetRValue(drawColor),GetGValue(drawColor),GetBValue(drawColor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
		sprintf(szTemp, "bbsysmeter.bitmapFile: %s\r\n", bitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(saveMessages) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbsysmeter.save.broams: %s\r\n", temp);
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


void mySetTimer(int time)
{
	
		//Start the 10 second plugin timer
		SetTimer(hwndBBSysMeter,		// handle to main window 
				IDT_TIMER,			// timer identifier 
				time,				// time/1000 - second interval 
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


//===========================================================================
void getStats(int stat)
{

		per_stat = 0;			
		if (stat == CPU) 
		{
			m_pStatsObj->GetData(per_stat);
			//help_stat = 100;
		}
		else if (stat == RAM)
		{
			GlobalMemoryStatus(&memstat);
			per_stat = (DWORD)(100.0 - ((memstat.dwAvailPhys * 100.0) / memstat.dwTotalPhys));
			//help_stat = 100;
		}
		else if (stat == SWAP)
		{
			GlobalMemoryStatus(&memstat);
			per_stat = (DWORD)(100.0 - ((memstat.dwAvailPageFile * 100.0) / memstat.dwTotalPageFile));
			//help_stat = 100;
		}
		else if (stat == HDD)
		{
			sprintf(szTemp, "%s:\\",drive_letter);
			GetDiskFreeSpaceEx((LPCTSTR)szTemp, &fba, &tnob, &tnofb);
			per_stat = (int)(fba.QuadPart / (DWORD)1024 / (DWORD)1024);
			help_stat = (int)(tnob.QuadPart / (DWORD)1024 / (DWORD)1024);
		}
		else if (stat == NETT)
		{
			per_stat = GetNetOctets(2)/1000;
		}
		else if (stat == NETIN)
		{
			per_stat = GetNetOctets(0)/1000;
		}
		else if (stat == NETOUT)
		{
			per_stat = GetNetOctets(1)/1000;
		}
/*		else if (stat == GDI)
		{
			per_stat = GetGuiResources(hwndBlackbox,1);
		}
*/
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
								SetWindowLong(hwndBBSysMeter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBSysMeter, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBSysMeter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBSysMeter, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBSysMeter, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBSysMeter, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBSysMeter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBSysMeter, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBSysMeter, NULL, false);		
}


//---------------------------------------------------------------------------

LPCTSTR GetSysNfo(UINT id)
{

	static char stemp[4096];
			static char s[4096];
	DWORD len = 4095;
	UINT stat;
switch(id)
	{
	case CPU:
	    m_pStatsObj->GetData(stat);
		_itoa(stat,stemp,10);
		return stemp;
	case RAM:
		GlobalMemoryStatus(&memstat);
		stat = (DWORD)(100.0 - ((memstat.dwAvailPhys * 100.0) / memstat.dwTotalPhys));
		_itoa(stat,stemp,10);
		return stemp;
	case SWAP:
		GlobalMemoryStatus(&memstat);
		stat = (DWORD)(100.0 - ((memstat.dwAvailPageFile * 100.0) / memstat.dwTotalPageFile));
		_itoa(stat,stemp,10);
		return stemp;
	case HDD:
		sprintf(s, "%s:\\",drive_letter);
		GetDiskFreeSpaceEx((LPCTSTR)s, &fba, &tnob, &tnofb);
		stat = (int)(fba.QuadPart / (DWORD)1024 / (DWORD)1024);
		//help_stat = (int)(tnob.QuadPart / (DWORD)1024 / (DWORD)1024);
		_itoa(stat,stemp,10);
		return stemp;
	case COMPUTER_NAME:
		GetComputerName(stemp, &len);
		return stemp;

	case USER_NAME:
		GetUserName(stemp, &len);
		return stemp;

	case WORK_AREA:
		sprintf(stemp, "%i x %i", GetSystemMetrics(SM_CXFULLSCREEN), GetSystemMetrics(SM_CYFULLSCREEN));
		return stemp;

	case SCREEN_SIZE:
		sprintf(stemp, "%i x %i", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
		return stemp;

	case OS_VERSION:

		OSVERSIONINFO version;
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&version);

		if (version.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if (version.dwMajorVersion <= 4)
			{
				strcpy(stemp, "Windows NT");
			}
		else
		{
			if (version.dwMinorVersion == 2)
			{
				strcpy(stemp, "Windows 2003");
			}
			else if (version.dwMinorVersion == 1)
			{
				strcpy(stemp, "Windows XP");
			}
			else if (version.dwMinorVersion == 0)
			{
				strcpy(stemp, "Windows 2000");
			}
			else
			{
				strcpy(stemp, "Unknown");
			}
			}
		}
		else
		{
		if (version.dwMinorVersion < 10)
		{
			strcpy(stemp, "Windows 95");
		}
		else if (version.dwMinorVersion < 90)
		{
			strcpy(stemp, "Windows 98");
		}
		else
		{
			strcpy(stemp, "Windows ME");
		}
		}
		return stemp;

	case ADAPTER_DESCRIPTION:
		DWORD value;

		strcpy(s,"");
		if(GetNumberOfInterfaces(&value) == NO_ERROR)
		{

		if (ERROR_SUCCESS == GetAdaptersInfo((IP_ADAPTER_INFO*)stemp, &len))
		{
			PIP_ADAPTER_INFO info = (IP_ADAPTER_INFO*)stemp;
			{
			UINT i = 0;
			while (info)
				{
				if (i < value)
					{
					strcat(s,info->Description);
					strcat(s,"\n");
					}	
				info = info->Next;
				i++;
				}
			return s;
			}
		}
		}
		break;

/*	case IP_ADDRESS:
		if (NO_ERROR == GetIpAddrTable((PMIB_IPADDRTABLE)stemp, &len, FALSE))
		{
			PMIB_IPADDRTABLE ipTable = (PMIB_IPADDRTABLE)stemp;
		//	if (data < ipTable->dwNumEntries)
		//	{
				DWORD ip = ipTable->table[0].dwAddr;
				sprintf(stemp, "%i.%i.%i.%i", ip%256, (ip>>8)%256, (ip>>16)%256, (ip>>24)%256);
				return stemp;
		//	}
		}
		break;

	case NET_MASK:
		if (NO_ERROR == GetIpAddrTable((PMIB_IPADDRTABLE)stemp, &len, FALSE))
		{
			PMIB_IPADDRTABLE ipTable = (PMIB_IPADDRTABLE)stemp;
		//	if (data < ipTable->dwNumEntries)
		//	{
				DWORD ip = ipTable->table[0].dwMask;
				sprintf(stemp, "%i.%i.%i.%i", ip%256, (ip>>8)%256, (ip>>16)%256, (ip>>24)%256);
				return stemp;
		//	}
		}
		break;

	case GATEWAY_ADDRESS:
		if (ERROR_SUCCESS == GetAdaptersInfo((IP_ADAPTER_INFO*)stemp, &len))
		{
			PIP_ADAPTER_INFO info = (IP_ADAPTER_INFO*)stemp;
//			int i = 0;
		//	while (info)
		//	{
		//		if (i == data)
		//		{
					return info->GatewayList.IpAddress.String;
		//		}
		//		info = info->Next;
		//		i++;
		//	}
		}
		break;

	case HOST_NAME:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)stemp, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)stemp;
			return info->HostName;
		}
		break;

	case DOMAIN_NAME:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)stemp, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)stemp;
			return info->DomainName;
		}
		break;

	case DNS_SERVER:
		if (ERROR_SUCCESS == GetNetworkParams((PFIXED_INFO)stemp, &len))
		{
			PFIXED_INFO info = (PFIXED_INFO)stemp;
			if (info->CurrentDnsServer)
			{
				return info->CurrentDnsServer->IpAddress.String;
			}
			else
			{
				return info->DnsServerList.IpAddress.String;
			}
		}
		break;*/
	}

	return NULL;

} 

//---------------------------------------------------------------------
//net things

void NetTable()
{
 if(c_Table == NULL)
	{
		// Gotta reserve few bytes for the tables
		DWORD value;
		if(GetNumberOfInterfaces(&value) == NO_ERROR)
		{
			c_NumOfTables = value;

			if(c_NumOfTables > 0)
			{
				DWORD size = sizeof(MIB_IFTABLE) + sizeof(MIB_IFROW) * c_NumOfTables;
				c_Table = (MIB_IFTABLE*)new char[size];
			}
		}
	}

	if(c_Table)
	{
		DWORD size = sizeof(MIB_IFTABLE) + sizeof(MIB_IFROW) * c_NumOfTables;
		if(GetIfTable(c_Table, &size, false) != NO_ERROR)
		{
			delete [] c_Table;
			c_Table = (MIB_IFTABLE*)new char[size];
			if(GetIfTable(c_Table, &size, false) != NO_ERROR)
			{
				// Something's wrong. Unable to get the table.
				delete [] c_Table;
				c_Table = NULL;
			}
		}
	}
}

DWORD GetNetOctets(UINT net){
DWORD value = 0;
for(int i = 0; i < c_NumOfTables; i++)
		{
			// Ignore the loopback
			if(strcmp((char*)c_Table->table[i].bDescr, "MS TCP Loopback interface") != 0)
			{
				switch (net)
				{
				case 0:
					value += c_Table->table[i].dwInOctets;
					break;

				case 1:
					value += c_Table->table[i].dwOutOctets;
					break;

				case 2:
					value += c_Table->table[i].dwInOctets;
					value += c_Table->table[i].dwOutOctets;
					break;
				}
			}
		}
return value;
}
//---------------------------------------------------------------------------

void createMenu()
{
	bool tempBool = false;

	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
		
			//Now we define all menus and submenus
			
			
			otherSubmenu = MakeMenu("Other");
			strcpy(szTemp,command);
			MakeMenuItem(otherSubmenu, "Draw Border", strcat(szTemp,"DrawBorder"), drawBorder);
			strcpy(szTemp,command);
			MakeMenuItem(otherSubmenu, "Fill", strcat(szTemp,"Fill"), fill);
			strcpy(szTemp,command);
			MakeMenuItemInt(otherSubmenu, "Width", strcat(szTemp,"Width"), width, 20, 400);
			strcpy(szTemp,command);
			MakeMenuItemInt(otherSubmenu, "Height", strcat(szTemp,"Height"), height, 20, 400);
			strcpy(szTemp,command);
			MakeMenuItem(otherSubmenu, "Use Anti-Alias", strcat(szTemp,"AntiAlias"), anti);
		//	strcpy(szTemp,command);
		//	MakeMenuItemInt(otherSubmenu, "Hand Length", strcat(szTemp,"HandLength"), handLength, 0, (int)width/2);
			strcpy(szTemp,command);
			MakeMenuItemString(otherSubmenu, "Drive Letter", strcat(szTemp,"DriveLetter"), drive_letter);
		 // MakeMenuItem(windowStyleSubmenu, "Draw Color", "@BBSysMeterColor", false);
			
			//MakeMenuNOP(windowStyleSubmenu, "___________________");
			windowStyleSubmenu = MakeMenu("Style");
			if(StrStrI(windowStyle, "toolbar") != NULL) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar:", strcat(szTemp,"StyleToolbar"), tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonnp") != NULL) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar.button:", strcat(szTemp,"StyleButton"), tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "buttonpr") != NULL) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", strcat(szTemp,"StyleButtonPr"), tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar.label:", strcat(szTemp,"StyleLabel"), tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "windowlabel") != NULL) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", strcat(szTemp,"StyleWindowLabel"), tempBool);
			tempBool = false;
			if(StrStrI(windowStyle, "clock") != NULL) tempBool = true;
			strcpy(szTemp,command);
			MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", strcat(szTemp,"StyleClock"), tempBool);
					
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			strcpy(szTemp,command);
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", strcat(szTemp,"Slit"), wantInSlit);
			strcpy(szTemp,command);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", strcat(szTemp,"PluginToggle"), pluginToggle);
			strcpy(szTemp,command);
			MakeMenuItem(generalConfigSubmenu, "Always on top", strcat(szTemp,"OnTop"), alwaysOnTop);
			strcpy(szTemp,command);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", strcat(szTemp,"Transparent"), transparency);
			strcpy(szTemp,command);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", strcat(szTemp,"SetTransparent"),alpha,0,255);
			strcpy(szTemp,command);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", strcat(szTemp,"FullTrans"), fullTrans);
			strcpy(szTemp,command);
			MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", strcat(szTemp,"SnapToEdge"), snapWindow);
			strcpy(szTemp,command);
			MakeMenuItem(generalConfigSubmenu, "Lock Position", strcat(szTemp,"LockPosition"), lockp);
			

			monitorSubmenu = MakeMenu("Monitor");
			tempBool = false;strcpy(szTemp,command);
			if (monitor == CPU) tempBool = true;
			MakeMenuItem(monitorSubmenu, "CPU", strcat(szTemp,"Mon CPU"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == RAM) tempBool = true;
			MakeMenuItem(monitorSubmenu, "RAM", strcat(szTemp,"Mon RAM"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == SWAP) tempBool = true;
			MakeMenuItem(monitorSubmenu, "SWAP", strcat(szTemp,"Mon SWAP"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == HDD) tempBool = true;
			MakeMenuItem(monitorSubmenu, "HDD", strcat(szTemp,"Mon HDD"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == NETT) tempBool = true;
			MakeMenuItem(monitorSubmenu, "NET TOTAL", strcat(szTemp,"Mon NETT"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == NETIN) tempBool = true;
			MakeMenuItem(monitorSubmenu, "NET IN", strcat(szTemp,"Mon NETIN"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (monitor == NETOUT) tempBool = true;
			MakeMenuItem(monitorSubmenu, "NET OUT", strcat(szTemp,"Mon NETOUT"), tempBool);
			

			drawSubmenu = MakeMenu("Draw Type");
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 0) tempBool = true;
			MakeMenuItem(drawSubmenu, "Ellipse", strcat(szTemp,"Draw 0"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 1) tempBool = true;
			MakeMenuItem(drawSubmenu, "Hand", strcat(szTemp,"Draw 1"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 2) tempBool = true;
			MakeMenuItem(drawSubmenu, "Chart", strcat(szTemp,"Draw 2"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 3) tempBool = true;
			MakeMenuItem(drawSubmenu, "Rect L->R", strcat(szTemp,"Draw 3"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 4) tempBool = true;
			MakeMenuItem(drawSubmenu, "Rect B->T", strcat(szTemp,"Draw 4"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (draw_type == 5) tempBool = true;
		//	MakeMenuItem(drawSubmenu, "Text", strcat(szTemp,"Draw 5"), tempBool);
		
			browseSubmenu = MakeMenu("Browse");
			strcpy(szTemp,command);
			MakeMenuItem(browseSubmenu, "Browse...", strcat(szTemp,"LoadBitmap"), false);

			bitmapSubmenu = MakeMenu("Bitmap");
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
			strcpy(szTemp,command);
			MakeMenuItem(bitmapSubmenu, "Nothing", strcat(szTemp,"NoBitmap"), noBitmap);

			settingsSubmenu = MakeMenu("Settings");
			strcpy(szTemp,command);
			MakeMenuItem(settingsSubmenu, "Edit Settings", strcat(szTemp,"EditRC"), false);
			strcpy(szTemp,command);
			MakeMenuItem(settingsSubmenu, "Reload Settings", strcat(szTemp,"ReloadSettings"), false);
			strcpy(szTemp,command);
			MakeMenuItem(settingsSubmenu, "Save Settings", strcat(szTemp,"SaveSettings"), false);
			
			messagesSubmenu = MakeMenu("Messages");
			strcpy(szTemp,command);
			MakeMenuItem(messagesSubmenu, "Save Bro@ms", strcat(szTemp,"SaveBroams"), saveMessages);
			strcpy(szTemp,command);
			MakeMenuItem(messagesSubmenu, "View Bro@ms", strcat(szTemp,"ViewBroams"), false);
			
			textSubmenu = MakeMenu("Text");
			strcpy(szTemp,command);
			MakeMenuItemInt(textSubmenu, "Font Size", strcat(szTemp,"FontSize"), fontSizeC, 6, width/3);
			strcpy(szTemp,command);
			MakeMenuItemInt(textSubmenu, "Text Width", strcat(szTemp,"TextWidth"), text_width, 3, width);
	
			tempBool = false;strcpy(szTemp,command);
			if (text_pos == 0) tempBool = true;
			MakeMenuItem(textSubmenu, "Top", strcat(szTemp,"TextPosition 0"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (text_pos == 1) tempBool = true;
			MakeMenuItem(textSubmenu, "Bottom", strcat(szTemp,"TextPosition 1"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (text_pos == 2) tempBool = true;
			MakeMenuItem(textSubmenu, "Left", strcat(szTemp,"TextPosition 2"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (text_pos == 3) tempBool = true;
			MakeMenuItem(textSubmenu, "Right", strcat(szTemp,"TextPosition 3"), tempBool);


			refreshSubmenu = MakeMenu("Refresh");
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 1) tempBool = true;
			MakeMenuItem(refreshSubmenu, " 1 s", strcat(szTemp,"Refresh 1"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 2) tempBool = true;
			MakeMenuItem(refreshSubmenu, " 2 s", strcat(szTemp,"Refresh 2"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 5) tempBool = true;
			MakeMenuItem(refreshSubmenu, " 5 s", strcat(szTemp,"Refresh 5"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 10) tempBool = true;
			MakeMenuItem(refreshSubmenu, "10 s", strcat(szTemp,"Refresh 10"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 30) tempBool = true;
			MakeMenuItem(refreshSubmenu, "30 s", strcat(szTemp,"Refresh 30"), tempBool);
			tempBool = false;strcpy(szTemp,command);
			if (refresh == 60) tempBool = true;
			MakeMenuItem(refreshSubmenu, "60 s", strcat(szTemp,"Refresh 60"), tempBool);
			//attach defined menus together
			strcpy(szTemp,command);
			strcpy(szTemp,szTemp + 1);
			strcat(szTemp," ");
			strcat(szTemp,szInfoVersion);
			myMenu = MakeMenu(szTemp);
			
			MakeSubmenu(configSubmenu, monitorSubmenu, "Monitor");
			MakeSubmenu(configSubmenu, drawSubmenu, "Draw Type");
			MakeSubmenu(configSubmenu, refreshSubmenu, "Refresh Time");
			MakeSubmenu(configSubmenu, textSubmenu, "Text");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, otherSubmenu, "Other");
			MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
			MakeSubmenu(configSubmenu, messagesSubmenu, "Bro@ms");
			
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			strcpy(szTemp,command);
			MakeMenuItem(myMenu, "About", strcat(szTemp,"About"), false);
			ShowMenu(myMenu);
}

// the end ....
