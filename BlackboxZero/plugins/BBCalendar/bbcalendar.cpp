/*
 ============================================================================
 Blackbox for Windows: Plugin bbcalendar 1.1 by Miroslav Petrasko [Theo] made
 from BBAnalogEx and Sdk Example
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

 ============================================================================
*/

#include "bbcalendar.h"
//#include "resource.h"

LPSTR szAppName = "BBCalendar";		// The name of our window class, etc.
LPSTR szVersion = "BBCalendar v1.1.1";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.1.1";
LPSTR szInfoAuthor = "Theo";
LPSTR szInfoRelDate = "2004-09-11";
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
	//initialize the plugin before getting style settings
	InitBBCalendar();

	// Get plugin and style settings...
	
	
	GetStyleSettings();
	ReadRCSettings();
	setFontColor();
	
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	place(placement);
	getCurrentDate();
	getAlarms();
	strcpy(name,getName());
	
	
	

	
	// Create the window...
	hwndBBCalendar = CreateWindowEx(
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
	if (!hwndBBCalendar)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	mySetTimer();

	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBCalendar);
	else inSlit = false;
	
	setStatus();
	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBCalendar, (LPARAM)msgs);
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBCalendar, GWL_USERDATA, magicDWord);
	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBCalendar, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBCalendar, SW_SHOW);
	
	InvalidateRect(hwndBBCalendar, NULL, false);
	executeAlarm(day);
	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBCalendarReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBCalendar()
{
	
    dwId = 0;
    dwMajorVer = 0;
    dwMinorVer = 0;

	_GetPlatformId (&dwId, &dwMajorVer, &dwMinorVer);
    
	//get screen dimensions
	ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
}

//===========================================================================

void endPlugin(HINSTANCE hPluginInstance)
{
    DestroyMessageBox();
	// Release our timer resources
	KillTimer(hwndBBCalendar, IDT_TIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (toolbar) delete toolbar;
	if (button) delete button;
	if (buttonpr) delete buttonpr;
	if (label) delete label;
	if (winlabel) delete winlabel;
	if (cclock) delete cclock;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...
	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBCalendar, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBCalendar);
	// Destroy our window...
	DestroyWindow(hwndBBCalendar);
	// Unregister window class...
	UnregisterClass(szAppName, hPluginInstance);
}

//===========================================================================

void drawRect(HDC &hdc, int type, RECT &re, LPCSTR path, LPCSTR text)
{

	switch (type){
		case NONE:
			break;
		case BMP:
			if ((!path == NULL)&&(!strcmp(path,".none")==0)&&(FileExists(path)))
						{
					
					HANDLE image;
					image = LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					
					HDC hdcMem = CreateCompatibleDC(hdc);
					
					//HBITMAP old = (HBITMAP) 
					SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);
					
					if (bopt == CENTER) 
					TransparentBlt(hdc, re.left + (re.right - re.left)/2 - bitmap.bmWidth/2, re.top + (re.bottom - re.top)/2 - bitmap.bmHeight/2, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
						
					else if (bopt == STRETCH)
					TransparentBlt(hdc, re.left, re.top, re.right - re.left, re.bottom - re.top, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
				
				//	SelectObject(hdcMem, old);
				//	DeleteObject(old);
					DeleteObject(image);
					DeleteDC(hdcMem);
				}
				DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case TOOLBAR:
			MakeGradient(hdc, re, toolbar->type,
							toolbarColor, toolbarColorTo,
							toolbar->interlaced,
							toolbar->bevelstyle,
							toolbar->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,tfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case BUTTON:
			MakeGradient(hdc, re, button->type,
							buttonColor, buttonColorTo,
							button->interlaced,
							button->bevelstyle,
							button->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,buttonfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case BUTTONPR:
			MakeGradient(hdc, re, buttonpr->type,
							buttonprColor, buttonprColorTo,
							buttonpr->interlaced,
							buttonpr->bevelstyle,
							buttonpr->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,buttonprfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case LABEL:
			MakeGradient(hdc, re, label->type,
							labelColor, labelColorTo,
							label->interlaced,
							label->bevelstyle,
							label->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,labelfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case WINLABEL:
			MakeGradient(hdc, re, winlabel->type,
							winlabelColor, winlabelColorTo,
							winlabel->interlaced,
							winlabel->bevelstyle,
							winlabel->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,winlabelfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case CLOCK:
			MakeGradient(hdc, re, cclock->type,
							clockColor, clockColorTo,
							cclock->interlaced,
							cclock->bevelstyle,
							cclock->bevelposition,
							bevelWidth, borderColor, 0); 
			SetTextColor(hdc,clockfontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case RECTT:
			{
			HPEN pPen,pPenOld;
			pPen = CreatePen(PS_SOLID, 0, fontColor);
			pPenOld = (HPEN)SelectObject(hdc, pPen);
			MoveToEx(hdc,re.left,re.top,NULL);
			LineTo(hdc,re.left,re.bottom-1);
			LineTo(hdc,re.right-1,re.bottom-1);
			LineTo(hdc,re.right-1,re.top);
			LineTo(hdc,re.left,re.top);
			SelectObject(hdc, pPenOld);
			DeleteObject(pPen);
			SetTextColor(hdc,fontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			}
			break;
		case TRANS:

			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
			
			{		HPEN pPen, pPenOld;
					HBRUSH pBrush, pBrushOld;
					pPen = CreatePen(PS_SOLID, 0, 0xff00ff);
					pBrush = CreateSolidBrush(0xff00ff);
					pPenOld = (HPEN)SelectObject(hdc, pPen);
					pBrushOld = (HBRUSH)SelectObject(hdc, pBrush);
					Rectangle(hdc, re.left, re.top, re.right, re.bottom);
					SelectObject(hdc, pPenOld);
					SelectObject(hdc, pBrushOld);
					DeleteObject(pBrush);
					DeleteObject(pPen);
					
			}
			SetTextColor(hdc,fontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
		case BORDER:

				//HPEN pPen, pPenOld;
					HBRUSH pBrush, pBrushOld;
					//pPen = CreatePen(PS_SOLID, 0, borderColor);
					pBrush = CreateSolidBrush(borderColor);
					//pPenOld = (HPEN)SelectObject(hdc, pPen);
					pBrushOld = (HBRUSH)SelectObject(hdc, pBrush);
					
					FillRect(hdc, &re, pBrush);
					//Rectangle(hdc, re.left, re.top, re.right, re.bottom);
					//SelectObject(hdc, pPenOld);
					SelectObject(hdc, pBrushOld);
					DeleteObject(pBrush);
					//DeleteObject(pPen);
					
			SetTextColor(hdc,fontColor);
			DrawText(hdc,text,-1,&re,DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			break;
	}
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
			
			SetBkMode(hdc, TRANSPARENT);
          //  

		   //Paint second background according to toolbar.label.color: and toolbar.label.colorto:
			if(drawBorder)
			{
					drawRect(hdc,BORDER,r,NULL,NULL);
					conRect(r,borderWidth);
					/*r.left += borderWidth;
					r.top += borderWidth;
					r.bottom -= borderWidth;
					r.right = r.right - borderWidth;
					*/	
			}
			rect = r; 
			if (!(cstyle == BMP))
			drawRect(hdc,cstyle,r,bitmapfile[CALENDAR],NULL);
			drawRect(hdc,BMP,r,bitmapfile[CALENDAR],NULL);
			
			HFONT oldfont;
			HFONT otherfont = CreateFont( dateFontSize,  
						0, ((text_pos<3)||(drawMode==LINE))?0:900, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						deffont?fontFace[DEFAULT]:fontFace[DATEFONT]);
		    oldfont = (HFONT)SelectObject(hdc, otherfont);
			SetTextColor(hdc, fontColor);
	
			///namesday	
			if (shownames)
			{
			rect.top = rect.bottom - dateFontSize - 3;
			drawName(hdc,rect);
			rect = r;
			rect.bottom = rect.bottom - dateFontSize - 3;
			}
//namesday end
			
			RECT hRect = rect;
			


			switch (text_pos)
			{
				case TOP:
						
						rect.bottom = rect.top + dateFontSize + 4;
						drawRect(hdc,dstyle,rect,bitmapfile[DATEC],NULL);
						switch (texta)
						{
						case 0: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);break;
						case 1: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);break;
						case 2: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);break;
						}
						rect = hRect;
						rect.top = rect.top + dateFontSize + 4; 
						break;
				case BOTTOM: 
						rect.top = rect.bottom - dateFontSize - 4;
						drawRect(hdc,dstyle,rect,bitmapfile[DATEC],NULL);	
						switch (texta)
						{
						case 0: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_LEFT | DT_SINGLELINE);break;
						case 1: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);break;
						case 2: DrawText(hdc, drawclock, -1, &rect, DT_VCENTER | DT_RIGHT | DT_SINGLELINE);break;
						}
						rect = hRect;
						rect.bottom = rect.bottom - dateFontSize - 4;
						break;
				case LEFT:
					{
						RECT helpr = rect;
						DrawText(hdc, drawclock, -1, &helpr, DT_LEFT | DT_VCENTER | DT_CALCRECT);
					
						if (drawMode == LINE){
						rect.right = rect.left + helpr.right - helpr.left;
						drawRect(hdc,dstyle,rect,bitmapfile[DATEC],drawclock);
						rect = hRect;
						rect.left += helpr.right - helpr.left;
					
						}else {
						rect.right = rect.left + dateFontSize + 4;
						drawRect(hdc,dstyle,rect,NULL,NULL);
						
						switch (texta)
						{
						case 0: rect.bottom += (helpr.bottom - helpr.top) - 1; break;
						case 1: rect.bottom = (rect.bottom - rect.top)/2 + (helpr.right - helpr.left)/2 + (helpr.bottom - helpr.top)  + 1;break;
						case 2: rect.bottom = rect.top + (helpr.right - helpr.left) + (helpr.bottom - helpr.top)  + 1;break;
						}

						DrawText(hdc, drawclock, -1, &rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
						rect = hRect;
						rect.left = r.left + dateFontSize + 4;
						}
						break;
					}
				case RIGHT:
					{
						RECT helpr = rect;
						DrawText(hdc, drawclock, -1, &helpr, DT_RIGHT | DT_VCENTER | DT_CALCRECT);
												
						if (drawMode == LINE){
						rect.left = rect.right - (helpr.right - helpr.left);
						drawRect(hdc,dstyle,rect,bitmapfile[DATEC],drawclock);
						rect = hRect;
						rect.right -= helpr.right - helpr.left;
					
						}else {
						rect.left = r.right - dateFontSize - 4;
						drawRect(hdc,dstyle,rect,NULL,NULL);

						switch (texta)
						{
						case 0: rect.bottom += (helpr.bottom - helpr.top) - 1; break;
						case 1: rect.bottom = (rect.bottom - rect.top)/2 + (helpr.right - helpr.left)/2 + (helpr.bottom - helpr.top)  + 1;break;
						case 2: rect.bottom = rect.top + (helpr.right - helpr.left) + (helpr.bottom - helpr.top)  + 1;break;
						}

						DrawText(hdc, drawclock, -1, &rect, DT_LEFT | DT_BOTTOM | DT_SINGLELINE);
						
						rect = hRect;
						rect.right = r.right - dateFontSize - 4;
				
						}
						break;
					}
			}
				
		
			
			
	
				SelectObject(hdc, oldfont);
				DeleteObject(otherfont);
				otherfont = CreateFont( fontSize,   //pBBCalendar->text.fontSize,
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						deffont?fontFace[DEFAULT]:fontFace[WEEKFONT]);
				SelectObject(hdc, otherfont);

				HFONT daysfont = CreateFont( fontSize,  
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						deffont?fontFace[DEFAULT]:fontFace[DAYSFONT]);
		    

				hRect = rect;
				int help;
		SetTextColor(hdc, fontColor);
		switch (drawMode){
			case HORIZONTAL:{
					
				help = ((rect.right - rect.left) - ((int)((rect.right - rect.left)/osy))*osy)/2;
					
					if ((wpos == POSS1)||(wpos == BOTH)){
					rect.bottom = rect.top + fontSize + 2;
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);

					rect.left +=help;
					rect.right -= help;
					drawWeek(hdc, rect);
					
					rect.bottom = hRect.bottom;
					rect.top += fontSize + 2;
					}
					if ((wpos == POSS2)||(wpos == BOTH)){
					rect.top = rect.bottom - (fontSize + 2);
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);

					rect.left +=help;
					rect.right -= help;
					drawWeek(hdc, rect);
					
					rect.top = hRect.top;
					rect.bottom -= fontSize + 2;
					}
					if (wpos == BOTH) rect.top += fontSize + 2;

					rect.left = hRect.left;
					rect.right = hRect.right;
					drawRect(hdc,nstyle,rect,bitmapfile[DAYS],NULL);
					
					rect.left +=help;
					rect.right -= help;
					rect.top +=2;

					help = ((rect.bottom - rect.top) - ((int)((rect.bottom - rect.top)/osx))*osx)/2;
					rect.top += help;
					rect.bottom -= help;
					SelectObject(hdc,daysfont);
					SetTextColor(hdc, nfontColor);
					
					drawCalendar(hdc, rect);
					break;
							}
			case VERTICAL:{
					help = ((rect.bottom - rect.top) - ((int)((rect.bottom - rect.top)/osx))*osx)/2;
					
					if ((wpos == POSS1)||(wpos == BOTH)){
					rect.right = rect.left + fontSize + 2;
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);

					rect.top += help;
					rect.bottom -= help;
					
					drawWeek(hdc, rect);
					
					rect = hRect;
					rect.left += fontSize + 2;
					}
					if ((wpos == POSS2)||(wpos == BOTH)){
					rect.left = rect.right - (fontSize + 2);
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);

					rect.top += help;
					rect.bottom -= help;
					
					drawWeek(hdc, rect);
					
					rect = hRect;
					rect.right -= fontSize + 2;
					}
					
					if (wpos == BOTH) rect.left += fontSize + 2;
					
					drawRect(hdc,nstyle,rect,bitmapfile[DAYS],NULL);
					rect.top += help;
					rect.bottom -= help;
					rect.left += 2;
					help = ((rect.right - rect.left) - ((int)((rect.right - rect.left)/osy))*osy)/2;
					rect.left +=help;
					rect.right -= help;
					SelectObject(hdc,daysfont);
					SetTextColor(hdc, nfontColor);
					drawCalendar(hdc, rect);
				//	drawFree(hdc);
					break;
						  }			
			case LINE:{
					int help;
					help = ((rect.right - rect.left) - ((int)((rect.right - rect.left)/daysInMonth[month-1]))*daysInMonth[month-1])/2;
				
					if ((wpos == POSS1)||(wpos == BOTH)){
					rect.bottom = rect.top + fontSize + 2;
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);
					rect.left +=help;
					rect.right -= help;
					drawWeek(hdc, rect);
					rect = hRect;
					rect.top += fontSize + 2;
					}
					if ((wpos == POSS2)||(wpos == BOTH)){
					rect.top = rect.bottom - (fontSize + 2);
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);
					rect.left +=help;
					rect.right -= help;
					drawWeek(hdc, rect);
					rect = hRect;
					rect.bottom -= fontSize + 2;
					}
					if (wpos == BOTH) rect.top += fontSize + 2;
					drawRect(hdc,nstyle,rect,bitmapfile[DAYS],NULL);
				
					rect.left +=help;
					rect.right -= help;
					rect.top +=2;
					rect.bottom -=2;
					SelectObject(hdc,daysfont);
					drawRCalendar(hdc, rect);
					  break;}
			
			case ROW:{


					
					int help;
					help = ((rect.bottom - rect.top) - ((int)((rect.bottom - rect.top)/daysInMonth[month-1]))*daysInMonth[month-1])/2;
					
					if ((wpos == POSS1)||(wpos == BOTH)){
					rect.right = rect.left + fontSize + 2;
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);
					rect.top +=help;
					rect.bottom -= help;
					drawWeek(hdc, rect);
					rect = hRect;
					rect.left += fontSize + 2;
					}

					if ((wpos == POSS2)||(wpos == BOTH)){
					rect.left = rect.right - (fontSize + 2);
					drawRect(hdc,wstyle,rect,bitmapfile[WEEK],NULL);
					rect.top +=help;
					rect.bottom -= help;
					drawWeek(hdc, rect);
					rect = hRect;
					rect.right -= fontSize + 2;
					}
					
					if (wpos == BOTH) rect.left += fontSize + 2;
					drawRect(hdc,nstyle,rect,bitmapfile[DAYS],NULL);
					rect.left +=2;
					rect.right -=2;
					rect.top +=help;
					rect.bottom -= help;
					SelectObject(hdc,daysfont);
					drawRCalendar(hdc, rect);
					 break;}

			case FREEE:{
				        break;}

			}
		
			SelectObject(hdc,oldfont);		
			DeleteObject(otherfont);
			DeleteObject(daysfont);

