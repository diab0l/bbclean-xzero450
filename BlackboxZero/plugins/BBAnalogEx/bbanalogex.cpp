/*
============================================================================
Blackbox for Windows: Plugin BBAnalogEx 1.0 by Miroslav Petrasko [Theo]
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

#include "bbanalogex.h"
#include "resource.h"

LPSTR szAppName = "BBAnalogEx";		// The name of our window class, etc.
LPSTR szVersion = "BBAnalogEx v1.5";	// Used in MessageBox titlebars

LPSTR szInfoVersion = "1.5";
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
	//	executeAlarm();
	if(!hSlit) inSlit = false;
	else inSlit = wantInSlit;
	//initialize the plugin before getting style settings
	InitBBAnalogEx();
	GetStyleSettings();
	getCurrentDate();

	// Create the window...
	hwndBBAnalogEx = CreateWindowEx(
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
	if (!hwndBBAnalogEx)
	{
		UnregisterClass(szAppName, hPluginInstance);
		MessageBox(0, "Error creating window", szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);
		return 1;
	}

	//Start the plugin timer
	mySetTimer();
	if(inSlit && hSlit)// Yes, so Let's let BBSlit know.
		SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBAnalogEx);
	else inSlit = false;

	setStatus();	
	// Register to receive Blackbox messages...
	SendMessage(hwndBlackbox, BB_REGISTERMESSAGE, (WPARAM)hwndBBAnalogEx, (LPARAM)msgs);

	const long magicDWord = 0x49474541;
#if !defined _WIN64
	// Set magicDWord to make the window sticky (same magicDWord that is used by LiteStep)...
	SetWindowLong(hwndBBAnalogEx, GWL_USERDATA, magicDWord);
#else
	SetWindowLongPtr(hwndBBAnalogEx, GWLP_USERDATA, magicDWord);
#endif

	// Make the window AlwaysOnTop?
	if(alwaysOnTop) SetWindowPos(hwndBBAnalogEx, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
	// Show the window and force it to update...
	ShowWindow(hwndBBAnalogEx, SW_SHOW);

	InvalidateRect(hwndBBAnalogEx, NULL, true);

	return 0;
}

//===========================================================================
//This function is used once in beginPlugin and in @BBAnalogExReloadSettings def. found in WndProc.
//Do not initialize objects here.  Deal with them in beginPlugin and endPlugin

void InitBBAnalogEx()
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
	KillTimer(hwndBBAnalogEx, IDT_TIMER);
	KillTimer(hwndBBAnalogEx, IDT_ALARMTIMER);
	KillTimer(hwndBBAnalogEx, IDT_MTIMER);
	//shutdown the gdi+ engine
	GdiplusShutdown(g_gdiplusToken);

	//	KillTimer(hwndBBAnalogEx, IDT_ALARMTIMER);
	// Write the current plugin settings to the config file...
	WriteRCSettings();
	// Delete used StyleItems...
	if (myStyleItem) delete myStyleItem;
	if (myStyleItem2) delete myStyleItem2;
	// Delete the main plugin menu if it exists (PLEASE NOTE that this takes care of submenus as well!)
	if (myMenu){ DelMenu(myMenu); myMenu = NULL;}
	// Unregister Blackbox messages...


	SendMessage(hwndBlackbox, BB_UNREGISTERMESSAGE, (WPARAM)hwndBBAnalogEx, (LPARAM)msgs);
	if(inSlit && hSlit)
		SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBAnalogEx);
	// Destroy our window...
	DestroyWindow(hwndBBAnalogEx);
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
				Rectangle(hdc, r.left-1, r.top-1, r.right+1, r.bottom+1);
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


				TransparentBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0xff00ff);
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



				/*	r.left +=3;
				r.right -=3;
				r.bottom -=3;
				r.top += 3;
				*/

				HGDIOBJ otherfont = CreateFont(fontSize, 
					0, 0,  0, FW_NORMAL,
					false, false, false,
					DEFAULT_CHARSET,
					OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS,
					DEFAULT_QUALITY,
					DEFAULT_PITCH,
					fontFace);

				SelectObject(hdc, otherfont);
				SetTextColor(hdc,fontColor);


				Graphics *graphics;


				Pen *pen;
				SolidBrush *br;
				SolidBrush *nr;

				graphics = new Graphics(hdc);
				pen = new Pen(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)),2);
				br = new SolidBrush(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));
				nr = new SolidBrush(Color(160,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));

				if (anti) graphics->SetSmoothingMode(SmoothingModeAntiAlias);

				/*	if (!strcmp(bitmapFile,".none")==0) 
				{	Image *image;

				image->FromFile(bitmapFile);

				graphics->DrawImage(image,0,0,image->GetWidth(),image->GetHeight());

				delete image;
				}*/
				int cntX,cntY,radius;
				double theta;

				cntX = (r.right - r.left)/2 + r.left;
				cntY = (r.bottom - r.top)/2 + r.top;

				radius = ((r.right - r.left)>(r.bottom - r.top)) ? ((r.bottom - r.top)/2 - 2):((r.right - r.left)/2 - 2);
				theta = 0;

				/*			pen->SetColor(Color(255,0,0));
				pen->SetWidth(5);
				graphics->DrawArc(pen, r.left+10,r.top+10,r.right-r.left-20,r.bottom-r.top-20,270,second*360/60);
				pen->SetColor(Color(255,0,255));
				graphics->DrawArc(pen, r.left,r.top,r.right-r.left,r.bottom-r.top,270,hour*360/12);
				pen->SetColor(Color(255,255,0));
				graphics->DrawArc(pen, r.left+5,r.top+5,r.right-r.left-10,r.bottom-r.top-10,270,minute*360/60);


				*/

				//draw small clock
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));

				for (int i=0;i<sccount;i++)
				{
					int sscr,sscx,sscy,sscs,diff;
					COLORREF sscch,ssccm,ssccs;

					sscr = scr[i];
					sscx = scx[i];
					sscy = scy[i];
					sscs = scs[i];
					sscch  = scch[i];
					ssccm  = sccm[i];
					ssccs  = sccs[i];
					diff = scdiff[i];

					if ((div(sscs,100000).rem >= 10000)&&(quadrant>0))
					{
						nr->SetColor(Color(160,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));

						pen->SetColor(Color(255,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));
						sscr = (int)(sscr*0.7);
						graphics->FillPie(nr,sscx-sscr,sscy-sscr,sscr*2,sscr*2,(float)(quadrant-1)*30-90,30);
						graphics->DrawEllipse(pen, (float)(sscx - 2), (float)(sscy - 2), (float)4, (float)4);

						theta = (5 - smal) * (PI / 5);
						graphics->DrawLine(pen, (float)(sscx + (1.5 * cos(theta))),
							(float)(sscy - (1.5 * sin(theta))),
							(float)(sscx + ((sscr * 80 / 100) * cos(theta))),
							(float)(sscy - ((sscr * 80 / 100) * sin(theta))));
					}
					else if (div(sscs,10000).rem >= 1000)
					{
						RECT text;
						text.top = sscy;
						text.left = sscx;
						text.bottom = sscy + sscr*2;
						text.right = sscx + sscr*2;
						_itoa(dday,szTemp,10);
						SetTextColor(hdc, sscch);
						DrawText(hdc,szTemp,-1,&text,DT_VCENTER|DT_CENTER|DT_SINGLELINE);
					}
					else{
						/*	for(theta = 0; theta < 2 * PI; theta = theta + (PI / 6))
						{
						//	if(!(theta == 0 || theta == PI / 2 || theta == PI || theta == (3 * PI) / 2))
						{
						pen->SetWidth(1);
						graphics->DrawLine(pen,
						(float)(sscx + ((sscr - 3) * cos(theta))),
						(float)(sscy - ((sscr - 3) * sin(theta))),
						(float)(sscx + ((sscr - 1) * cos(theta))),
						(float)(sscy - ((sscr - 1) * sin(theta))));
						}
						}
						*/
						pen->SetWidth(2.0);

						if (div(sscs,100).rem >= 10)
						{

							//	if (!acolor) 
							pen->SetColor(Color(255,GetRValue(ssccm), GetGValue(ssccm), GetBValue(ssccm)));

							theta = (15 - minute) * (PI / 30);
							graphics->DrawLine(pen, (float)(sscx + (1.5 * cos(theta))),
								(float)(sscy - (1.5 * sin(theta))),
								(float)(sscx + ((sscr * mlength / 100) * cos(theta))),
								(float)(sscy - ((sscr * mlength / 100) * sin(theta))));

						}

						if (div(sscs,1000).rem >= 100)
						{
							//	pen->SetWidth((float)hwidth);
							//	if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
							pen->SetColor(Color(255,GetRValue(sscch), GetGValue(sscch), GetBValue(sscch)));

							theta = (3 - (hour+diff)-(minute/60.0)) * (PI / 6);
							graphics->DrawLine(pen, (float)(sscx + (1.5 * cos(theta))),
								(float)(sscy - (1.5 * sin(theta))),
								(float)(sscx + ((sscr * hlength / 100) * cos(theta))),
								(float)(sscy - ((sscr * hlength / 100) * sin(theta))));

							graphics->DrawEllipse(pen, (float)(sscx - 2), (float)(sscy - 2), (float)4, (float)4);

						}	

						if (div(sscs,2).rem == 1)
						{
							//	if (showSeconds)
							{
								//	pen->SetWidth((float)swidth);
								//	if (!acolor) 	pen->SetColor(Color(255,GetRValue(scolor), GetGValue(scolor), GetBValue(scolor)));
								pen->SetColor(Color(255,GetRValue(ssccs), GetGValue(ssccs), GetBValue(ssccs)));

								theta = (15 - second) * (PI / 30);
								graphics->DrawLine(pen, (float)(sscx - ((sscr * .25) * cos(theta))),
									(float)(sscy + ((sscr * .25) * sin(theta))),
									(float)(sscx + ((sscr * slength / 100) * cos(theta))),
									(float)(sscy - ((sscr * slength / 100) * sin(theta))));
							}

							//	graphics->DrawEllipse(pen, (float)(sscx + ((sscr * (slength-20) / 100) * cos(theta)) - 1.5), (float)(sscy - ((sscr * (slength-20) / 100) * sin(theta)) - 1.5), (float)3, (float)3);

							pen->SetColor(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));
						}
						//	pen->SetWidth((float)hwidth);
						if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
						graphics->DrawEllipse(pen, (float)(sscx - 2), (float)(sscy - 2), (float)4, (float)4);
						//	pen->SetWidth((float)cwidth);
						if (!acolor) 	pen->SetColor(Color(255,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));
						//	if (drawCircle) graphics->DrawEllipse(pen, (float)r.left, (float)r.top, (float)(r.right - r.left), (float)(r.bottom - r.top));

						//	DrawText(hdc,"XII",-1,&r,DT_TOP|DT_CENTER|DT_SINGLELINE);
						//	DrawText(hdc,"III",-1,&r,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
						//	DrawText(hdc,"IX",-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
						//	DrawText(hdc,"VI",-1,&r,DT_BOTTOM|DT_CENTER|DT_SINGLELINE);
					}

				} //small clocks



				/*
				if (drawMode == 0) {
				for(theta = 0; theta < 2 * PI; theta = theta + (PI / 6))
				{
				if(!(theta == 0 || theta == PI / 2 || theta == PI || theta == (3 * PI) / 2))
				{
				pen->SetWidth(1);
				graphics->DrawLine(pen,
				(float)(cntX + ((radius - 3) * cos(theta))),
				(float)(cntY - ((radius - 3) * sin(theta))),
				(float)(cntX + ((radius - 1) * cos(theta))),
				(float)(cntY - ((radius - 1) * sin(theta))));
				}
				}

				pen->SetWidth((float)mwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(mcolor), GetGValue(mcolor), GetBValue(mcolor)));
				theta = (15 - minute) * (PI / 30);
				graphics->DrawLine(pen, (float)(cntX + (1.5 * cos(theta))),
				(float)(cntY - (1.5 * sin(theta))),
				(float)(cntX + ((radius * mlength / 100) * cos(theta))),
				(float)(cntY - ((radius * mlength / 100) * sin(theta))));


				pen->SetWidth((float)hwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
				theta = ((3 - hour)-(minute/60.0)) * (PI / 6);
				graphics->DrawLine(pen, (float)(cntX + (1.5 * cos(theta))),
				(float)(cntY - (1.5 * sin(theta))),
				(float)(cntX + ((radius * hlength / 100) * cos(theta))),
				(float)(cntY - ((radius * hlength / 100) * sin(theta))));

				if (showSeconds)
				{
				pen->SetWidth((float)swidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(scolor), GetGValue(scolor), GetBValue(scolor)));
				theta = (15 - second) * (PI / 30);
				graphics->DrawLine(pen, (float)(cntX - ((radius * .25) * cos(theta))),
				(float)(cntY + ((radius * .25) * sin(theta))),
				(float)(cntX + ((radius * slength / 100) * cos(theta))),
				(float)(cntY - ((radius * slength / 100) * sin(theta))));

				graphics->DrawEllipse(pen, (float)(cntX + ((radius * (slength-20) / 100) * cos(theta)) - 1.5), (float)(cntY - ((radius * (slength-20) / 100) * sin(theta)) - 1.5), (float)3, (float)3);

				pen->SetColor(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));
				}
				pen->SetWidth((float)hwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
				graphics->DrawEllipse(pen, (float)(cntX - 2), (float)(cntY - 2), (float)4, (float)4);
				pen->SetWidth((float)cwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));
				if (drawCircle) graphics->DrawEllipse(pen, (float)r.left, (float)r.top, (float)(r.right - r.left), (float)(r.bottom - r.top));

				DrawText(hdc,"XII",-1,&r,DT_TOP|DT_CENTER|DT_SINGLELINE);
				DrawText(hdc,"III",-1,&r,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
				DrawText(hdc,"IX",-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
				DrawText(hdc,"VI",-1,&r,DT_BOTTOM|DT_CENTER|DT_SINGLELINE);

				}else if (drawMode == 1){



				pen->SetWidth((float)mwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(mcolor), GetGValue(mcolor), GetBValue(mcolor)));
				theta = (15 - minute) * (PI / 30);
				graphics->DrawLine(pen, (float)(cntX + (radius * hlength/100 * cos(theta))),
				(float)(cntY - (radius * hlength/100 * sin(theta))),
				(float)(cntX + ((radius * mlength/100) * cos(theta))),
				(float)(cntY - ((radius * mlength/100) * sin(theta))));

				if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
				pen->SetWidth((float)hwidth);
				theta = (3 - hour) * (PI / 6);
				graphics->DrawLine(pen, (float)(cntX + (radius *0.025 * cos(theta))),
				(float)(cntY - (radius * 0.025 * sin(theta))),
				(float)(cntX + ((radius * hlength/100) * cos(theta))),
				(float)(cntY - ((radius * hlength/100) * sin(theta))));

				if (showSeconds)
				{
				pen->SetWidth((float)swidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(scolor), GetGValue(scolor), GetBValue(scolor)));
				theta = (15 - second) * (PI / 30);
				graphics->DrawLine(pen, (float)(cntX + ((radius * mlength/100) * cos(theta))),
				(float)(cntY - ((radius * mlength/100) * sin(theta))),
				(float)(cntX + ((radius * slength/100) * cos(theta))),
				(float)(cntY - ((radius * slength/100) * sin(theta))));

				pen->SetColor(Color(255,GetRValue(fontColor), GetGValue(fontColor), GetBValue(fontColor)));


				if (drawCircle){
				pen->SetWidth((float)cwidth);

				if (!acolor) 	pen->SetColor(Color(255,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));
				graphics->DrawEllipse(pen, (float)(cntX - (radius * mlength/100)), (float)(cntY - (radius * mlength/100)), (float)(radius*mlength/50), (float)(radius*mlength/50));
				graphics->DrawEllipse(pen, (float)(cntX - (radius * hlength/100)), (float)(cntY - (radius * hlength/100)), (float)(radius*hlength/50), (float)(radius*hlength/50));
				graphics->DrawEllipse(pen, (float)(cntX - (radius * slength/100)), (float)(cntY - (radius * slength/100)), (float)(radius*slength/50), (float)(radius*slength/50));
				}
				}
				pen->SetWidth((float)hwidth);
				if (!acolor) 	pen->SetColor(Color(255,GetRValue(hcolor), GetGValue(hcolor), GetBValue(hcolor)));
				graphics->DrawEllipse(pen, (float)(cntX - 2), (float)(cntY - 2), (float)4, (float)4);

				}
				*/

				if (quad>0) {
					nr->SetColor(Color(160,GetRValue(ccolor), GetGValue(ccolor), GetBValue(ccolor)));

					graphics->FillPie(nr,0,0,width,height,(float)(hquad-1)*30-90,30);

				}

				delete nr;
				delete br;
				delete pen;
				delete graphics;



				DeleteObject(otherfont);

				if (!strcmp(overBitmapFile,".none")==0) 
				{

					HANDLE image;
					image = LoadImage(NULL, overBitmapFile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

					HDC hdcMem = CreateCompatibleDC(hdc);

					HBITMAP old = (HBITMAP) SelectObject(hdcMem, image);
					BITMAP bitmap;
					GetObject(image, sizeof(BITMAP), &bitmap);


					TransparentBlt(hdc, r.left, r.top, r.right - r.left, r.bottom - r.top, hdcMem, 0, 0, bitmap.bmWidth, bitmap.bmHeight, 0x000000);
					SelectObject(hdcMem, old);
					DeleteObject(old);
					DeleteObject(image);
					DeleteDC(hdcMem);

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
			InvalidateRect(hwndBBAnalogEx, NULL, true);
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
				ShowWindow( hwndBBAnalogEx, SW_SHOW);
				InvalidateRect( hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBHidePlugins") &&  pluginToggle && !inSlit)
			{
				// Hide window...
				ShowWindow( hwndBBAnalogEx, SW_HIDE);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExAbout"))
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
			else if (!_stricmp(szTemp, "@BBAnalogExPluginToggle"))
			{
				// Hide window...
				pluginToggle = !pluginToggle;
			}
			else if (!_stricmp(szTemp, "@BBAnalogExOnTop"))
			{
				// Always on top...
				alwaysOnTop = !alwaysOnTop;

				if (alwaysOnTop && !inSlit) SetWindowPos(hwndBBAnalogEx, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
				else SetWindowPos(hwndBBAnalogEx, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExSlit"))
			{
				// Does user want it in the slit...
				wantInSlit = !wantInSlit;

				inSlit = wantInSlit;
				if(wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBAnalogEx);
				else if(!wantInSlit && hSlit)
					SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBAnalogEx);
				else
					inSlit = false;	

				setStatus();

				GetStyleSettings();
				//update window
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExTransparent"))
			{
				// Set the transparent attributes to the window
				transparency = !transparency;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBAnalogExFullTrans"))
			{
				// Set the transparent bacground attribut to the window
				fullTrans = !fullTrans;
				setStatus();
			}

			else if (!_stricmp(szTemp, "@BBAnalogExSnapToEdge"))
			{
				// Set the snapWindow attributes to the window
				snapWindow = !snapWindow;
			}
			else if (!_stricmp(szTemp, "@BBAnalogExDrawBorder"))
			{
				drawBorder = !drawBorder;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExAnti"))
			{
				anti = !anti;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExDrawCircle"))
			{
				drawCircle = !drawCircle;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExEnableAlarms"))
			{
				alarms = !alarms;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExAColor"))
			{
				acolor = !acolor;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExShowSeconds"))
			{
				showSeconds = !showSeconds;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExNoBitmap"))
			{
				//	noBitmap = true;
				strcpy(bitmapFile, ".none");

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExNoOBitmap"))
			{
				//	noBitmap = true;
				strcpy(overBitmapFile, ".none");

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleLabel"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "label");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleToolbar"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "toolbar");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleButton"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonnp");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleButtonPr"))
			{
				// Set the label attributes to the window style
				strcpy(windowStyle, "buttonpr");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleWindowLabel"))
			{
				// Set the windowLabel attributes to the window style
				strcpy(windowStyle, "windowlabel");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExStyleClock"))
			{
				// Set the clock attributes to the window style
				strcpy(windowStyle, "clock");
				GetStyleSettings();
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExEditRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, rcpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExEditAlarmsRC"))
			{
				BBExecute(GetDesktopWindow(), NULL, alarmpath, NULL, NULL, SW_SHOWNORMAL, false);
			}
			else if (!_stricmp(szTemp, "@BBAnalogExReloadSettings"))
			{

				{
					//remove from slit before resetting window attributes
					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_REMOVE, NULL, (LPARAM)hwndBBAnalogEx);

					//Re-initialize
					ReadRCSettings();
					InitBBAnalogEx();
					inSlit = wantInSlit;
					GetStyleSettings();

					setStatus();

					//set window on top is alwaysontop: is true
					if ( alwaysOnTop) SetWindowPos( hwndBBAnalogEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
					else SetWindowPos( hwndBBAnalogEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

					if(inSlit && hSlit)
						SendMessage(hSlit, SLIT_ADD, NULL, (LPARAM)hwndBBAnalogEx);
					else inSlit = false;

					//update window
					InvalidateRect(hwndBBAnalogEx, NULL, true);
				}
			}
			else if (!_stricmp(szTemp, "@BBAnalogExSaveSettings"))
			{
				WriteRCSettings();
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExMode", 14))
			{
				drawMode = atoi(szTemp + 15);

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExFontSize", 19))
			{
				fontSize = atoi(szTemp + 20);

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExSColor", 16))
			{
				//	strcpy(szTemp,);
				scolor = strtol(szTemp + 19, NULL, 16);
				scolor =  PALETTERGB(GetBValue(scolor),GetGValue(scolor),GetRValue(scolor));
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExMColor", 16))
			{
				//	strcpy(szTemp,);
				mcolor = strtol(szTemp + 19, NULL, 16);
				mcolor =  PALETTERGB(GetBValue(mcolor),GetGValue(mcolor),GetRValue(mcolor));

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExHColor", 16))
			{
				//	strcpy(szTemp,);
				hcolor = strtol(szTemp + 19, NULL, 16);
				hcolor =  PALETTERGB(GetBValue(hcolor),GetGValue(hcolor),GetRValue(hcolor));
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExCColor", 16))
			{
				//	strcpy(szTemp,);

				ccolor = strtol(szTemp + 19, NULL, 16);
				ccolor =  PALETTERGB(GetBValue(ccolor),GetGValue(ccolor),GetRValue(ccolor));

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}

			else if (!_strnicmp(szTemp, "@BBAnalogExWidth", 16))
			{ //changing the clock size
				width = atoi(szTemp + 17);

				if ( alwaysOnTop) SetWindowPos( hwndBBAnalogEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBAnalogEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExHeight", 17))
			{ //changing the clock size
				height = atoi(szTemp + 18);

				if ( alwaysOnTop) SetWindowPos( hwndBBAnalogEx, HWND_TOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);
				else SetWindowPos( hwndBBAnalogEx, HWND_NOTOPMOST, xpos, ypos, width, height, SWP_NOACTIVATE);

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExSetTransparent", 24))
			{
				alpha = atoi(szTemp + 25);

				if (transparency) setStatus();					
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExSLength", 17))
			{
				slength = atoi(szTemp + 18);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExMLength", 17))
			{
				mlength = atoi(szTemp + 18);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExHLength", 17))
			{
				hlength = atoi(szTemp + 18);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExSwidth", 16))
			{
				swidth = atoi(szTemp + 17);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExMwidth", 16))
			{
				mwidth = atoi(szTemp + 17);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExHwidth", 16))
			{
				hwidth = atoi(szTemp + 17);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExCwidth", 16))
			{
				cwidth = atoi(szTemp + 17);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}

			else if (!_strnicmp(szTemp, "@BBAnalogExNC", 13))
			{
				sccount = atoi(szTemp + 13);

				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}

			else if (!_strnicmp(szTemp, "@BBAnalogExscx", 14))
			{
				char num[3];

				strcpy(szTemp,szTemp + 14);
				strncpy(num,szTemp,2);
				num[2]=0;
				strcpy(szTemp,szTemp + 2);
				scx[atoi(num)] = atoi(szTemp);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}

			else if (!_strnicmp(szTemp, "@BBAnalogExscy", 14))
			{
				char num[3];

				strcpy(szTemp,szTemp + 14);
				strncpy(num,szTemp,2);
				num[2]=0;
				strcpy(szTemp,szTemp + 2);
				scy[atoi(num)] = atoi(szTemp);
				//scy[0] = atoi(szTemp + 14);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExscradius", 19))
			{
				char num[3] = {0, 0, 0};

				strcpy(szTemp,szTemp + 19);
				strncpy(num,szTemp,2);
				
				strcpy(szTemp,szTemp + 2);
				scr[atoi(num)] = atoi(szTemp);
				//scy[0] = atoi(szTemp + 14);

				//	scr[0] = atoi(szTemp + 19);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExscdiff", 17))
			{
				char num[3] = {0, 0, 0};

				strncpy(num, szTemp+17, 2);

				//strcpy(szTemp,szTemp + 17);
				//strncpy(num,szTemp,2);

				//strcpy(szTemp,szTemp + 2);
				scdiff[atoi(num)] = atoi(szTemp+2);
				//scy[0] = atoi(szTemp + 14);

				//	scr[0] = atoi(szTemp + 19);
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}

			else if (!_strnicmp(szTemp, "@BBAnalogExSetBitmap", 20))
			{

				strcpy(bitmapFile,szTemp + 22);
				noBitmap = false;
				InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			else if (!_strnicmp(szTemp, "@BBAnalogExMessage", 18))
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
			else if (!_strnicmp(szTemp, "@BBAnalogExPlayWav", 18))
			{
				strcpy(szTemp,szTemp + 20);
				PlaySound(szTemp, NULL, SND_FILENAME | SND_ASYNC);

			}
			/*	else if (!_strnicmp(szTemp, "@BBAnalogExDatePossition", 24))
			{
			text_pos = atoi(szTemp + 25);

			InvalidateRect(hwndBBAnalogEx, NULL, true);
			}
			*/	else if (!_stricmp(szTemp, "@BBAnalogExLoadBitmap"))
			{

				OPENFILENAME ofn;       // common dialog box structure

				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndBBAnalogEx;
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
				if (GetOpenFileName(&ofn)) 

					InvalidateRect(hwndBBAnalogEx, NULL, true);

			}
			else if (!_stricmp(szTemp, "@BBAnalogExLoadOBitmap"))
			{

				OPENFILENAME ofn;       // common dialog box structure

				// Initialize OPENFILENAME
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = hwndBBAnalogEx;
				ofn.lpstrFile = overBitmapFile;
				ofn.nMaxFile = sizeof(overBitmapFile);
				ofn.nFilterIndex = 1;
				ofn.lpstrFileTitle = NULL;
				ofn.nMaxFileTitle = 0;
				ofn.lpstrFilter = "Bitmaps (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
				//			ofn.lpstrInitialDir = defaultpath;
				//			ofn.lpstrTitle = title;
				//			ofn.lpstrDefExt = defaultextension;	

				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				if (GetOpenFileName(&ofn)) 

					InvalidateRect(hwndBBAnalogEx, NULL, true);

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
				MoveWindow(hwndBBAnalogEx, xpos, ypos, width, height, true);
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
		//	InvalidateRect(hwndBBAnalogEx, NULL, false);
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
			if (quadrant > 0)
			{quadrant = 0;}
			else{
				//open control panel with:  control timedate.cpl,system,0
				//BBExecute(GetDesktopWindow(), NULL, "control", "timedate.cpl,system,0", NULL, SW_SHOWNORMAL, false);
				quad=2;
				POINT poss;
				RECT windowRect;
				GetCursorPos(&poss);
				GetWindowRect(hwndBBAnalogEx,&windowRect);
				poss.x -= windowRect.left;
				poss.y -= windowRect.top;


				//	POINT poss;
				//	GetCursorPos(&poss);
				double alpha;
				int sx,sy;
				double pom;

				sx = width/2;
				sy = height/2;


				pom = sqrt((double)(((poss.x-sx)*(poss.x-sx))+((poss.y-sy)*(poss.y-sy))));

				alpha = acos(abs(poss.y-sy)/pom);


				quadrant = (int)(alpha/(PI/6))+1; 


				if ((poss.x>sx) && (poss.y>sy)) quadrant = abs(quadrant-4)+3;
				if ((poss.x<sx) && (poss.y>sy)) quadrant = quadrant+6;
				if ((poss.x<sx) && (poss.y<sy)) quadrant = abs(quadrant-4)+9;
				hquad = quadrant;
				//itoa((quadrant),szTemp,10);
				//MessageBox(hwndBlackbox, szTemp, szAppName, MB_OK | MB_ICONERROR | MB_TOPMOST);

				SetTimer(hwndBBAnalogEx,		// handle to main window 
					IDT_ALARMTIMER,			// timer identifier 
					2000,				// second interval 
					(TIMERPROC) NULL);	// no timer callback 

				SetTimer(hwndBBAnalogEx,		// handle to main window 
					IDT_MTIMER,			// timer identifier 
					100,				// second interval 
					(TIMERPROC) NULL);	// no timer callback 
			}
			InvalidateRect(hwndBBAnalogEx, NULL, true);


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
					//	show = !show;
					if (quad>0) quad--; 
					getCurrentDate();
					InvalidateRect(hwndBBAnalogEx, NULL, false);
					//	if ((second == 0)&&(show)&&(alarms)) executeAlarm();

				} break;

			case IDT_ALARMTIMER:
				{
					//redraw the window
					//show = !show;
					quadrant = quadrant - 1;
					//	PlaySound("d:/electronicping.wav", NULL, SND_FILENAME | SND_ASYNC);
					if (quadrant == 0){ KillTimer(hwndBBAnalogEx, IDT_ALARMTIMER);
					PlaySound("d:/electronicping.wav", NULL, SND_FILENAME | SND_ASYNC);
					}
					InvalidateRect(hwndBBAnalogEx, NULL, false);
				} break; 

			case IDT_MTIMER:
				{
					//redraw the window
					//show = !show;
					smal = smal - 1;
					if (smal == 0) smal=10; 
					if (quadrant == 0) KillTimer(hwndBBAnalogEx, IDT_ALARMTIMER);
					InvalidateRect(hwndBBAnalogEx, NULL, false);
				} break; 
			}
		}
		break;

		/*	case WM_MOUSEMOVE:
		{
		InvalidateRect(hwndBBAnalogEx, NULL, false);
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

	strcat(temp, "bbanalogex.rc");
	strcat(path, "bbanalogexrc");
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

		strcat(temp, "bbanalogex.rc");
		strcat(path, "bbanalogexrc");
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
			strcpy(overBitmapFile, ".none");
			swidth = mwidth = hwidth = cwidth = 2;
			mlength = 70;
			hlength = 50;
			slength = 90;
			//			strcpy(clockformat, "%d %a %#H:%M");
			noBitmap = true;
			drawCircle = true;
			drawMode = 0;
			scolor = mcolor = hcolor = ccolor = 0xffffff;
			acolor = true;
			//			magStart = false;
			//	scx[0]=scy[0]=scr[0]=10;

			WriteRCSettings();

			return;
		}
	}
	// If a config file was found we read the plugin settings from the file...
	//Always checking non-bool values to make sure they are the right format
	xpos = ReadInt(rcpath, "bbanalogex.x:", 10);
	ypos = ReadInt(rcpath, "bbanalogex.y:", 10);

	width = ReadInt(rcpath, "bbanalogex.width:", 100);
	height = ReadInt(rcpath, "bbanalogex.height:", 50);

	//	text_pos = ReadInt(rcpath, "bbanalogex.date.possition:", 0);
	//	if ((text_pos<0) ||(text_pos>4)) text_pos = 0; 

	alpha = ReadInt(rcpath, "bbanalogex.alpha:", 160);
	if(alpha > 255) alpha = 255;

	if(ReadString(rcpath, "bbanalogex.inSlit:", NULL) == NULL) wantInSlit = true;
	else wantInSlit = ReadBool(rcpath, "bbanalogex.inSlit:", true);

	alwaysOnTop = ReadBool(rcpath, "bbanalogex.alwaysOnTop:", true);

	drawBorder = ReadBool(rcpath, "bbanalogex.drawBorder:", true);
	drawMode = ReadInt(rcpath, "bbanalogex.drawMode:", 0);
	drawCircle = ReadBool(rcpath, "bbanalogex.draw.circle:", true);
	anti = ReadBool(rcpath, "bbanalogex.anti.alias:", false);
	showSeconds = ReadBool(rcpath, "bbanalogex.show.seconds:", true);
	alarms = ReadBool(rcpath, "bbanalogex.enabled.alarms:", false);
	//	if (magStart) mag = true;

	snapWindow = ReadBool(rcpath, "bbanalogex.snapWindow:", true);
	transparency = ReadBool(rcpath, "bbanalogex.transparency:", false);
	fullTrans = ReadBool(rcpath, "bbanalogex.fullTrans:", false);
	fontSize = ReadInt(rcpath, "bbanalogex.fontSize:", 6);
	alwaysOnTop = ReadBool(rcpath, "bbanalogex.alwaysontop:", true);
	pluginToggle = ReadBool(rcpath, "bbanalogex.pluginToggle:", false);
	strcpy(windowStyle, ReadString(rcpath, "bbanalogex.windowStyle:", "windowlabel"));
	if(((StrStrI(windowStyle, "label") == NULL) || ((StrStrI(windowStyle, "label") != NULL) && (strlen(windowStyle) > 5))) 
		&& (StrStrI(windowStyle, "windowlabel") == NULL) && (StrStrI(windowStyle, "clock") == NULL)  && (StrStrI(windowStyle, "button") == NULL)  && (StrStrI(windowStyle, "buttonpr") == NULL)  && (StrStrI(windowStyle, "toolbar") == NULL)) 
		strcpy(windowStyle, "windowLabel");

	strcpy(bitmapFile, ReadString(rcpath, "bbanalogex.bitmapFile:", ".none"));
	//	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 

	strcpy(overBitmapFile, ReadString(rcpath, "bbanalogex.overBitmapFile:", ".none"));
	//	if (strcmp(bitmapFile,".none")==0) noBitmap = true; else noBitmap = false; 

	int widths;
	widths = ReadInt(rcpath, "bbanalogex.hands.width:", 1111);
	hwidth = div(widths,1000).quot;
	widths -= hwidth*1000;
	mwidth = div(widths,100).quot;
	widths -= mwidth*100;
	swidth = div(widths,10).quot;
	cwidth = div(widths,10).rem;
	widths = ReadInt(rcpath, "bbanalogex.hands.lenght:", 507090);
	hlength = div(widths,10000).quot;
	widths -= hlength*10000;
	mlength = div(widths,100).quot;
	slength = div(widths,100).rem;
	scolor = ReadColor(rcpath, "bbanalogex.second.color:","0xffffff");
	mcolor = ReadColor(rcpath, "bbanalogex.minute.color:","0xffffff");
	hcolor = ReadColor(rcpath, "bbanalogex.hour.color:","0xffffff");
	ccolor = ReadColor(rcpath, "bbanalogex.circle.color:","0xffffff");
	acolor = ReadBool(rcpath, "bbanalogex.auto.colors:", true);

	sccount = ReadInt(rcpath, "bbanalogex.sc.count:", 0);

	for (int i=0;i<sccount;i++)
	{
		sprintf(szTemp, "bbanalogex.sc%d.x:", i, temp);
		scx[i] = ReadInt(rcpath, szTemp, 50);
		sprintf(szTemp, "bbanalogex.sc%d.y:", i, temp);
		scy[i] = ReadInt(rcpath, szTemp, 50);
		sprintf(szTemp, "bbanalogex.sc%d.r:", i, temp);
		scr[i] = ReadInt(rcpath, szTemp, 50);
		sprintf(szTemp, "bbanalogex.sc%d.d:", i, temp);
		scdiff[i] = ReadInt(rcpath, szTemp, 50);
		sprintf(szTemp, "bbanalogex.sc%d.s:", i, temp);
		scs[i] = ReadInt(rcpath, szTemp, 111);
		sprintf(szTemp, "bbanalogex.sc%d.ch:", i, temp);
		scch[i] = ReadColor(rcpath, szTemp,"0xffffff");
		sprintf(szTemp, "bbanalogex.sc%d.cm:", i, temp);
		sccm[i] = ReadColor(rcpath, szTemp,"0xffffff");
		sprintf(szTemp, "bbanalogex.sc%d.cs:", i, temp);
		sccs[i] = ReadColor(rcpath, szTemp,"0xffffff");
	}

	//	strcpy(clockformat, ReadString(rcpath, "bbanalogex.clockformat:", "%d %a %#H:%M"));

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

		sprintf(szTemp, "! BBAnalogEx %s alarms file.\r\n",szInfoVersion);
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

		sprintf(szTemp, "! BBAnalogEx %s config file.\r\n",szInfoVersion);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "!============================\r\n\r\n");
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.x: %d\r\n", xpos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.y: %d\r\n", ypos, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.width: %d\r\n", width, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.height: %d\r\n", height, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		//		sprintf(szTemp, "bbanalogex.date.possition: %d\r\n", text_pos, temp);
		//	WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.windowStyle: %s\r\n", windowStyle);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(wantInSlit) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.inSlit: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alwaysOnTop) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.alwaysOnTop: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(snapWindow) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.snapWindow: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(transparency) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.transparency: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.alpha: %d\r\n", alpha, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(fullTrans) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.fullTrans: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(pluginToggle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.pluginToggle: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawBorder) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.drawBorder: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.drawMode: %d\r\n", drawMode, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(anti) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.anti.alias: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(showSeconds) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.show.seconds: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(drawCircle) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.draw.circle: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(alarms) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.enabled.alarms: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		(acolor) ? strcpy(temp, "true") : strcpy(temp, "false");
		sprintf(szTemp, "bbanalogex.auto.colors: %s\r\n", temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.second.color: #%.2x%.2x%.2x\r\n", GetRValue(scolor),GetGValue(scolor),GetBValue(scolor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.minute.color: #%.2x%.2x%.2x\r\n", GetRValue(mcolor),GetGValue(mcolor),GetBValue(mcolor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.hour.color: #%.2x%.2x%.2x\r\n", GetRValue(hcolor),GetGValue(hcolor),GetBValue(hcolor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.circle.color: #%.2x%.2x%.2x\r\n", GetRValue(ccolor),GetGValue(ccolor),GetBValue(ccolor));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);


		sprintf(szTemp, "bbanalogex.fontSize: %d\r\n", fontSize, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.hands.width: %d\r\n", (hwidth*1000+mwidth*100+swidth*10+cwidth));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.hands.length: %d\r\n", (hlength*10000+mlength*100+slength));
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.bitmapFile: %s\r\n", bitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.overBitmapFile: %s\r\n", overBitmapFile);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		sprintf(szTemp, "bbanalogex.sc.count: %d\r\n", sccount, temp);
		WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

		for (int i=0;i<sccount;i++)
		{
			sprintf(szTemp, "bbanalogex.sc%d.x: %d\r\n", i,scx[i], temp);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.y: %d\r\n", i,scy[i], temp);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.r: %d\r\n", i,scr[i], temp);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.d: %d\r\n", i,scdiff[i], temp);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.s: %d\r\n", i,scs[i], temp);
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.ch: #%.2x%.2x%.2x\r\n",i, GetRValue(scch[i]),GetGValue(scch[i]),GetBValue(scch[i]));
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.cm: #%.2x%.2x%.2x\r\n",i, GetRValue(sccm[i]),GetGValue(sccm[i]),GetBValue(sccm[i]));
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);

			sprintf(szTemp, "bbanalogex.sc%d.cs: #%.2x%.2x%.2x\r\n",i, GetRValue(sccs[i]),GetGValue(sccs[i]),GetBValue(sccs[i]));
			WriteFile(file, szTemp, strlen(szTemp), &retLength, NULL);
		}


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
	SetTimer(hwndBBAnalogEx,		// handle to main window 
		IDT_TIMER,			// timer identifier 
		1000,				// second interval 
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
				SetWindowLong(hwndBBAnalogEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
				BBSetLayeredWindowAttributes(hwndBBAnalogEx, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY|LWA_ALPHA);
			}
			else
			{
				SetWindowLong(hwndBBAnalogEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
				BBSetLayeredWindowAttributes(hwndBBAnalogEx, NULL, (unsigned char)alpha, LWA_ALPHA);
			}
		}
		else if ((!transparency) && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
		{
			if (fullTrans && (dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
			{
				SetWindowLong(hwndBBAnalogEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);
				BBSetLayeredWindowAttributes(hwndBBAnalogEx, 0xFF00FF, (unsigned char)alpha, LWA_COLORKEY);
			}
			else
				SetWindowLong(hwndBBAnalogEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		}

	}
	else if((transparency)||(fullTrans)) SetWindowLong(hwndBBAnalogEx, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	InvalidateRect(hwndBBAnalogEx, NULL, false);		
}


void getCurrentDate()
{

	time(&systemTime);
	localTime = localtime(&systemTime);
	second	= localTime->tm_sec;
	minute	= localTime->tm_min;
	hour		= localTime->tm_hour;

	dday = localTime->tm_mday;
	if(hour > 12) hour -= 12;
	if(hour == 0) hour = 12;

}


void executeAlarm()
{
	sprintf(htime,"%.2d.%.2d:",rhour,minute);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
		SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);


	sprintf(htime,"%.2d.NN:",rhour);

	strcpy(alarm, ReadString(alarmpath, htime, "n"));

	if (alarm[0] == '@')
		SendMessage(hwndBlackbox, BB_BROADCAST, 0, (LPARAM)alarm);

}

void createMenu()
{
	//	bool tempBool = false;

	if(myMenu){ DelMenu(myMenu); myMenu = NULL;}

	//Now we define all menus and submenus

	otherSubmenu = MakeMenu("Other");

	MakeMenuItem(otherSubmenu, "Draw Border", "@BBAnalogExDrawBorder", drawBorder);
	MakeMenuItem(otherSubmenu, "Draw Circle", "@BBAnalogExDrawCircle", drawCircle);
	MakeMenuItemInt(otherSubmenu, "Circle Width", "@BBAnalogExCwidth", cwidth,1,9);
	MakeMenuItem(otherSubmenu, "Show Seconds", "@BBAnalogExShowSeconds", showSeconds);
	MakeMenuItem(otherSubmenu, "Use Anti-Aliash", "@BBAnalogExAnti", anti);
	//	MakeMenuItem(otherSubmenu, "Remove First 0", "@BBAnalogExRemove0", remove_zero);
	MakeMenuItem(otherSubmenu, "Enable Alarms", "@BBAnalogExEnableAlarms", alarms);
	MakeMenuItemInt(otherSubmenu, "Width", "@BBAnalogExWidth", width, 20, 400);
	MakeMenuItemInt(otherSubmenu, "Height", "@BBAnalogExHeight", height, 20, 400);
	MakeMenuItemInt(otherSubmenu, "Font Size", "@BBAnalogExFontSize", fontSize, 6, 50);
	//	MakeMenuItemString(otherSubmenu, "Text Format", "@BBAnalogExClockFormat", clockformat);

	colorSubmenu = MakeMenu("Colors");
	MakeMenuItem(colorSubmenu, "Auto", "@BBAnalogExAColor", acolor);
	sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(scolor),GetGValue(scolor),GetBValue(scolor));
	MakeMenuItemString(colorSubmenu, "Second Color", "@BBAnalogExSColor", szTemp);
	sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(mcolor),GetGValue(mcolor),GetBValue(mcolor));
	MakeMenuItemString(colorSubmenu, "Minute Color", "@BBAnalogExMColor", szTemp);
	sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(hcolor),GetGValue(hcolor),GetBValue(hcolor));
	MakeMenuItemString(colorSubmenu, "Hour Color", "@BBAnalogExHColor", szTemp);
	sprintf(szTemp, "#%.2x%.2x%.2x", GetRValue(ccolor),GetGValue(ccolor),GetBValue(ccolor));
	MakeMenuItemString(colorSubmenu, "Circle Color", "@BBAnalogExCColor", szTemp);




	windowStyleSubmenu = MakeMenu("Style");
	MakeMenuItem(windowStyleSubmenu, "toolbar:", "@BBAnalogExStyleToolbar", (StrStrI(windowStyle, "toolbar") != NULL));
	MakeMenuItem(windowStyleSubmenu, "toolbar.button:", "@BBAnalogExStyleButton", (StrStrI(windowStyle, "buttonnp") != NULL));
	MakeMenuItem(windowStyleSubmenu, "toolbar.button.pressed:", "@BBAnalogExStyleButtonPr", (StrStrI(windowStyle, "buttonpr") != NULL));
	MakeMenuItem(windowStyleSubmenu, "toolbar.label:", "@BBAnalogExStyleLabel", (StrStrI(windowStyle, "label") != NULL && strlen(windowStyle) < 6));
	MakeMenuItem(windowStyleSubmenu, "toolbar.windowLabel:", "@BBAnalogExStyleWindowLabel", (StrStrI(windowStyle, "windowlabel") != NULL));
	MakeMenuItem(windowStyleSubmenu, "toolbar.clock:", "@BBAnalogExStyleClock", (StrStrI(windowStyle, "clock") != NULL));

	configSubmenu = MakeMenu("Configuration");


	generalConfigSubmenu = MakeMenu("General");
	if(hSlit) MakeMenuItem(generalConfigSubmenu, "In Slit", "@BBAnalogExSlit", wantInSlit);
	MakeMenuItem(generalConfigSubmenu, "Toggle with Plugins", "@BBAnalogExPluginToggle", pluginToggle);
	MakeMenuItem(generalConfigSubmenu, "Always on top", "@BBAnalogExOnTop", alwaysOnTop);
	if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
		MakeMenuItem(generalConfigSubmenu, "Transparency", "@BBAnalogExTransparent", transparency);
	if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
		MakeMenuItemInt(generalConfigSubmenu, "Set Transparency", "@BBAnalogExSetTransparent",alpha,0,255);
	if ((dwId == VER_PLATFORM_WIN32_NT)&&(dwMajorVer > 4))
		MakeMenuItem(generalConfigSubmenu, "Transparent Background", "@BBAnalogExFullTrans", fullTrans);
	MakeMenuItem(generalConfigSubmenu, "Snap Window To Edge", "@BBAnalogExSnapToEdge", snapWindow);

	//	browseSubmenu = MakeMenu("Browse");

	bitmapSubmenu = MakeMenu("Bitmap");
	//			MakeSubmenu(bitmapSubmenu, browseSubmenu, "Bitmap");
	MakeMenuItem(bitmapSubmenu, "Browse...", "@BBAnalogExLoadBitmap", false);
	MakeMenuItem(bitmapSubmenu, "Nothing", "@BBAnalogExNoBitmap", (strcmp(bitmapFile,".none")==0));


	//	MakeSubmenu(bitmapSubmenu, browseSubmenu, "OverlayBitmap");
	MakeMenuItem(bitmapSubmenu, "Browse...", "@BBAnalogExLoadOBitmap", false);
	MakeMenuItem(bitmapSubmenu, "Nothing", "@BBAnalogExNoOBitmap", (strcmp(overBitmapFile,".none")==0));

	modeSubmenu = MakeMenu("Draw Mode");
	//	tempBool = false;
	//	if (drawMode == 0) tempBool = true;
	MakeMenuItem(modeSubmenu, "Classic", "@BBAnalogExMode 0", (drawMode == 0));
	MakeMenuItem(modeSubmenu, "Circles", "@BBAnalogExMode 1", (drawMode == 1));
	//	MakeMenuItem(modeSubmenu, "Just Text", "@BBAnalogExMode 2", (drawMode == 2));

	settingsSubmenu = MakeMenu("Settings");
	MakeMenuItem(settingsSubmenu, "Edit Alarms", "@BBAnalogExEditAlarmsRC", false);
	MakeMenuItem(settingsSubmenu, "Edit Settings", "@BBAnalogExEditRC", false);
	MakeMenuItem(settingsSubmenu, "Reload Settings", "@BBAnalogExReloadSettings", false);
	MakeMenuItem(settingsSubmenu, "Save Settings", "@BBAnalogExSaveSettings", false);

	handSubmenu = MakeMenu("Hands");
	MakeMenuItemInt(handSubmenu, "Second Width", "@BBAnalogExSwidth", swidth, 1,9);
	MakeMenuItemInt(handSubmenu, "Second Length %", "@BBAnalogExSLength", slength, 1,99);
	MakeMenuItemInt(handSubmenu, "Minute Width", "@BBAnalogExMwidth",mwidth,1,9);
	MakeMenuItemInt(handSubmenu, "Minute Length %", "@BBAnalogExMLength",mlength,1,99);
	MakeMenuItemInt(handSubmenu, "Hour Width", "@BBAnalogExHwidth", hwidth,1,9);
	MakeMenuItemInt(handSubmenu, "Hour Length %", "@BBAnalogExHLength", hlength,1,99);
	static char temp[8];
	smallclockSubmenu = MakeMenu("Clocks");
	MakeMenuItemInt(smallclockSubmenu, "# Clocks", "@BBAnalogExNC", sccount, 1,10);

	for (int i=0;i<sccount;i++)
	{
		sprintf(szTemp, "@BBAnalogExscx %d", i, temp);
		MakeMenuItemInt(smallclockSubmenu, "X Possition", szTemp, scx[i], 1,width);
		sprintf(szTemp, "@BBAnalogExscy %d", i, temp);
		MakeMenuItemInt(smallclockSubmenu, "Y Possition", szTemp, scy[i], 1,height);
		sprintf(szTemp, "@BBAnalogExscradius %d", i, temp);
		MakeMenuItemInt(smallclockSubmenu, "Radius", szTemp, scr[i], 1,width/2);
		sprintf(szTemp, "@BBAnalogExscdiff %d", i, temp);
		MakeMenuItemInt(smallclockSubmenu, "Difference", szTemp, scdiff[i], 0,23);
	}
	//attach defined menus together
	myMenu = MakeMenu("BBAnalogEx 1.5");

	MakeSubmenu(configSubmenu, modeSubmenu, "Draw Mode");
	MakeSubmenu(configSubmenu, handSubmenu, "Hands");
	MakeSubmenu(configSubmenu, colorSubmenu, "Colors");
	MakeSubmenu(configSubmenu, windowStyleSubmenu, "Style");
	MakeSubmenu(configSubmenu, generalConfigSubmenu, "General");
	MakeSubmenu(configSubmenu, otherSubmenu, "Other");
	MakeSubmenu(configSubmenu, bitmapSubmenu, "Image");
	MakeSubmenu(configSubmenu, smallclockSubmenu, "Clocks");

	MakeSubmenu(myMenu, configSubmenu, "Configuration");
	MakeSubmenu(myMenu, settingsSubmenu, "Settings");
	MakeMenuItem(myMenu, "About", "@BBAnalogExAbout", false);
	ShowMenu(myMenu);
}


// the end ....
