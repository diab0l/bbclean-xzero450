-------------------------------------------------------------------------
-                                                                       -
-     ******   ******   *       *                       *   **          -  
-     *     *  *     *  **     **                          *            -  
-     *     *  *     *  **     **                          *            -  
-     *     *  *     *  * *   * *   ***    ** *  * **   * *** *   *     -  
-     ******   ******   * *   * *  *   *  *  **  **  *  *  *  *   *     -  
-     *     *  *     *  *  * *  *      *  *   *  *   *  *  *   * *      -  
-     *     *  *     *  *  * *  *   ****  *   *  *   *  *  *   * *      -  
-     *     *  *     *  *  * *  *  *   *  *   *  *   *  *  *   * *      -  
-     *     *  *     *  *   *   *  *  **  *  **  *   *  *  *    *       -  
-     ******   ******   *   *   *   ** *   ** *  *   *  *  *    *       -  
-                                             *                 *       -  
-                                         *   *                 *       -  
-                                          ***                **        -  
-                                                                       -
-                          BBMagnify 1.0 beta 5                         -
-------------------------------------------------------------------------
									
BBMagnify is a magnifier plugin... and it shows the cursor position and
the color bellow the cursor				

I wanted the picture in my SideBar to do something so i created this...									
-------------------------------------------------------------------------

known issues:

 - nothing i know about...

-------------------------------------------------------------------------

controls:

 - left click: start/stop magnifying
 - ctrl + left click & drag: move
 - ctrl + right click: open menu

-------------------------------------------------------------------------

bro@ms:


	@BBMagnifyDrawBorder
	@BBMagnifyDrawCross
	@BBMagnifyDrawPossition
	@BBMagnifyDrawColor
	@BBMagnifyAtStart

	@BBMagnifyWidth #
	@BBMagnifyHeight #
	@BBMagnifyFontSize #

	@BBMagnifyStyleToolbar
	@BBMagnifyStyleButton
	@BBMagnifyStyleButtonPr
	@BBMagnifyStyleLabel
	@BBMagnifyStyleWindowLabel
	@BBMagnifyStyleClock

	@BBMagnifySlit
	@BBMagnifyPluginToggle
	@BBMagnifyOnTop
	@BBMagnifyTransparent
	@BBMagnifySetTransparent #
	@BBMagnifyFullTrans
	@BBMagnifySnapToEdge

	@BBMagnifyLoadBitmap
	@BBMagnifySetBitmap #
	@BBMagnifyNoBitmap

	@BBMagnifyEditRC
	@BBMagnifyReloadSettings
	@BBMagnifySaveSettings

	@BBMagnifyRatio # 

	@BBMagnifyAbout

	@BBMagnifyColorToClipboard

 - i think that the names expalain what they do 
 - the # means that a number has to be inserted...
 - by setBitmap bro@am the # means the full path to the used bitmap
 - color to clipboard sends the color bellow the cursor to the clipboard
   in the standard format ( for example #00000F) all letters are upper
   case

-------------------------------------------------------------------------
some comments:
	
 - the color to clipboard bro@am is best used with the bbkeys plugin
   (thanks to ArcAngel for this idea)
   when this broam is sent the plugin shous the text "COPIED" on top of
   plugin window for about 2s
 - the ratio can be other then the 1,2,4,8,16,32, just change the
   ratio in the rc file ... min is 1
 - i dont really know if this works on win9x systems... i couldnt really 
   test it
 - the text is drawn using the color that is set to the text in the used 
   style setting (hopefully someone understands this line... :-)) 
 
-------------------------------------------------------------------------

future plans for this plugin:

 - nothing right now

-------------------------------------------------------------------------

version history:
BBMagnify 1.0 beta 5 - 14/08/2004

 - color to clipboard bro@am
 - added some xoblite stuff: beginPluginEx & case PLUGIN_BROAMS
 

BBMagnify 1.0 beta 4 - 13/08/2004

 - now you can show the color below the cursor..
 - the draw border function effects the bitmap background ...  

BBMagnify 1.0 beta 3 - 10/08/2004
  
 - added a croshair
 - added a show/dont show the cursor position setting
 - added a setting if you wont/dont wont to magnify at start  

BBMagnify 1.0 beta 2 - 09/08/2004
  
 - added the aggressiveoptimize.h

BBMagnify 1.0 beta 1 - 06/08/2004

 - first version

--------------------------------------------------------------------------

Created by Miroslav Petrasko [Theo] (theo.devil@gmx.net)

                                              all rights reserved... C2004

--------------------------------------------------------------------------

BBMagnify  IS  PROVIDED  "AS IS"  WITHOUT WARRANTY OF ANY KIND. THE AUTHOR
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