//----------------------------------------------------------
//overlay bitmap
	if ((!strcmp(bitmapfile[6],".none")==0)&&(FileExists(bitmapfile[6])))
						{
					
					GetClientRect(hwnd, &r);
					HANDLE image;
					image = LoadImage(NULL, bitmapfile[6], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
					
					HDC hdcMem = CreateCompatibleDC(hdc);
					
					//HBITMAP old = (HBITMAP) 
					SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);
					
				//	if (bopt == CENTER) 
				//	TransparentBlt(hdc, re.left + (re.right - re.left)/2 - bitmap.bmWidth/2, re.top + (re.bottom - re.top)/2 - bitmap.bmHeight/2, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
						
				//	else if (bopt == STRETCH)
					TransparentBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xffff00);
				
				//	SelectObject(hdcMem, old);
				//	DeleteObject(old);
					DeleteObject(image);
					DeleteDC(hdcMem);
				}
//--------------------------------------------







				//Paint to the screen 
			BitBlt(hdc_scrn, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
		
			SelectObject(hdc, bufbmp); //mortar: select just incase it is no longer in the context
			
			
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
			setFontColor();
			getCurrentDate();
			InvalidateRect(hwndBBCalendar, NULL, false);
			InvalidateRect(hwndMessage, NULL, true);
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
				ShowWindow( hwndBBCalendar, SW_SHOW);
				InvalidateRect( hwndBBCalendar, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBCalendar, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBCalendarAbout"))
			{
				sprintf(dcaption, "%s About", szVersion);
				
				sprintf(dmessage, "%s \nby %s ©2004\n\n%s\n\n%s",
						szVersion, szInfoAuthor, szInfoEmail, szInfoLink);
				CreateMessageBox();

			}
			else if (!_stricmp(szTemp, "@BBCalendarPluginToggle"))
			{
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBCalendarOnTop"))
			{
				alwaysOnTop = !alwaysOnTop;
				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBCalendar, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBCalendar, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBCalendarSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBCalendar);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBCalendar);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				setFontColor();

				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}

			else if (!_stricmp(szTemp, "@BBCalendarDefaultFont"))
			{
				// Set the transparent attributes to the window
				deffont = !deffont;
			//	setStatus();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}

	/*		else if (!_stricmp(szTemp, "@BBCalendarFullTrans"))
			{
				// Set the transparent attributes to the window
				fullTrans = !fullTrans;
				setStatus();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
		*/	else if (!_stricmp(szTemp, "@BBCalendarSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBCalendarNames"))
			{
				shownames = !shownames;
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarShowGrid"))
			{
				showGrid = !showGrid;
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarSundayFirst"))
			{
				sundayFirst = !sundayFirst;
				getCurrentDate();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarDrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarActualMonth"))
			{
				// Set the snapWindow attributes to the window
				moveMonth = 0;

			//	if (moveMonth == 12) moveMonth = 0;
				getCurrentDate();
				getAlarms();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarNextMonth"))
			{
				// Set the snapWindow attributes to the window
				moveMonth++;

			//	if (moveMonth == 12) moveMonth = 0;
				getCurrentDate();
				getAlarms();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarPreviousMonth"))
			{
				// Set the snapWindow attributes to the window
				moveMonth--;
			//	if (moveMonth == -12) moveMonth = 0;
				getCurrentDate();
				getAlarms();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarEditAlarmsRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, alarmpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarReloadAlarmsRC"))
			{	
				if (!FileExists(alarmpath)) createAlarmFile();
				
				getAlarms();
				InvalidateRect(hwndBBCalendar, NULL, false);

			}
			else if (!_stricmp(szTemp, "@BBCalendarReloadSettings"))
			{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBCalendar);

					//Re-initialize
					ReadRCSettings();
					InitBBCalendar();
					inSlit = wantInSlit;
					GetStyleSettings();
					setFontColor();
					setStatus();

					if ( alwaysOnTop) SetWindowPos( hwndBBCalendar, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBCalendar, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBCalendar);
					else inSlit = false;

					if (!FileExists(alarmpath)) createAlarmFile();
				
					getAlarms();

					InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_stricmp(szTemp, "@BBCalendarSaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDateFont", 18))
			{
				strcpy(fontFace[DATEFONT],szTemp + 20);
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarWeekFont", 18))
			{
				strcpy(fontFace[WEEKFONT],szTemp + 20);
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDaysFont", 18))
			{
				strcpy(fontFace[DAYSFONT],szTemp + 20);
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarTextSize", 18))
			{
				fontSize = atoi(szTemp + 19);
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDateSize", 18))
			{
				dateFontSize = atoi(szTemp + 19);
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarPlacement", 19))
			{
				placement = atoi(szTemp + 20);
				
				place(placement);
			
				InvalidateRect(hwndBBCalendar, NULL, false);



			}
			else if (!_strnicmp(szTemp, "@BBCalendarSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);
				if (transparency)	setStatus();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			
			else if (!_strnicmp(szTemp, "@BBCalendarNBitmap", 17))
			{
				int tmp;
				tmp = atoi(szTemp + 18);
			
				strcpy(bitmapfile[tmp],".none");

				InvalidateRect(hwndBBCalendar, NULL, false);
			}

			else if (!_strnicmp(szTemp, "@BBCalendarLBitmap", 17))
			{
				int tmp;
				tmp = atoi(szTemp + 18);
			
				OPENFILENAME ofn;       // common dialog box structure

			// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndBBCalendar;
				ofn.lpstrFile = bitmapfile[tmp];
				ofn.nMaxFile = sizeof(bitmapfile[tmp]);
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
//				ofn.lpstrInitialDir = defaultpath;
				ofn.lpstrTitle = "BBCalendar Bitmap";
//				ofn.lpstrDefExt = defaultextension;	

				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (!GetOpenFileName(&ofn)) strcpy(bitmapfile[tmp],".none");

				InvalidateRect(hwndBBCalendar, NULL, false);
			}

			else if (!_strnicmp(szTemp, "@BBCalendarDatePossition", 23))
			{
				text_pos = atoi(szTemp + 24);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDrawMode", 18))
			{
				drawMode = atoi(szTemp + 19);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarBOptions", 18))
			{
				bopt = atoi(szTemp + 19);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarAStyle", 16))
			{
				astyle = atoi(szTemp + 17);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarNStyle", 16))
			{
				nstyle = atoi(szTemp + 17);
				setFontColor();	
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarCStyle", 16))
			{
				cstyle = atoi(szTemp + 17);
				setFontColor();			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarWStyle", 16))
			{
				wstyle = atoi(szTemp + 17);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDStyle", 16))
			{
				dstyle = atoi(szTemp + 17);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarCDStyle", 17))
			{
				cdstyle = atoi(szTemp + 18);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarTextA", 15))
			{
				texta = atoi(szTemp + 16);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarWeekP", 15))
			{
				wpos = atoi(szTemp + 16);
			
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarExecute", 17))
			{
				executeAlarm(atoi(szTemp + 18));
			
			}
			else if (!_strnicmp(szTemp, "@BBCalendarDateFormat", 20))
			{
				strcpy(clockformat,szTemp + 22);
				getCurrentDate();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarAdd", 13))
			{
				char datee[12];
				
				strcpy(szTemp,szTemp + 15);
				strncpy(datee,szTemp,11);
				datee[11]=0;
				strcpy(szTemp,szTemp + 12);
			
				WriteString(alarmpath, datee, szTemp);
				getAlarms();
				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarWidth", 15))
			{ 
				width = atoi(szTemp + 16);
				
				if ( alwaysOnTop) SetWindowPos( hwndBBCalendar, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBCalendar, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarHeight", 16))
			{ 
				height = atoi(szTemp + 17);
									
				if ( alwaysOnTop) SetWindowPos( hwndBBCalendar, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBCalendar, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBCalendar, NULL, false);
			}
			else if (!_strnicmp(szTemp, "@BBCalendarMessage", 17))
			{
				
		
				for (UINT i=0;i < strlen(alarm); i++)
				if ((alarm[i]=='\\') && ((alarm[i+1]=='n')||(alarm[i+1]=='N'))) {alarm[i]=' ';alarm[i+1]='\n';}
				
				sprintf(dcaption, "BlackBox Message");
				
				strcpy(dmessage,szTemp + 19);
				CreateMessageBox();
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
			if (placement == 8)
				{
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
				MoveWindow(hwndBBCalendar, xpos, ypos, width, height, true);
				} else 	place(placement);
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
			
			if(!inSlit)
				return DefWindowProc(hwnd,message,wParam,lParam);
		}
		break;

		
		// ==========

		case WM_LBUTTONUP: {
			
			GetCursorPos(&poloha);
			hday = getDay(poloha);
			if (hday != 0) executeAlarm(hday);
			 } break;
		
		// ==========

		// Right mouse button clicked?
		case WM_NCRBUTTONUP:
		{	
			
			char szzTemp[15];
			
			GetCursorPos(&poloha);
			hday = getDay(poloha);

			if (hday != 0) {

			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
			
			sprintf(szTemp,"%.2d.%.2d.%.2d",hday,month,year);
			myMenu = MakeMenu(szTemp);
			strcat(szTemp,": ");
			if (!isAlarm[hday-1])
			MakeMenuItemString(myMenu, "Add", "@BBCalendarAdd", szTemp);
			else 
			
			{
			sprintf(szzTemp,"%.2d.%.2d.%.4d:",hday,month,year);
			strcpy(szTemp,szzTemp);
			strcat(szTemp," ");
			strcat(szTemp, ReadString(alarmpath, szzTemp, "!"));
			MakeMenuItemString(myMenu, "Change", "@BBCalendarAdd", szTemp);

			sprintf(szTemp,"@BBCalendarAdd %.2d.%.2d.%.2d: !%s",hday,month,year,ReadString(alarmpath, szzTemp, "!"));
			MakeMenuItem(myMenu, "Remove", szTemp,false);
			
			sprintf(szTemp,"@BBCalendarExecute %d",hday);
			MakeMenuItem(myMenu, "Execute", szTemp, false);
			}
			// Finally, we show the menu...
			ShowMenu(myMenu);
		//	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}
			}

			else createMenu();
		}
		break;
	
		// ==========

	//	case WM_NCRBUTTONDOWN: {} break;

		// ==========
	/*	case WM_NCLBUTTONDBLCLK: 
		{
			//open control panel with:  control timedate.cpl,system,0
			CreateMessageBox();
			//BBExecute(GetDesktopWindow(), NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
		}
		break;
		
*/
		// ==========
		
		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_TIMER:
				{
					getCurrentDate();
					if ((hour == 0) && (minute < 30)) strcpy(name,getName());
				//	if (((isAlarm[day])||(isMAlarm[day])||(isYAlarm[day])) && (hour == 12) && (minute == 0)) executeAlarm(day);
					InvalidateRect(hwndBBCalendar, NULL, false);
				} break;
			}
		}
		break;


		case WM_TIMECHANGE:
		{
			getCurrentDate();
			strcpy(name,getName());
			InvalidateRect(hwndBBCalendar, NULL, false);
		}
		break;

		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

//===========================================================================

void GetStyleSettings()
{
	// Get the path to the current style file from Blackbox...
	strcpy(stylepath, stylePath());

	// ...and some additional parameters
	bevelWidth = ReadInt(stylepath, "bevelWidth:", 2);
	borderWidth = ReadInt(stylepath, "borderWidth:", 1);
	borderColor = ReadColor(stylepath, "borderColor:", "#000000");

	char tempstyle[MAX_LINE_LENGTH];
	strcpy(tempstyle, ReadString(stylepath, "toolbar:", "Flat Gradient Vertical"));

	
	//toolbar
		     
	if (toolbar) delete toolbar;	//else use the the toolbar: settings
	toolbar = new StyleItem;
	ParseItem(tempstyle, toolbar);	//use original tempstyle if "parentrelative"
	toolbarColor = ReadColor(stylepath, "toolbar.color:", "#000000");		//have to do this if parent relative found, it seems bb4win uses
	toolbarColorTo = ReadColor(stylepath, "toolbar.colorTo:", "#FFFFFF");		//the toolbar.color if parent relative is found for toolbar.clock
	tfontColor = ReadColor(stylepath, "toolbar.textColor:", "#FFFFFF");
		
	
	char tempstyle2[MAX_LINE_LENGTH];
	

	//label
		strcpy(tempstyle2, ReadString(stylepath, "toolbar.label:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (label) delete label;	//if everything is found in toolbar.label: then make a new StyleItem
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
			labelColor = toolbarColor;			//have to do this if parent relative found, it seems bb4win uses
			labelColorTo = toolbarColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			labelfontColor = tfontColor;
		
		}
		

	//clock

		strcpy(tempstyle2, ReadString(stylepath, "toolbar.clock:", "parentrelative"));
		if (!IsInString("", tempstyle2)&&!IsInString(tempstyle2, "parentrelative"))
		{
			if (cclock) delete cclock;	//if everything is found in toolbar.windowLabel: then make a new StyleItem
			cclock = new StyleItem;			
			ParseItem(tempstyle2, cclock);
			
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
			if (cclock) delete cclock;	//else use the the toolbar: settings
			cclock = new StyleItem;
			ParseItem(tempstyle, cclock);	//use original tempstyle if "parentrelative"
			clockColor = toolbarColor;			//have to do this if parent relative found, it seems bb4win uses
			clockColorTo = toolbarColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			clockfontColor = tfontColor;
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
			winlabelColor = toolbarColor;			//have to do this if parent relative found, it seems bb4win uses
			winlabelColorTo = toolbarColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			winlabelfontColor = tfontColor;
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
			buttonColor = toolbarColor;			//have to do this if parent relative found, it seems bb4win uses
			buttonColorTo = toolbarColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			buttonfontColor = tfontColor;
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
			
			buttonprfontColor = ReadColor(stylepath, "toolbar.button.pressed.picColor:", "#FFFFFF");
		}
		else
		{
			if (buttonpr) delete buttonpr;	//else use the the toolbar: settings
			buttonpr = new StyleItem;
			ParseItem(tempstyle, buttonpr);	//use original tempstyle if "parentrelative"
			buttonprColor = toolbarColor;			//have to do this if parent relative found, it seems bb4win uses
			buttonprColorTo = toolbarColorTo;		//the toolbar.color if parent relative is found for toolbar.clock
			buttonprfontColor = tfontColor;
		}
	
    // ...font settings...
	strcpy(fontFace[DEFAULT], ReadString(stylepath, "toolbar.font:", ""));
	if (!_stricmp(fontFace[DEFAULT], "")) strcpy(fontFace[DEFAULT], ReadString(stylepath, "*font:", "Tahoma"));
	
}

void setFontColor()
{
	switch (cstyle)
	{
	case WINLABEL:	fontColor = winlabelfontColor; break;
	case BUTTONPR:	fontColor = buttonprfontColor; break;
	case BUTTON:	fontColor = buttonfontColor; break;
	case CLOCK:		fontColor = clockfontColor; break;
	case TOOLBAR:	fontColor = tfontColor; break;
	case LABEL:		fontColor = labelfontColor; break;
	default:		fontColor = tfontColor; break;
	}

	switch (nstyle)
	{
	//case NONE:		nfontColor = fontColor;
	case WINLABEL:	nfontColor = winlabelfontColor; break;
	case BUTTONPR:	nfontColor = buttonprfontColor; break;
	case BUTTON:	nfontColor = buttonfontColor; break;
	case CLOCK:		nfontColor = clockfontColor; break;
	case TOOLBAR:	nfontColor = tfontColor; break;
	case LABEL:		nfontColor = labelfontColor; break;
	default:		nfontColor = fontColor; break;
	}
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

	strcpy(alarmpath, rcpath);
	strcpy(freeformpath, rcpath);
	strcpy(namepath, rcpath);

	strcat(alarmpath,"alarms.rc");
	strcat(namepath,"names.rc");
	strcat(freeformpath,"freeform.rc");


	if (!FileExists(alarmpath)) createAlarmFile();

	strcat(temp, "bbcalendar.rc");
	strcat(path, "bbcalendarrc");
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
		strcat(temp, "bbcalendar.rc");
		strcat(path, "bbcalendarrc");
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
		//	fullTrans = false;
			showGrid = false;
			drawBorder = true;
			sundayFirst = false;
			pluginToggle = false;
			shownames = false;
			fontSize = dateFontSize = 8;
			strcpy(weekDayNames, "MTWTFSS");
			drawType = false;
			for (int i=0;i<6;i++)
			strcpy(bitmapfile[i], ".none");
			strcpy(clockformat, "%d %a %#H:%M");
			strcpy(fontFace[DATEFONT],fontFace[DEFAULT]);
			strcpy(fontFace[WEEKFONT],fontFace[DEFAULT]);
			strcpy(fontFace[DAYSFONT],fontFace[DEFAULT]);

			WriteRCSettings();
			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbcalendar.x:", 10);
	ypos = ReadInt(rcpath, "bbcalendar.y:", 10);
	width  = ReadInt(rcpath, "bbcalendar.width:", 100);
	height = ReadInt(rcpath, "bbcalendar.height:", 100);
	placement = ReadInt(rcpath, "bbcalendar.placement:", 8);
	if ((placement >8)||(placement<0)) placement = 8;
	if(width < 5 ) width = 100;
	if (height <5) height = 100;
	alpha = ReadInt(rcpath, "bbcalendar.alpha:", 160);
	if(alpha > 255) alpha = 255;
	if(ReadString(rcpath, "bbcalendar.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbcalendar.inSlit:", true);
	
	alwaysOnTop = ReadBool(rcpath, "bbcalendar.alwaysOnTop:", true);
	snapWindow = ReadBool(rcpath, "bbcalendar.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbcalendar.transparency:", false);
//	fullTrans = ReadBool(rcpath, "bbcalendar.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbcalendar.fontSize.text:", 8);
	dateFontSize = ReadInt(rcpath, "bbcalendar.fontSize.date:", 8);
	alwaysOnTop = ReadBool(rcpath, "bbcalendar.alwaysontop:", true);
	shownames = ReadBool(rcpath, "bbcalendar.showNameDay:", false);
	pluginToggle = ReadBool(rcpath, "bbcalendar.pluginToggle:", false);
	showGrid = ReadBool(rcpath, "bbcalendar.showGrid:", false);
	drawBorder = ReadBool(rcpath, "bbcalendar.drawBorder:", true);
	sundayFirst = ReadBool(rcpath, "bbcalendar.sundayFirst:", false);
	
	deffont = ReadBool(rcpath, "bbcalendar.font.default:", false);
	
	strcpy(fontFace[DATEFONT], ReadString(rcpath, "bbcalendar.font.date:", fontFace[DEFAULT]));
	strcpy(fontFace[WEEKFONT], ReadString(rcpath, "bbcalendar.font.week:", fontFace[DEFAULT]));
	strcpy(fontFace[DAYSFONT], ReadString(rcpath, "bbcalendar.font.days:", fontFace[DEFAULT]));


	int helpint;
	
	helpint = ReadInt(rcpath, "bbcalendar.mode:", 11110);
	bopt = helpint % 10;
	helpint = (helpint - bopt)/10;
	wpos = helpint % 10;
	helpint = (helpint - wpos)/10;
	texta = helpint % 10;
	helpint = (helpint - texta)/10;
	text_pos = helpint % 10;
	helpint = (helpint - text_pos)/10;
	drawMode = helpint % 10;

	if ((drawMode<0) || (drawMode>4)) drawMode = 0;
	if ((text_pos<0) || (text_pos>4)) text_pos = 1;
	if ((texta<0) || (texta>3)) texta = 1;
	
	helpint = ReadInt(rcpath, "bbcalendar.styles:", 2222222);
	nstyle = helpint % 10;
	helpint = (helpint - nstyle)/10;
	astyle = helpint % 10;
	helpint = (helpint - astyle)/10;
	cdstyle = helpint % 10;
	helpint = (helpint - cdstyle)/10;
	wstyle = helpint % 10;
	helpint = (helpint - wstyle)/10;
	dstyle = helpint % 10;
	helpint = (helpint - dstyle)/10;
	cstyle = helpint % 10;

	strcpy(clockformat, ReadString(rcpath, "bbcalendar.clockformat:", "%d %a %#H:%M"));
	strcpy(weekDayNames, ReadString(rcpath, "bbcalendar.weekDayNames:", "MTWTFSS"));


	strcpy(bitmapfile[CALENDAR], ReadString(rcpath, "bbcalendar.bitmap.calendar:",	".none"));
	strcpy(bitmapfile[DATEC],	 ReadString(rcpath, "bbcalendar.bitmap.date:",		".none"));
	strcpy(bitmapfile[WEEK],	 ReadString(rcpath, "bbcalendar.bitmap.week:",		".none"));
	strcpy(bitmapfile[DAYS],	 ReadString(rcpath, "bbcalendar.bitmap.days:",		".none"));
	strcpy(bitmapfile[CURRENTD], ReadString(rcpath, "bbcalendar.bitmap.currentd:",	".none"));
	strcpy(bitmapfile[ALARM],	 ReadString(rcpath, "bbcalendar.bitmap.alarm:",		".none"));
	strcpy(bitmapfile[6],    	 ReadString(rcpath, "bbcalendar.bitmap.overlay:",	".none"));


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

		sprintf(szTemp, "! BBCalendar %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		

		sprintf(szTemp, "bbcalendar.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbcalendar.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbcalendar.placement: %d\r\n", placement, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);


		sprintf(szTemp, "bbcalendar.fontSize.text: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.fontSize.date: %d\r\n", dateFontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		
		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.inSlit: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.alwaysOnTop: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.snapWindow: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.transparency: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

	/*	(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.fullTrans: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.pluginToggle: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		(shownames) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.showNameDay: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(showGrid) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.showGrid: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.drawBorder: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(sundayFirst) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.sundayFirst: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		
		sprintf(szTemp, "bbcalendar.styles: %d\r\n", (cstyle*100000 + dstyle*10000 + wstyle*1000 + cdstyle*100 + astyle*10 + nstyle), temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.mode: %d\r\n", (drawMode*10000 + text_pos*1000 + texta*100 + wpos*10 + bopt), temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		
	/*	sprintf(szTemp, "bbcalendar.circleColor: #%.2x%.2x%.2x\r\n", GetRValue(numbColor),GetGValue(numbColor),GetBValue(numbColor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
*/
//		sprintf(szTemp, "bbcalendar.gridColor: #%.2x%.2x%.2x\r\n", GetRValue(gridColor),GetGValue(gridColor),GetBValue(gridColor));
//		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.weekDayNames: %s\r\n", weekDayNames);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbcalendar.clockformat: %s\r\n", clockformat);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);


		(deffont) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbcalendar.font.default: %s\r\n", temp);
 		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "bbcalendar.font.date: %s\r\n", fontFace[DATEFONT]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.font.week: %s\r\n", fontFace[WEEKFONT]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.font.days: %s\r\n", fontFace[DAYSFONT]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		
		sprintf(szTemp, "bbcalendar.bitmap.calendar: %s\r\n", bitmapfile[CALENDAR]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.date: %s\r\n", bitmapfile[DATEC]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.week: %s\r\n", bitmapfile[WEEK]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.days: %s\r\n", bitmapfile[DAYS]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.currentd: %s\r\n", bitmapfile[CURRENTD]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.alarm: %s\r\n", bitmapfile[ALARM]);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbcalendar.bitmap.overlay: %s\r\n", bitmapfile[6]);
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
	if (div(year,4).rem==0) daysInMonth[1]=29;
	else daysInMonth[1]=28;
	
	time(&systemTime);
	localTime = localtime(&systemTime);

	minute	= localTime->tm_min;
	hour	= localTime->tm_hour;


	strftime(szTemp, 10, "%m", localTime);
	month = (int)atoi(szTemp);
	strftime(szTemp, 10, "%Y", localTime);
	year = (int)atoi(szTemp);
	
	strftime(szTemp, 10, "%d", localTime);
	day = (int)atoi(szTemp);

	if (!(moveMonth==0))
	{
	helpTime = localtime(&systemTime);
	helpTime->tm_mday = 1;
	helpTime->tm_year = localTime->tm_year;
	if (month+moveMonth>12) helpTime->tm_year += (month+moveMonth) / 12 ;
	if (month+moveMonth<1) helpTime->tm_year += (month+moveMonth) / 12 ;
	
	helpTime->tm_mon = div(month+moveMonth-1,12).rem;
	
	mktime(helpTime);
	
	strftime(szTemp, 10, "%w", helpTime);
	move = (int)atoi(szTemp);
	
	if (!sundayFirst) move--;
	if ((sundayFirst)&&((drawMode == LINE)||(drawMode == ROW))) move--;
	move = move - 1;//div(1,7).rem;
	if (move<-1) move+=7;

	strftime(szTemp, 10, "%m", helpTime);
	month = (int)atoi(szTemp);

	strftime(szTemp, 10, "%Y", helpTime);
	year = (int)atoi(szTemp);
	strftime(drawclock,256,clockformat,helpTime);	

	}
	else
	{
	strftime(drawclock,256,clockformat,localTime);	
	strftime(szTemp, 10, "%w", localTime);
	move = (int)atoi(szTemp);
	if (!sundayFirst) move--;
	if ((sundayFirst)&&((drawMode == LINE)||(drawMode == ROW))) move--;
	move = move - div(day,7).rem;
	if (move<-1) move+=7;
	}

	if (((div((move + daysInMonth[month-1]+ 1),7).quot)<5)||(((div((move + daysInMonth[month-1]+ 1),7).quot) == 5)&&((div((move + daysInMonth[month-1]+ 1),7).rem) == 0))) osy = 5; else osy = 6;
}		

//---------------------------------------------------------------------------

void mySetTimer()
{
		SetTimer(hwndBBCalendar,	// handle to main window 
				IDT_TIMER,			// timer identifier 
				60000,				// 60-second interval 
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

int beginPluginEx(HINSTANCE hPluginInstance, HWND hwndBBSlit) 
{ 
 inSlit = true; 
 hSlit = hwndBBSlit; 
 
 return beginPlugin(hPluginInstance); 
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
								SetWindowLong(hwndBBCalendar, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBCalendar, 0xff00ff, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
							}
							else
							{
							SetWindowLong(hwndBBCalendar, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
							BBSetLayeredWindowAttributes(hwndBBCalendar, NULL, (unsigned char)alpha, LWA_ALPHA);
							}
						}
						else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
						{
							if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
							{
								SetWindowLong(hwndBBCalendar, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
								BBSetLayeredWindowAttributes(hwndBBCalendar, 0xff00ff, (unsigned char)alpha, LWA_COLORKEY);
							}
							else
							SetWindowLong(hwndBBCalendar, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
						}
							
					}
					else if((transparency)||(fullTrans)) SetWindowLong(hwndBBCalendar, GWL_EXSTYLE, WS_EX_TOOLWINDOW);



}


//===========================================================================
void drawWeek(HDC &hdc, RECT &re)
{
	
	
	switch (drawMode){
	
	case HORIZONTAL:{
		for (int j=0;j<osx;j++)
				{
				r1.left = ((re.right - re.left)/osx)*j + re.left;
				r1.right = ((re.right - re.left)/osx)*(j+1) + re.left;
				r1.top = re.top;
				r1.bottom = re.bottom;
						
				
				if (sundayFirst) sprintf(szTemp,"%c",weekDayNames[div(j+6,7).rem]);
				else sprintf(szTemp,"%c",weekDayNames[j]);
		//		if (showGrid) Rectangle(hdc, (int)r1.left, (int)r1.top, (int)r1.right, (int)r1.bottom);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
					}
					break;}
	case VERTICAL:{
				for (int j=0;j<osx;j++)
				{
				r1.left = re.left;
				r1.right = re.right;
				r1.top = ((re.bottom - re.top)/osx)*j + re.top;
				r1.bottom = ((re.bottom - re.top)/osx)*(j+1) + re.top;
						
				
				if (sundayFirst) sprintf(szTemp,"%c",weekDayNames[div(j+6,7).rem]);
				else sprintf(szTemp,"%c",weekDayNames[j]);
		//		if (showGrid) Rectangle(hdc, (int)r1.left, (int)r1.top, (int)r1.right, (int)r1.bottom);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
					}break;
				  }
	case LINE:{
			//	if (sundayFirst) move++;
				for (int j=0;j<daysInMonth[month-1];j++)
				{
				r1.left = ((re.right - re.left)/daysInMonth[month-1])*j + re.left;
				r1.right = ((re.right - re.left)/daysInMonth[month-1])*(j+1) + re.left;
				r1.top = re.top;
				r1.bottom = re.bottom;
				
								sprintf(szTemp,"%c",weekDayNames[(j+move+1)%7]);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
				}
					break;}
	case ROW:{
				
			//	if (sundayFirst) move++;
				for (int j=0;j<daysInMonth[month-1];j++)
				{
				r1.left = re.left;
				r1.right = re.right;
				r1.top = ((re.bottom - re.top)/daysInMonth[month-1])*j + re.top;
				r1.bottom = ((re.bottom - re.top)/daysInMonth[month-1])*(j+1) + re.top;
				
				
				sprintf(szTemp,"%c",weekDayNames[(j+move+1)%7]);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
				}break;
			 }
	}
}
void drawCalendar(HDC &hdc, RECT &re)
{
	
	//	drawRect(hdc,wstyle,re,NULL,NULL);
	if (drawMode == HORIZONTAL)
			{
			for (int i=0;i<osy;i++)
				for (int j=0;j<osx;j++)
				{
				r1.left = ((re.right - re.left)/osx)*j + re.left;
				r1.right = ((re.right - re.left)/osx)*(j+1) + re.left;
				r1.top = ((re.bottom - re.top)/osy)*i + re.top;
				r1.bottom = ((re.bottom - re.top)/osy)*(i+1) + re.top;
						
							
				if (((j+(osx*i)-move)<=(daysInMonth[month-1]))&&(j+(osx*i)-move>0))	
					{
					sprintf(szTemp,"%d",(j+(osx*i))-move);
					if (showGrid) drawRect(hdc,RECTT,r1,NULL,NULL);//Rectangle(hdc, (int)r1.left, (int)r1.top, (int)r1.right, (int)r1.bottom);
		
					}

				else strcpy(szTemp,"  ");
		//		}

				SetTextColor(hdc,nfontColor);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
				
				
				if (((j+(osx*i)-move-1)>=0) && ((j+(osx*i)-move-1) < daysInMonth[month-1]))
				{
					dayRect[j+(osx*i)-move-1] = r1;
					if ((isAlarm[j+(osx*i)-move-1])||(isMAlarm[j+(osx*i)-move-1])||(isYAlarm[j+(osx*i)-move-1]))
					{
						drawRect(hdc,astyle,r1,bitmapfile[ALARM],szTemp);
					}
				}
				
				
				if ((j+(osx*i)-move==day)&&(moveMonth==0)) 
					{ 
						drawRect(hdc,cdstyle,r1,bitmapfile[CURRENTD],szTemp);	
					}
				}
				}
				
	else if (drawMode == VERTICAL)
			{
			for (int i=0;i<osy;i++)
				for (int j=0;j<osx;j++)
				{
				r1.left = ((re.right - re.left)/osy)*i + re.left;
				r1.right = ((re.right - re.left)/osy)*(i+1) + re.left;
				r1.top = ((re.bottom - re.top)/osx)*j + re.top;
				r1.bottom = ((re.bottom - re.top)/osx)*(j+1) + re.top;
						
							
				if (((j+(osx*i)-move)<=(daysInMonth[month-1]))&&(j+(osx*i)-move>0))	
					{
					sprintf(szTemp,"%d",(j+(osx*i))-move);
					if (showGrid) drawRect(hdc,RECTT,r1,NULL,NULL);//Rectangle(hdc, (int)r1.left, (int)r1.top, (int)r1.right, (int)r1.bottom);
		
					}

				else strcpy(szTemp,"  ");

				SetTextColor(hdc,nfontColor);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			
				
				if (((j+(osx*i)-move-1)>=0) && ((j+(osx*i)-move-1) < daysInMonth[month-1]))
				{
					dayRect[j+(osx*i)-move-1] = r1;
					if ((isAlarm[j+(osx*i)-move-1])||(isMAlarm[j+(osx*i)-move-1])||(isYAlarm[j+(osx*i)-move-1]))
					{
						drawRect(hdc,astyle,r1,bitmapfile[ALARM],szTemp);
					}
				}
				if ((j+(osx*i)-move==day)&&(moveMonth==0)) 
					{ 
				
					drawRect(hdc,cdstyle,r1,bitmapfile[CURRENTD],szTemp);	
					}
			}
	}	
}		


void drawRCalendar(HDC &hdc, RECT &re)
{

	
				for (int j=0;j<daysInMonth[month-1];j++)
				{
			
				if (drawMode == LINE){
				r1.left = ((re.right - re.left)/daysInMonth[month-1])*j + re.left;
				r1.right = ((re.right - re.left)/daysInMonth[month-1])*(j+1) + re.left;
				r1.top = re.top;
				r1.bottom = re.bottom;
				}else if (drawMode == ROW){
				r1.left = re.left;//((re.right - re.left)/daysInMonth[month-1])*j + re.left;
				r1.right = re.right;//((re.right - re.left)/daysInMonth[month-1])*(j+1) + re.left;
				r1.top = ((re.bottom - re.top)/daysInMonth[month-1])*j + re.top;
				r1.bottom = ((re.bottom - re.top)/daysInMonth[month-1])*(j+1) + re.top;
				}
							
				if ((j<(daysInMonth[month-1]))&&(j>=0))	
					{
					sprintf(szTemp,"%d",j+1);
					dayRect[j] = r1;
					if (showGrid) drawRect(hdc,RECTT,r1,NULL,NULL);//Rectangle(hdc, (int)r1.left, (int)r1.top, (int)r1.right, (int)r1.bottom);
		
					}

				else strcpy(szTemp,"  ");
				SetTextColor(hdc,nfontColor);
				DrawText(hdc, szTemp, -1, &r1, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
			

				
				if ((j>=0) && (j <= daysInMonth[month-1]))
					if ((isAlarm[j])||(isMAlarm[j])||(isYAlarm[j])) 
					{
						drawRect(hdc,astyle,r1,bitmapfile[ALARM],szTemp);
					}

				if ((j+1==day)&&(moveMonth==0)) 
					{ 
				
					drawRect(hdc,cdstyle,r1,bitmapfile[CURRENTD],szTemp);	
					}
		
				}
}

void drawFree(HDC &hdc)
{

for (int n=1;n<daysInMonth[month-1];n++){

RECT ur;
int s;
sprintf(htime,"day%i.x:",n);

ur.left = ReadInt(freeformpath, htime, 1);
sprintf(htime,"day%i.y:",n);
ur.top = ReadInt(freeformpath, htime, 1);
sprintf(htime,"day%i.s:",n);
s = ReadInt(freeformpath, htime, 1);
ur.right = ur.left + 100;
ur.bottom = ur.top + 100;
char *nn;
itoa(n,nn,10); 

drawRect(hdc,BUTTON,ur,NULL,"dfsafdf");
DrawText(hdc, "dddd", -1, &ur, DT_VCENTER | DT_CENTER | DT_SINGLELINE);}

}

void drawName(HDC &hdc, RECT &re)
{

	DrawText(hdc, name, -1, &re, DT_VCENTER | DT_CENTER | DT_SINGLELINE);	
}

LPCSTR getName()
{

		sprintf(htime,"%.2d.%.2d:",day,month);

		return ReadString(namepath, htime, "!");
}

void conRect(RECT &recttc, int con)
{
	recttc.left += con;
	recttc.right -= con;
	recttc.top +=con;
	recttc.bottom -=con;
}

int getDay(POINT &pos)
{	
	 RECT windowRect;
	GetWindowRect(hwndBBCalendar,&windowRect);
	pos.x -= windowRect.left;
	pos.y -= windowRect.top;
	for (int i=0;i<daysInMonth[month-1];i++)
		if	(PtInRect(&dayRect[i],pos)) return i+1;


	return 0;
	
}
//------------------------------------------------------------------------------------------
void getAlarms()
{

	for (int i = 0;i<31;i++) 
	{
		isAlarm[i] = false;
		sprintf(htime,"%.2d.%.2d.%.4d:",i+1,month,year);

		strcpy(alarm, ReadString(alarmpath, htime, "!"));

		if (alarm[0] != '!') isAlarm[i] = true;
	}
	
	for (i = 0;i<31;i++) 
	{
		isMAlarm[i] = false;
		sprintf(htime,"%.2d.NN.NNNN:",i+1);

		strcpy(alarm, ReadString(alarmpath, htime, "!"));

		if (alarm[0] != '!') isMAlarm[i] = true;
	}

	for (i = 0;i<31;i++) 
	{
		isYAlarm[i] = false;
		sprintf(htime,"%.2d.%.2d.NNNN:",i+1,month);

		strcpy(alarm, ReadString(alarmpath, htime, "!"));

		if (alarm[0] != '!') isYAlarm[i] = true;
	}

}
//------------------------------------------------------------------------------------------

void executeAlarm(int ddd)
{
//	char mess[MAX_PATH];
	bool create = false;
	sprintf(htime,"%.2d.%.2d.%.4d:",ddd,month,year);

	strcpy(alarm, ReadString(alarmpath, htime, "!"));
	strcpy(dmessage,"");
	//MessageBox(hwndBlackbox, alarm, szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);

	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);
	else if(alarm[0] != '!')
	{
			//static char mess[MAX_LINE_LENGTH];
			for (UINT i=0;i < strlen(alarm); i++)
				if ((alarm[i]=='\\') && ((alarm[i+1]=='n')||(alarm[i+1]=='N'))) {alarm[i]=' ';alarm[i+1]='\n';}
				
				sprintf(dcaption, "%d/%d/%d %s",ddd,month,year,szAppName);
				
				strcpy(dmessage,alarm);
				create = true;
			//	CreateMessageBox();
			
	}
	
//	if (isMAlarm[ddd+1]){
	sprintf(htime,"%.2d.NN.NNNN:",ddd);

	strcpy(alarm, ReadString(alarmpath, htime, "!"));
	
	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);
	else if(alarm[0] != '!')
	{
			//static char mess[MAX_LINE_LENGTH];
			for (UINT i=0;i < strlen(alarm); i++)
				if ((alarm[i]=='\\') && ((alarm[i+1]=='n')||(alarm[i+1]=='N'))) {alarm[i]=' ';alarm[i+1]='\n';}
				
				sprintf(dcaption, "%d/%d/%d %s",ddd,month,year,szAppName);
				if (create) strcat(dmessage,"\n \n");
				strcat(dmessage,alarm);
				create = true;
			//	CreateMessageBox();
			
	}
//	}
	
//	if (isYAlarm[ddd+1]){
	sprintf(htime,"%.2d.%.2d.NNNN:",ddd,month);

	strcpy(alarm, ReadString(alarmpath, htime, "!"));
	
	if (alarm[0] == '@')
	SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);
	else if(alarm[0] != '!')
	{
			//static char mess[MAX_LINE_LENGTH];
			for (UINT i=0;i < strlen(alarm); i++)
				if ((alarm[i]=='\\') && ((alarm[i+1]=='n')||(alarm[i+1]=='N'))) {alarm[i]=' ';alarm[i+1]='\n';}
				
				sprintf(dcaption, "%d/%d/%d %s",ddd,month,year,szAppName);
				if (create) strcat(dmessage,"\n \n");
				strcat(dmessage,alarm);
				create = true;
			//	CreateMessageBox();
			
	}
//	}
	if (create) CreateMessageBox();
}
//-----------------------------------------------------------------------
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

		sprintf(szTemp, "! BBCalendar %s alarms file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! Enter alarms here - one per line.\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! For example:\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! 01.08.2004: @BB8BallFortune \r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! 15.12.2003: Show This Message...\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		
		sprintf(szTemp, "! 15.NN.NNNN: Every month alarm...\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "! 28.03.NNNN: Every year alarm...\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
	}
	CloseHandle(file);
}
///////////////------------------------------------------------------------------------------
void createMenu()
{

//			bool tempBool = false;
			// First we delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
			if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

			
			alarmStyleSubmenu = MakeMenu("Alarm Style");
			MakeMenuItem(alarmStyleSubmenu, "None", "@BBCalendarAStyle 0", (astyle == NONE));
			MakeMenuItem(alarmStyleSubmenu, "Bitmap", "@BBCalendarAStyle 1", (astyle == BMP));
			MakeMenuItem(alarmStyleSubmenu, "Toolbar", "@BBCalendarAStyle 2", (astyle == TOOLBAR));
			MakeMenuItem(alarmStyleSubmenu, "Button", "@BBCalendarAStyle 3", (astyle == BUTTON));
			MakeMenuItem(alarmStyleSubmenu, "ButtonPr", "@BBCalendarAStyle 4", (astyle == BUTTONPR));
			MakeMenuItem(alarmStyleSubmenu, "Label", "@BBCalendarAStyle 5", (astyle == LABEL));
			MakeMenuItem(alarmStyleSubmenu, "WinLabel", "@BBCalendarAStyle 6", (astyle == WINLABEL));
			MakeMenuItem(alarmStyleSubmenu, "Clock", "@BBCalendarAStyle 7", (astyle == CLOCK));
			MakeMenuItem(alarmStyleSubmenu, "Rect", "@BBCalendarAStyle 8", (astyle == RECTT));
			MakeMenuItem(alarmStyleSubmenu, "Trans", "@BBCalendarAStyle 9", (astyle == TRANS));
			
			cdayStyleSubmenu = MakeMenu("Current Day Style");
			MakeMenuItem(cdayStyleSubmenu, "None", "@BBCalendarCDStyle 0", (cdstyle == NONE));
			MakeMenuItem(cdayStyleSubmenu, "Bitmap", "@BBCalendarCDStyle 1", (cdstyle == BMP));
			MakeMenuItem(cdayStyleSubmenu, "Toolbar", "@BBCalendarCDStyle 2", (cdstyle == TOOLBAR));
			MakeMenuItem(cdayStyleSubmenu, "Button", "@BBCalendarCDStyle 3", (cdstyle == BUTTON));
			MakeMenuItem(cdayStyleSubmenu, "ButtonPr", "@BBCalendarCDStyle 4", (cdstyle == BUTTONPR));
			MakeMenuItem(cdayStyleSubmenu, "Label", "@BBCalendarCDStyle 5", (cdstyle == LABEL));
			MakeMenuItem(cdayStyleSubmenu, "WinLabel", "@BBCalendarCDStyle 6", (cdstyle == WINLABEL));
			MakeMenuItem(cdayStyleSubmenu, "Clock", "@BBCalendarCDStyle 7", (cdstyle == CLOCK));
			MakeMenuItem(cdayStyleSubmenu, "Rect", "@BBCalendarCDStyle 8", (cdstyle == RECTT));
			MakeMenuItem(cdayStyleSubmenu, "Trans", "@BBCalendarCDStyle 9", (cdstyle == TRANS));

			calendarStyleSubmenu = MakeMenu("Calendar Style");
		//	MakeMenuItem(calendarStyleSubmenu, "None", "@BBCalendarCStyle 0", (cstyle == NONE));
		//	MakeMenuItem(calendarStyleSubmenu, "Bitmap", "@BBCalendarCStyle 1", (cstyle == BMP));
			MakeMenuItem(calendarStyleSubmenu, "Toolbar", "@BBCalendarCStyle 2", (cstyle == TOOLBAR));
			MakeMenuItem(calendarStyleSubmenu, "Button", "@BBCalendarCStyle 3", (cstyle == BUTTON));
			MakeMenuItem(calendarStyleSubmenu, "ButtonPr", "@BBCalendarCStyle 4", (cstyle == BUTTONPR));
			MakeMenuItem(calendarStyleSubmenu, "Label", "@BBCalendarCStyle 5", (cstyle == LABEL));
			MakeMenuItem(calendarStyleSubmenu, "WinLabel", "@BBCalendarCStyle 6", (cstyle == WINLABEL));
			MakeMenuItem(calendarStyleSubmenu, "Clock", "@BBCalendarCStyle 7", (cstyle == CLOCK));
		//	MakeMenuItem(calendarStyleSubmenu, "Rect", "@BBCalendarCStyle 8", (cstyle == RECTT));
			MakeMenuItem(calendarStyleSubmenu, "Trans", "@BBCalendarCStyle 9", (cstyle == TRANS));


			weekStyleSubmenu = MakeMenu("Week Style");
			MakeMenuItem(weekStyleSubmenu, "None", "@BBCalendarWStyle 0", (wstyle == NONE));
			MakeMenuItem(weekStyleSubmenu, "Bitmap", "@BBCalendarWStyle 1", (wstyle == BMP));
			MakeMenuItem(weekStyleSubmenu, "Toolbar", "@BBCalendarWStyle 2", (wstyle == TOOLBAR));
			MakeMenuItem(weekStyleSubmenu, "Button", "@BBCalendarWStyle 3", (wstyle == BUTTON));
			MakeMenuItem(weekStyleSubmenu, "ButtonPr", "@BBCalendarWStyle 4", (wstyle == BUTTONPR));
			MakeMenuItem(weekStyleSubmenu, "Label", "@BBCalendarWStyle 5", (wstyle == LABEL));
			MakeMenuItem(weekStyleSubmenu, "WinLabel", "@BBCalendarWStyle 6", (wstyle == WINLABEL));
			MakeMenuItem(weekStyleSubmenu, "Clock", "@BBCalendarWStyle 7", (wstyle == CLOCK));
			MakeMenuItem(weekStyleSubmenu, "Rect", "@BBCalendarWStyle 8", (wstyle == RECTT));
			MakeMenuItem(weekStyleSubmenu, "Trans", "@BBCalendarWStyle 9", (wstyle == TRANS));

			dateStyleSubmenu = MakeMenu("Date Style");
			MakeMenuItem(dateStyleSubmenu, "None", "@BBCalendarDStyle 0", (dstyle == NONE));
			MakeMenuItem(dateStyleSubmenu, "Bitmap", "@BBCalendarDStyle 1", (dstyle == BMP));
			MakeMenuItem(dateStyleSubmenu, "Toolbar", "@BBCalendarDStyle 2", (dstyle == TOOLBAR));
			MakeMenuItem(dateStyleSubmenu, "Button", "@BBCalendarDStyle 3", (dstyle == BUTTON));
			MakeMenuItem(dateStyleSubmenu, "ButtonPr", "@BBCalendarDStyle 4", (dstyle == BUTTONPR));
			MakeMenuItem(dateStyleSubmenu, "Label", "@BBCalendarDStyle 5", (dstyle == LABEL));
			MakeMenuItem(dateStyleSubmenu, "WinLabel", "@BBCalendarDStyle 6", (dstyle == WINLABEL));
			MakeMenuItem(dateStyleSubmenu, "Clock", "@BBCalendarDStyle 7", (dstyle == CLOCK));
			MakeMenuItem(dateStyleSubmenu, "Rect", "@BBCalendarDStyle 8", (dstyle == RECTT));
			MakeMenuItem(dateStyleSubmenu, "Trans", "@BBCalendarDStyle 9", (dstyle == TRANS));

			daysStyleSubmenu = MakeMenu("Days Style");
			MakeMenuItem(daysStyleSubmenu, "None", "@BBCalendarNStyle 0", (nstyle == NONE));
			MakeMenuItem(daysStyleSubmenu, "Bitmap", "@BBCalendarNStyle 1", (nstyle == BMP));
			MakeMenuItem(daysStyleSubmenu, "Toolbar", "@BBCalendarNStyle 2", (nstyle == TOOLBAR));
			MakeMenuItem(daysStyleSubmenu, "Button", "@BBCalendarNStyle 3", (nstyle == BUTTON));
			MakeMenuItem(daysStyleSubmenu, "ButtonPr", "@BBCalendarNStyle 4", (nstyle == BUTTONPR));
			MakeMenuItem(daysStyleSubmenu, "Label", "@BBCalendarNStyle 5", (nstyle == LABEL));
			MakeMenuItem(daysStyleSubmenu, "WinLabel", "@BBCalendarNStyle 6", (nstyle == WINLABEL));
			MakeMenuItem(daysStyleSubmenu, "Clock", "@BBCalendarNStyle 7", (nstyle == CLOCK));
			MakeMenuItem(daysStyleSubmenu, "Rect", "@BBCalendarNStyle 8", (nstyle == RECTT));
			MakeMenuItem(daysStyleSubmenu, "Trans", "@BBCalendarNStyle 9", (nstyle == TRANS));


			windowStyleSubmenu = MakeMenu("Window Style");
			MakeSubmenu(windowStyleSubmenu, calendarStyleSubmenu, "Calendar");
			MakeSubmenu(windowStyleSubmenu, dateStyleSubmenu, "Date");
			MakeSubmenu(windowStyleSubmenu, weekStyleSubmenu, "Week");
			MakeSubmenu(windowStyleSubmenu, daysStyleSubmenu, "Days");
			MakeSubmenu(windowStyleSubmenu, cdayStyleSubmenu, "Current Day");
			MakeSubmenu(windowStyleSubmenu, alarmStyleSubmenu, "Alarm");
			
			
			textaSubmenu = MakeMenu("Text Alligment");
			MakeMenuItem(textaSubmenu, "Left", "@BBCalendarTextA 0", (texta == 0));
			MakeMenuItem(textaSubmenu, "Center", "@BBCalendarTextA 1", (texta == 1));
			MakeMenuItem(textaSubmenu, "Right", "@BBCalendarTextA 2", (texta == 2));

			weekSubmenu = MakeMenu("Week Possition");
			MakeMenuItem(weekSubmenu, "None", "@BBCalendarWeekP 0", (wpos == 0));
			MakeMenuItem(weekSubmenu, "Possition 1", "@BBCalendarWeekP 1", (wpos == 1));
			MakeMenuItem(weekSubmenu, "Possition 2", "@BBCalendarWeekP 2", (wpos == 2));
			MakeMenuItem(weekSubmenu, "Both", "@BBCalendarWeekP 3", (wpos == 3));

			dateSubmenu = MakeMenu("Date Position");
			MakeSubmenu(dateSubmenu, textaSubmenu, "Text Alligment");
			MakeMenuItem(dateSubmenu, "No Date", "@BBCalendarDatePossition 0", (text_pos == 0));
			MakeMenuItem(dateSubmenu, "Top", "@BBCalendarDatePossition 1", (text_pos == 1));
			MakeMenuItem(dateSubmenu, "Bottom", "@BBCalendarDatePossition 2", (text_pos == 2));
			MakeMenuItem(dateSubmenu, "Left", "@BBCalendarDatePossition 3", (text_pos == 3));
			MakeMenuItem(dateSubmenu, "Right", "@BBCalendarDatePossition 4", (text_pos == 4));

			modeSubmenu = MakeMenu("Draw Mode");
			MakeMenuItem(modeSubmenu, "Horizontal", "@BBCalendarDrawMode 0", (drawMode == 0));
			MakeMenuItem(modeSubmenu, "Vertical", "@BBCalendarDrawMode 1", (drawMode == 1));
			MakeMenuItem(modeSubmenu, "Line", "@BBCalendarDrawMode 2", (drawMode == 2));
			MakeMenuItem(modeSubmenu, "Row", "@BBCalendarDrawMode 3", (drawMode == 3));
			MakeMenuItem(modeSubmenu, "Free", "@BBCalendarDrawMode 4", (drawMode == 4));
		
		
			configSubmenu = MakeMenu("Configuration");

			generalConfigSubmenu = MakeMenu("General");
			if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBCalendarSlit", wantInSlit);
			MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBCalendarPluginToggle", pluginToggle);
			MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBCalendarOnTop", alwaysOnTop);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBCalendarTransparent", transparency);
			if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBCalendarSetTransparent",alpha,0,255);
		/*	if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
				MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBCalendarFullTrans", fullTrans);
		*/	MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBCalendarSnapToEdge", snapWindow);
		
			monthSubmenu = MakeMenu("Month");
			MakeMenuItem(monthSubmenu, "Next", "@BBCalendarNextMonth", false);
			MakeMenuItem(monthSubmenu, "Previous", "@BBCalendarPreviousMonth", false);
			MakeMenuItem(monthSubmenu, "Actual", "@BBCalendarActualMonth", false);
			
			fontSubmenu = MakeMenu("Fonts");
			MakeMenuItem(fontSubmenu, "Use Default Fonts", "@BBCalendarDefaultFont", deffont);
			MakeMenuItemString(fontSubmenu, "Date Font", "@BBCalendarDateFont", fontFace[DATEFONT]);
			MakeMenuItemString(fontSubmenu, "Week Font", "@BBCalendarWeekFont", fontFace[WEEKFONT]);
			MakeMenuItemString(fontSubmenu, "Days Font", "@BBCalendarDaysFont", fontFace[DAYSFONT]);
			MakeMenuItemInt(fontSubmenu, "Date Size", "@BBCalendarDateSize", dateFontSize, 6, 72);
			MakeMenuItemInt(fontSubmenu, "Week + Days Size", "@BBCalendarTextSize", fontSize, 6, 72);
			
			placementSubmenu = MakeMenu("Placement");
			MakeMenuItem(placementSubmenu, "Top Left", "@BBCalendarPlacement 0", placement == 0);
			MakeMenuItem(placementSubmenu, "Top Center", "@BBCalendarPlacement 1", placement == 1);
			MakeMenuItem(placementSubmenu, "Top Right", "@BBCalendarPlacement 2", placement == 2);
			MakeMenuItem(placementSubmenu, "Center Left", "@BBCalendarPlacement 3", placement == 3);
			MakeMenuItem(placementSubmenu, "Center Right", "@BBCalendarPlacement  4", placement == 4);
			MakeMenuItem(placementSubmenu, "Bottom Left", "@BBCalendarPlacement 5", placement == 5);
			MakeMenuItem(placementSubmenu, "Bottom Center", "@BBCalendarPlacement  6", placement == 6);
			MakeMenuItem(placementSubmenu, "Bottom Right", "@BBCalendarPlacement 7", placement == 7);
			MakeMenuItem(placementSubmenu, "Custom", "@BBCalendarPlacement  8", placement == 8);

			calendarConfigSubmenu = MakeMenu("Other");
			MakeMenuItem(calendarConfigSubmenu, "Draw Border", "@BBCalendarDrawBorder", drawBorder);
			MakeMenuItem(calendarConfigSubmenu, "Show Grid", "@BBCalendarShowGrid", showGrid);
			MakeMenuItem(calendarConfigSubmenu, "Sunday First", "@BBCalendarSundayFirst", sundayFirst);
			MakeMenuItem(calendarConfigSubmenu, "Show Nameday", "@BBCalendarNames", shownames);
			MakeMenuItemInt(calendarConfigSubmenu, "Width", "@BBCalendarWidth", width, 20, 1280);
			MakeMenuItemInt(calendarConfigSubmenu, "Height", "@BBCalendarHeight", height, 20, 1024);
			MakeMenuItemString(calendarConfigSubmenu, "Date Format", "@BBCalendarDateFormat", clockformat);
	
			settingsSubmenu = MakeMenu("Settings");
			MakeMenuItem(settingsSubmenu, "Edit Alarms", "@BBCalendarEditAlarmsRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Alarms", "@BBCalendarReloadAlarmsRC", false);
			MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBCalendarEditRC", false);
			MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBCalendarReloadSettings", false);
			MakeMenuItem(settingsSubmenu, "Save Settings", "@BBCalendarSaveSettings", false);
	
			bitmapSubmenu = MakeMenu("Bitmap");
			
			browseSubmenu = MakeMenu("Calendar Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 0", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 0", (strcmp(bitmapfile[CALENDAR],".none")==0));
				
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Calendar");
		//	DelMenu(browseSubmenu);
			browseSubmenu = MakeMenu("Date Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 1", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 1", (strcmp(bitmapfile[DATEC],".none")==0));
				
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Date");
			browseSubmenu = MakeMenu("Week Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 2", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 2", (strcmp(bitmapfile[WEEK],".none")==0));
	
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Week");

			browseSubmenu = MakeMenu("Days Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 3", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 3", (strcmp(bitmapfile[DAYS],".none")==0));
	
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Days");

			browseSubmenu = MakeMenu("Current Day Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 4", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 4", (strcmp(bitmapfile[CURRENTD],".none")==0));
	
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Current Day");

			browseSubmenu = MakeMenu("Alarm Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 5", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 5", (strcmp(bitmapfile[ALARM],".none")==0));
	
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Alarm");

			browseSubmenu = MakeMenu("Overlay Bitmap");
			MakeMenuItem(browseSubmenu, "Browse...", "@BBCalendarLBitmap 6", false);
			MakeMenuItem(browseSubmenu, "Nothing", "@BBCalendarNBitmap 6", (strcmp(bitmapfile[6],".none")==0));
	
			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Overlay");

			
			imageOptionsSubmenu = MakeMenu("Bitmap Options");
			MakeMenuItem(imageOptionsSubmenu, "Center", "@BBCalendarBOptions 0", (bopt == CENTER));
			MakeMenuItem(imageOptionsSubmenu, "Stretch", "@BBCalendarBOptions 1", (bopt == STRETCH));
			

			imageSubmenu = MakeMenu("Image");
			MakeSubmenu(imageSubmenu, bitmapSubmenu, "Bitmap");
			MakeSubmenu(imageSubmenu, imageOptionsSubmenu, "Bitmap Options");


			myMenu = MakeMenu(szVersion);
			
			MakeSubmenu(configSubmenu, modeSubmenu, "Draw Mode");
			MakeSubmenu(configSubmenu, dateSubmenu, "Date");
			MakeSubmenu(configSubmenu, weekSubmenu, "Week");
			MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
			MakeSubmenu(configSubmenu, fontSubmenu, "Fonts");
			MakeSubmenu(configSubmenu, imageSubmenu, "Image");
			MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
			MakeSubmenu(configSubmenu, placementSubmenu, "Placement");
			MakeSubmenu(configSubmenu, calendarConfigSubmenu, "Other");
			
			
		//	MakeSubmenu(configSubmenu, sizeSubmenu, "Sizes");
			MakeSubmenu(myMenu, monthSubmenu, "Month");
			MakeSubmenu(myMenu, configSubmenu, "Configuration");
			MakeSubmenu(myMenu, settingsSubmenu, "Settings");
			MakeMenuItem(myMenu, "About", "@BBCalendarAbout", false);
			
			
			// Finally, we show the menu...
			ShowMenu(myMenu);


}

//=============================================================================================
//=============================================================================================

int CreateMessageBox()
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


	 //*************
	HDC hdc = CreateCompatibleDC(NULL);
	HFONT oldf;
	HFONT otherfont = CreateFont( 14,   //pBBCalendar->text.fontSize,
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace[DEFAULT]);
		 oldf =   (HFONT)SelectObject(hdc, otherfont);
	RECT sss;
	sss.right = 10;  sss.left = 0; sss.bottom = 10; sss.top = 0;
	DrawText(hdc, dmessage, -1, &sss, DT_CENTER | DT_VCENTER | DT_CALCRECT);
	SelectObject(hdc,oldf);
	DeleteObject(otherfont);
	DeleteDC(hdc);
	//*************


	hwndMessage = CreateWindowEx(
						WS_EX_TOOLWINDOW,								// window style
						"Message",										// our window class name
						NULL,											// NULL -> does not show up in task manager!
						WS_POPUP ,	// window parameters
						ScreenWidth/2 - ((((sss.right - sss.left)+20)<200)?100:((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2)/2),											// x position
						ScreenHeight/2 - ((sss.bottom - sss.top)+60+(bevelWidth + borderWidth)*2)/2,											// y position
						(((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2)<200)?200:((sss.right - sss.left)+20+(bevelWidth + borderWidth)*2),											// window width
						((sss.bottom - sss.top)+50) + (bevelWidth + borderWidth)*2,											// window height
						NULL, //hwndBBCalendar,									// parent window
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
		//	rec = r;
			

			//to prevent flicker of the display, we draw to memory first,
            //then put it on screen in one single operation. This is like this:

            //first get a new 'device context'
			HDC hdc = CreateCompatibleDC(NULL);

			
			HBITMAP bufbmp = CreateCompatibleBitmap(hdc_scrn, r.right, r.bottom);
            SelectObject(hdc, bufbmp);
			

			SetBkMode(hdc, TRANSPARENT);

	
			//if(drawBorder)
			//{
					drawRect(hdc,BORDER,r,NULL,NULL);
					conRect(r,borderWidth);
					/*r.left += borderWidth;
					r.top += borderWidth;
					r.bottom -= borderWidth;
					r.right = r.right - borderWidth;
					*/	
			//}
						
			rec = r;
			r.bottom = r.top + 18;
			
			HFONT oldfont;
			HFONT otherfont = CreateFont( 14,   //pBBCalendar->text.fontSize,
						0, 0, 0, FW_NORMAL,
						false, false, false,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS,
						DEFAULT_QUALITY,
						DEFAULT_PITCH,
						fontFace[DEFAULT]);
		    oldfont = (HFONT)SelectObject(hdc, otherfont);
			
			drawRect(hdc,TOOLBAR,r,NULL,dcaption);
			
			SetTextColor(hdc, fontColor);

			r.bottom = rec.bottom;
			r.top = r.top + bevelWidth + 18;

			
			drawRect(hdc,LABEL,r,NULL,NULL);
			
			
			r.top = rec.top;
			
			r.top += 23;
			r.bottom -=23;
			
			SetTextColor(hdc, labelfontColor);
			DrawText(hdc, dmessage, -1, &r, DT_CENTER | DT_VCENTER);
			r.bottom +=23;
			r.left = r.right - 53;
			r.right = r.right - 3;
			r.top = r.bottom - 23;
			r.bottom = r.bottom - 3;

			okRect = r;

			if (!pressed)	drawRect(hdc,BUTTON,r,NULL,"OK");
			else			drawRect(hdc,BUTTONPR,r,NULL,"OK");
				
			SelectObject(hdc,oldfont);
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
				InvalidateRect(hwndMessage, NULL, true);
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
				DestroyMessageBox();
				pressed = false;
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
		}
		break;
	
		// ==========

		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;
}

void place(int where)
{
	if(!inSlit || !hSlit)
				{
 
			/*	int relx, rely;
				int oldscreenwidth = ScreenWidth;
				int oldscreenheight = ScreenHeight;
			*/	
				ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
				ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
				
				switch (where){
					case 0: xpos = 0; ypos = 0; break;
					case 1: xpos = (int)(ScreenWidth/2 - width/2); ypos = 0; break;
					case 2: xpos = ScreenWidth - width; ypos = 0; break;
					case 3: xpos = 0; ypos = (int)(ScreenHeight/2 - height/2); break;
					case 4: xpos = ScreenWidth - width; ypos = (int)(ScreenHeight/2 - height/2); break;
					case 5: xpos = 0; ypos = ScreenHeight - height; break;
					case 6: xpos = (int)(ScreenWidth/2 - width/2); ypos = ScreenHeight - height; break;
					case 7: xpos = ScreenWidth - width; ypos = ScreenHeight - height; break;
					case 8: break;
				}

				MoveWindow(hwndBBCalendar, xpos, ypos, width, height, true);
			}
				

}
