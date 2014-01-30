-------------------------------------------------------------------------
-									-
-  ******   ******   *****    *         *             *  *******        -
-  *     *  *     *  *    *                *          *  *              - 
-  *     *  *     *  *     *               *          *  *              -
-  *     *  *     *  *     *  *   ** *  * ***   ***   *  *        *   * -
-  ******   ******   *     *  *  *  **  *  *   *   *  *  *******   * *  -  
-  *     *  *     *  *     *  *  *   *  *  *       *  *  *         * *  -  
-  *     *  *     *  *     *  *  *   *  *  *    ****  *  *          *   -  
-  *     *  *     *  *     *  *  *   *  *  *   *   *  *  *         * *  -  
-  *     *  *     *  *    *   *  *  **  *  *   *  **  *  *         * *  -  
-  ******   ******   *****    *   ** *  *  **   ** *  *  *******  *   * -  
-                                    *                                  -  
-                                *   *                                  -  
-                                 ***                                   -  
-                                                                       -   
-                         BBDigitalEx 1.0 beta 7                        -
-------------------------------------------------------------------------
									
I decided to split bbanalogex into two plugins containing the analog
and the digital clock. this is the first beta of the digital clock...			
				
-------------------------------------------------------------------------

known issues:

 - there are problems under win9x.. but under winxp it works fine

-------------------------------------------------------------------------

controls:

 - ctrl + left click & drag: move
 - ctrl + right click: open menu
 - left click - opens the windows clock dialog 

-------------------------------------------------------------------------

some comments:

 - not everithing works perfect right now but maybe later.... :-)
 - one draw mode is for clocks with bigger width the other with bigger 
   height

-------------------------------------------------------------------------   

something about the alarms:

 - if you delete the alarms.rc file a new one is created with this text:
	
	!============================
	! BBDigitalEx 1.0 alarms file.
	! Enter alarms here - one per line.
	! For example:
	! 15.46: @ShrinkMemory 
	! 01.12: @BB8BallPredict 
	!============================

 - after this every line you add is a allarm
 - the line should start with the time you wont to execute the bro@am
 - dont forget that the hours 1-9 got to have a leading 0 before them
 - after the time just add the bro@am you wont to execute
 - best used with combination of broamrelay and bbinterface
 - if the broam wont start with @ nothing is done
 - if enable alarms is false the digitalEx doestn read this file
 - the file is tested everytime the seconds are zero

-------------------------------------------------------------------------

something about the bitmap draw mode:

 - when you want to use bitmap numbers you have to first create the 
   bitmap. the numbers on the bitmap have to be divided into 10 parts
   with the same width and one with a half width.
 - the numbers have to be in this order 0123456789: 0-9 have the same width
   : has a half width. the width is calculated as the bitmap.width / 10.5 
 - if the bitmap mode is selected and no bitmap is loaded the vertical 
   mode is used
-------------------------------------------------------------------------

clock formating:

 %a Abbreviated weekday name 
 %A Full weekday name 
 %b Abbreviated month name 
 %B Full month name 
 %c Date and time representation appropriate for locale 
 %d Day of month as decimal number (01 - 31) 
 %H Hour in 24-hour format (00 - 23) 
 %I Hour in 12-hour format (01 - 12) 
 %j Day of year as decimal number (001 - 366) 
 %m Month as decimal number (01 - 12) 
 %M Minute as decimal number (00 - 59) 
 %p Current locale's A.M./P.M. indicator for 12-hour clock 
 %S Second as decimal number (00 - 59) 
 %U Week of year as decimal number, with Sunday as first day of week 
    (00 - 53) 
 %w Weekday as decimal number (0 - 6; Sunday is 0) 
 %W Week of year as decimal number, with Monday as first day of week 
    (00 - 53) 
 %x Date representation for current locale 
 %X Time representation for current locale 
 %y Year without century, as decimal number (00 - 99) 
 %Y Year with century, as decimal number 
 %% Percent sign 
 %z Either the time-zone name or time zone abbreviation, depending on 
    registry settings; no characters if time zone is unknown 
 %Z same as above


 %#a, %#A, %#b, %#B, %#p, %#X, %#z, %#Z, %#% # flag is ignored 
 %#c Long date and time representation, appropriate for current locale. 
     example: "Tuesday, March 14, 1995, 12:41:29".  
 %#x Long date representation, appropriate to current locale. 
     example: "Tuesday, March 14, 1995".  
 %#d, %#H, %#I, %#j, %#m, %#M, %#S, %#U, %#w, %#W, %#y, %#Y Remove
     leading zeros (if any). 


