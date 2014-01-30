-------------------------------------------------------------------------
-                                                                       -
-   *****   *****    ****               *     *         *               -  
-   *    *  *    *  *    *              **   **         *               -   
-   *    *  *    *  *    * *   *  ***   **   **   ***  ***  ***   * *   -  
-   *    *  *    *  *      *   * *   *  * * * *  *   *  *  *   *  **    -  
-   ******  ******   ****   * *  *      * * * *  *   *  *  *   *  *     -  
-   *    *  *    *       *  * *   ***   * * * *  *****  *  *****  *     -  
-   *    *  *    *  *    *  * *      *  * * * *  *      *  *      *     -  
-   *    *  *    *  *    *   *   *   *  *  *  *  *   *  *  *   *  *     -  
-   *****   *****    ****    *    ***   *  *  *   ***   **  ***   *     -  
-                           *                                           -
-                                                                       -
-                          BBSysMeter 1.0 beta7                         -
-------------------------------------------------------------------------
									
BBSysMeter is a system monitoring plugin..				
									
you can monitor : CPU, SWAP, RAM and drive space....			
and you have five drawing types... (try and see)			
 
The drive you want to monitor is specified by the drive.letter setting 
in the rc file (c, d, e,.....) or directly from the menu
 
-------------------------------------------------------------------------

controls:

 - ctrl + left click & drag: move
 - ctrl + right click: open menu

-------------------------------------------------------------------------              

draw types:   

   No fill on the left, fill on the right
                                                             
                        ******           ******                              
   1. Ellipse type   ******** ***     **     *****                                 
                    ********     *   *      *******                                   
                    *******      *   *   **********                                  
                     ***      ***     ************                                
                        ******           ******                              
                                                                       
   2. Hand type        ******              ******                         
                    ***  *   ***        ******   ***                      
                   *     *      *      *******      *                     
                  *     ***      *    *********      *                    
                  ****************    ****************                    

   3. Chart type                                                          
                  **************   **************                         
                  *  ** *      *   *  ** *      *                         
                  ***  * **  * *   *********  * *                         
                  *        **  *   **************                         
                  **************   **************                         

   4. Rect L->R type                                                      

                  ***************    ***************                      
                  ********      *    *      *      *                      
                  ********      *    *      *      *                      
                  ***************    ***************                      

   5. Rect B->T type                                                      
                        ********   ********                               
                        *      *   *      *                               
                        *      *   *      *                               
                        ********   ********                               
                        ********   *      *                               
                        ********   *      *                               
                        ********   ********     
-------------------------------------------------------------------------

bro@ms:

o my god this will be long :-)

 
.... (dont forget to do this...)


-------------------------------------------------------------------------

known issues:

 - nothing i know about

-------------------------------------------------------------------------

some comments:
	
 - bbsysmeter requires gdiplus.dll, dont forget this...
 - the refresh time can be other then the 1,2,5,10,30,60, just change the
   time in the rc file ... min is 1
 - the save messages function takes all broams that are broadcasted and
   saves them in the messages.rc file in the plugin directory
   be carefull if you have more than one copy of bbsysmeter (with 
   different names) in one folder, since all of them will save the 
   messages in the same file, so if you have for example 5 instances
   of bbsysmeter every broam is saved 5 times in the messages.rc
 - this version should be stable, i personaly run 9 (started with 3, 
   then 5, then 7 and now its 9 :-) ) copies at once and it looks like 
   there are no problems...
 - i dont really know if this works on win9x systems... i couldnt really 
   test it
 
//- take note that if you change a setting using the menu all copies that
//   are runnig change - no longer valid...

 - the text and graphics is drawn using the color that is set to the text
   in the used style setting (hopefully someone understands this 
   line... :-)) 
 - when slitted, the height and width changes from menu take effect after
   the reloading of the slit...
 
-------------------------------------------------------------------------

future plans for this plugin:

 - add a network monitor (upload/download)
 - add a drive read/save monitor setting
 - add a blackbox monitor to monitor the cpu and ram usage of 
   blackbox.exe
 

-------------------------------------------------------------------------

plans allready done:

 - add text position settings (text on top/bottom/left/right/no_text)
 - change the way the used broams are created so every instance has its 
   own set of broams baset on the dll name...
 - add a forth draw mode .... 	*******
				***   * CPU
				*******
   something like this....

-------------------------------------------------------------------------

version history:


BBSysMeter 1.0 beta7 - 16/08/2004

 - the fill now effects the ellipse draw mode
 - cleaned a lot of procedures at most the paint procedure
 - added a first version of the plain text monitor...(not quite good
   right now)
 - another first is the net in/out/total monitor setting, but this doesnt
   work at all right now...(it does something but not what it has to do)

BBSysMeter 1.0 beta6 - 12/08/2004

 - repaired the problem where when only the last character in plugin name
   where diferent the broams effected both plugins
 - the menu name is now the same as the .dll name...
 - repaired some minor problems...(e.g. the initial refres time was 
   0 instead of 1)

BBSysMeter 1.0 beta5 - 09/08/2004

 - removed the dot radius and line length setting, because now the hand
   mode can be eliptical... if somebody wonts dots, make a bitmap back
 - added two more draw modes and renamed one..
 - the rect l->r means that the rect is filled from left to right
 - the rect b->t means that the rect if filled from bottom to top
 - the fill setting now effects the hand, the graph, and the rect draw
   modes...
 - added a use anti-alias setting
 - started using the aggressiveoptimize.h


BBSysMeter 1.0 beta4 - 08/08/2004

 - removed draw color choosing (wrong way to go..)
 - sellect the drive letter directly from the menu
 - added text positioning... top/bottom/left/right
 - added the text width setting (this has effect when the text position
   is left or right, it determinates the space used for text drawing)
 - the plugin uses the .dll name to create the .rc file name and the
   broams... so if you name the .dll file xyz for example... the .rc file
   will be xyz.rc and the broams will start with @xyz instead of the 
   standard @BBSysMeter
 - the draw border problem repaired...

BBSysMeter 1.0 beta3 - 07/08/2004

 - added draw color choosing...
 - added a setting to set the refresh time
 - save broams function
 - changed the controls	to use the ctrl	button

BBSysMeter 1.0 beta2 - 03/08/2004

 - added the drive monitor setting
 - added rect and ellipse draw mode

BBSysMeter 1.0 beta1 - 26/07/2004

 - first version
 - monitor cpu, ram, swap
 - hand draw mode... 

--------------------------------------------------------------------------

Created by Miroslav Petrasko [Theo] (theo.devil@gmx.net)

                                              all rights reserved... C2004

--------------------------------------------------------------------------

BBSysMeter IS  PROVIDED  "AS IS"  WITHOUT WARRANTY OF ANY KIND. THE AUTHOR
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