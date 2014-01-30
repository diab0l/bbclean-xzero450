-------------------------------------------------------------------------
-                                                                       -
-  *****   *****     ***           *                    *               -
-  *    *  *    *   *   *          *                    *               -
-  *    *  *    *  *     *   ***   *   ***   * **    ** *   ***   * *   - 
-  *    *  *    *  *        *   *  *  *   *  **  *  *  **  *   *  **    -
-  ******  ******  *            *  *  *   *  *   *  *   *      *  *     - 
-  *    *  *    *  *         ****  *  *****  *   *  *   *   ****  *     -
-  *    *  *    *  *     *  *   *  *  *      *   *  *   *  *   *  *     - 
-  *    *  *    *   *   *   *  **  *  *   *  *   *  *   *  *  **  *     - 
-  *****   *****     ***     ** *  *   ***   *   *   ****   ** *  *     -  
-                                                                       -
-                          BBCalendar 1.2 beta 1                        -
-------------------------------------------------------------------------
									
BBCalendar is a calendar plugin... 
			
-------------------------------------------------------------------------

known issues:

 - there seems to be some problems in the old code from 1.0 version
   im going to rewrite most of it in a next few days
 - there are problems with some fonts... some dont wont to be drawn
   verticali... so when you choose the text position left and there
   is just empty space, try changing the date font...

-------------------------------------------------------------------------

some notes:

 - you cannot have two alarms with one day, if you have, the seccond one
   is ignored

-------------------------------------------------------------------------
controls:

 calendar:
 - left click on a day which has alarm: execute the alarm
 - left click somewhere else: nothing
 - ctrl + left click & drag: move
 - ctrl + right click on a day: opens alarm menu
 - ctrl + right click somewhere else: open plugin menu
 
 message box:
 - enter key in a messagebox - presses the ok button
 - ctrl + left click & drag: move
 - left click on ok button - closes the message box
 
-------------------------------------------------------------------------

bro@ms:
 
 @BBCalendarPreviousMonth
 @BBCalendarNextMonth
 @BBCalendarActualMonth 
	- control which month is shown in the calendar

 @BBCalendarDrawMode #
	- changes the draw mode 0< # <3 

 @BBCalendarDatePossition # 
	- changes the date possition 0< # <4

 @BBCalendarMessage #
	- this is if you wont to use a message from 
	  somewhere outside of the calendar, just add the
          text you wont to show instead of the #

 - there are more bro@ms, if you wont to know them, use the "show bro@ms"
   setting in bblean

-------------------------------------------------------------------------

something about the nameday:

03.01: Daniela
04.01: Drahoslav
05.01: Andrea
06.01: Antonia

this is the format of the names.rc file... it starts with a date and
then there is the name to be shown (you can show anything you want)

if you dont know what a nameday is look here http://www.mynameday.com/
or here http://www.calendar.sk/nameday-enskhor.php

-------------------------------------------------------------------------
something about the alarms:

 - if you delete the alarms.rc file a new one is created with this text:
	
	!============================
	! BBCalendar 1.1 alarms file.
	! Enter alarms here - one per line.
	! For example:
	! 01.08.2004: @BB8BallFortune 
	! 15.12.2003: Show This Message...
	! 15.NN.NNNN: Every month alarm...
	! 28.03.NNNN: Every year alarm...
	!============================

 - after this every line you add is a allarm
 - the line should start with the date you wont to add the alarm
 - dont forget that the days and months 1-9 got to have a leading 0 
   before them
 - if the text after the date starts with a @ the broam is executed
 - if the text starts with ! it is ignored
 - if it starts with something else a messagebox with that text is created
 - use \n or \N in the text if you wont to insert a newline
 - the file is tested on the plugin start or when you reload settings or 
   reload alarms
 - there a 3 types of alarms, the first one is day alarm which is added
   to a current day... for example: 05.02.2003: this is a day
 - the second is a every month alarm: 03.NN.NNNN: alarm
   which is added to every 03 day in a month
 - the last type is a every year alarm: 28.03.NNNN: somebodys birthsday
   which is added to a single day in every year
 - throught the menu only the day alarms can be added/removed/changed
 - if you have more then one alarm on a day, and the alarm is executed,
   if the alarms are broams, every single one is executed (first the day
   then the month and then the year), if there are texts, you will get a 
   message box with the "first emply line seccon empty line third" alarm

-------------------------------------------------------------------------

future plans for this plugin:

 - each alarms has its own picture (or default)
 - there was a request to be able to choose which day is the first in 
   week (not only sunday or monday)
 
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

 - dont use the %S, because the calendar is updatet every minute,
   so you dont se any changes

-------------------------------------------------------------------------

version history:
BBCalendar 1.2 beta 1 - 18/11/2004
 
 - added a placement menu
 - added a nameday displaing under the calendar

BBCalendar 1.1.1 - 11/9/2004

 - there has been a little problem with slitting under xoblite.. 
   so i repaired it

BBCalendar 1.1  - 10/09/2004

 - release version