-------------------------------------------------------------------------

future plans for this plugin:

 - change the lclick into ldblclick... 

-------------------------------------------------------------------------

version history:

BBDigitalEx 1.0 beta 7 - 25/01/2005

 - added a 4 draw mode BITMAP
 - added the blink setting ... true - the : blinks.. false - dont

BBDigitalEx 1.0 beta 6 - 20/08/2004

 - added a clock formating setting
 - aded the draw mode menu (the modes are VERTICAL,HORIZONTAL,and TEXT)
 - the am/pm is no longer drawn, if you want it use the %p in the date
   setting
 - cleaned the paint procedure...

BBDigitalEx 1.0 beta 5 - 18/08/2004

 - added a @BBDigitalExPlayWav # bro@m, so if you wont to play a .wav
   sound as an alarm you just use this bro@m (# is the full path to the
   .wav file)
 - added universal hour alarm... for example if you want to play ping.waw 
   every full hour just add this line to your alarms.rc
   NN.00: @BBDigitalExPlayWav d:\ping.wav (just change the .wav path)
 - changed the messagebox icon
   

BBDigitalEx 1.0 beta 4 - 17/08/2004

 - added a @BBDigitalExMessage # bro@m, so if you wont to make a allarm
   write a message you just use this bro@m (# is the text)

BBDigitalEx 1.0 beta 3 - 16/08/2004

 - show seconds setting...
 - remove the first zero in hour setting added..
 - introducing bro@am alarms
 - enable alarms setting added

BBDigitalEx 1.0 beta 2 - 12/08/2004

 - added a second drawing mode
 - added a 24h format..

BBDigitalEx 1.0 beta 1 - 11/08/2004
  
 - first version

--------------------------------------------------------------------------

Created by Miroslav Petrasko [Theo] (theo.devil@gmx.net)

                                              all rights reserved... C2004

--------------------------------------------------------------------------

BBDigitalEx IS PROVIDED  "AS IS"  WITHOUT WARRANTY OF ANY KIND. THE AUTHOR
DISCLAIMS  ALL  WARRANTIES,  EITHER  EXPRESS  OR  IMPLIED,  INCLUDING  THE 
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. IN NO
EVENT  SHALL  THE  AUTHOR  OR  ITS  SUPPLIERS  BE  LIABLE  FOR ANY DAMAGES 
WHATSOEVER  INCLUDING  DIRECT,  INDIRECT,  INCIDENTAL, CONSEQUENTIAL, LOSS
OF BUSINESS  PROFITS  OR  SPECIAL  DAMAGES,  EVEN  IF  THE  AUTHOR  OR ITS 
SUPPLIERS  HAVE  BEEN   ADVISED  OF   THE  POSSIBILITY  OF  SUCH  DAMAGES.


--------------------------------------------------------------------------
-                                                                        -
-         *  *                 *              *          *               -        
-         ** ***   **   **     ***   **   ******     *** *  *            -        
-         *  *  * *  * *  *    *  * *  * *    *     *    * *             -        
-         *  *  * **** *  *    *  * *  *  **  *      **  ***             -        
-         *  *  * *    *  *    *  * *  *    * *        * * *             -        
-         ** *  *  ***  **   * *  *  **  ***  **  * ***  *  *            - 
-                                                                        -
--------------------------------------------------------------------------