BBCalendar 1.1 rc 2 - 08/09/2004

 - added a every month and every year alarm...
 - added the beginPluginEx function

BBCalendar 1.1 rc 1 - 06/09/2004

 - new bitmap options submenu (center/stretch bitmaps)
 - new fonts submenu (use default/define font for date/week/days)
 - repaired some problems with the left and right date possition, 
   the left/right/center now effects this possition 
 - hopefully this version works on a win9x system vithout problem
   (i hope and hope and hope)

BBCalendar 1.1 beta 9 - 02/09/2004

 - next,previus,actual month are back
 - can have alarms to any day you wont (not only the actual month)
 - the bitmap submenu has become bigger, every part can have its own
   bitmap
 - every part (date,week,calendar,days,curent day, alarm) can have
   its own style including toolbar, button, buttonpr, label, winlabel,
   clock, rect, trans, bitmap, none..
 - the calendar is drawn in this order:
   calendar,date,week,days,alarm,current day
 - the bitmaps are no longer resized, they are centered 
 - the week has now position 1, position 2, both or none setting
 - all the bitmaps are draw only when the calendar part has the bitmap 
   style selected, only exclusion is the calendar path, where the bitmap
   is draw when the bitmap.calendar is different from ".none"
 - the date now has its own font size setting
 - the color in whitch the numbers on the calendar are draw is taken 
   from the days style, when this style is none, the calendar style is
   used, when from both styles can not be predicted the color 
   (for example there are both trasparent) the toolbar.textColor is 
   used 

BBCalendar 1.1 beta 8 - 01/09/2004

 - have modified almost every part of the plugin
 - now you can select the style of every part of the calendar...

BBCalendar 1.1 beta 7 - 30/08/2004

 - added the draw mode menu
 - added a vertical, line, row draw modes....
 - added more text placement settings...

BBCalendar 1.1 beta 6 - 29/08/2004

 - messages fit the text...
 - removed a bug which cosed the plugin to crash with some styles 
   (same problem in bb8ball 1.1 beta 2)
 - changed the way new allarms are added to the alarms file
 - changed the little date menus, when the day doesnt have a alarm, 
   the menu has only add in it when there is alarm, the menu has 
   the parts: change,remove,execute
 - the remove function doesnt really removes the alarm, just adds ! to
   the start of the alarm, so bbcalendar ignores it.... its really 
   removed after you add a new one to that day... (so if you have removed 
   a alarm and you wont it back just choose edit alarms and remove the ! 
   from the start)
 - did some minor changes in the way the calendar calculates, 
   where have you clicked....
 - the $ is no longer ignored, changed it to ! 
   (think this is more bbstyle...)
 - the 3 lines max is gone...
 - changed the day circle into a day rect... :-)

BBCalendar 1.1 beta 5 - 24/08/2004

 - aded blackbox style messages
 - changed the about dialog
 - use \n in your messages if you wont a new line (currently 3 lines max)

BBCalendar 1.1 beta 4 - 21/08/2004

 - added a little menu when you ctrl+rightclick a calendar day
   containing a execute broam and a add broam 
 - now the left click work in the slit too
 - added a @BBCalendarMessage # broam
 - the alarms are threated a little different in this version
   when the alarm starts with @ for example:
   12.03.2004: @BBdosomethingbroam
   the broam is executed, when it starts with $ for example:
   25.05.2004: $something something something
   its ignored, and when it starts with something else for example:
   02.02.2001: this is something else
   the alarm text is displayed with the date in a messagebox

BBCalendar 1.1 beta 3 - 20/08/2004
 
 - now you can click on a date that has alarm to execute the alarm
   (works only in slit)
 - the date formating setting added
 - added a reloadalarmsrc in the settings menu 

BBCalendar 1.1 beta 2 - 19/08/2004

 - i have removed some features because im making a little more changes
   in the code
 - added bro@m alarms, currentli the allarms are triggered when the 
   bbcalendar starts and when its 12:00
 - you can sellect what bitmap is drawn  behind the number of a day
   whis has allarm...
 - added a bacground bitmap sellection
 - some text positioning added
 - added height and with changing from the menu 
 - added a editalarmsrc in the settings submenu 

BBCalendar 1.1 beta 1 - 16/08/2004

 - now there is a menu part called image where you can select the image
   that is drawn in the rect arround the current day (when nothing 
   selected the circle is drawn)
 - the controls have changed

BBCalendar 1.0 - ??/??/2004
 
 - the info about this is in the bbcalendar 1.0 readme

--------------------------------------------------------------------------

thanks:

 - thanks to Arc Angel who beta tested this plugin from the start... :-)

--------------------------------------------------------------------------

Created by Miroslav Petrasko [Theo] (theo.devil@gmx.net)

											  all rights reserved... C2004

--------------------------------------------------------------------------

BBCalendar IS  PROVIDED  "AS IS"  WITHOUT WARRANTY OF ANY KIND. THE AUTHOR
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