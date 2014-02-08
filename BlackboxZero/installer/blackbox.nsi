# comment
# where to create install.exe
OutFile c:\_install\install.exe

InstallDir c:\bb_i

; Set the text to prompt user to enter a directory
DirText "This will install BlackBox 4 Windows program on your computer. Choose a directory"

Name "BlackBox 4 Windows"
RequestExecutionLevel admin
#TargetMinimalOS 5.1    ; target Windows XP or more recent

AddBrandingImage left 256

Page custom brandimage "" ": Brand Image"
Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Installer Section"
SectionEnd

Section "un.Uninstaller Section"
SectionEnd

Function brandimage
  SetOutPath "$TEMP"

  SetFileAttributes installer.bmp temporary
  File installer.bmp
  SetBrandingImage "$TEMP\installer.bmp" /resizetofit
FunctionEnd

/*
docs\
lib\

backgrounds\
styles\

blackbox.rc
bsetroot.rc
extensions.rc
menu.rc
plugins.rc
shellfolders.rc
stickywindows.ini

bbnote.exe
bbnote-proxy.dll
bbstylemaker.exe
blackbox.exe
bsetbg.exe
bsetroot.exe
bsetshell.exe
deskhook.dll
readme.txt
*/


Section "BlackBox"
  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
	File "bbnote.exe"
	File "bbnote-proxy.dll"
	File "bbstylemaker.exe"
	File "blackbox.exe"
	File "bsetbg.exe"
	File "bsetroot.exe"
	File "bsetshell.exe"
	File "deskhook.dll"
	File "readme.txt"
  #createShortCut "$SMPROGRAMS\BlackBox.lnk" ""
SectionEnd

Section /o "BlackBox Styles"
  SetOutPath $INSTDIR\backgrounds
  File /r "backgrounds\"
  SetOutPath $INSTDIR\styles
  File /r "styles\"
SectionEnd

Section "BlackBox Configs"
  SetOutPath $INSTDIR
  CreateDirectory $INSTDIR
	File "blackbox.rc"
	File "bsetroot.rc"
	File "extensions.rc"
	File "menu.rc"
	File "plugins.rc"
	File "shellfolders.rc"
	File "stickywindows.ini"
SectionEnd


/*
plugins\bbAnalog
plugins\bbColor3dc
plugins\bbIconBox
plugins\bbInterface
plugins\bbKeys
plugins\bbLeanBar
plugins\bbLeanSkin
plugins\bbSlit

plugins\BBAnalogEx
plugins\bbCalendar
plugins\BBDigitalEx
plugins\bbFoomp
plugins\bbLeanBar+
plugins\BBMagnify
plugins\BBPager
plugins\bbRecycleBin
plugins\BBRSS
plugins\BBStyle
plugins\BBSysMeter
plugins\bbWorkspaceWheel
plugins\SystemBarEx

plugins\BB8Ball
plugins\bbInterface_iTunes
plugins\BBMessageBox
plugins\BBXO
*/

Section "BlackBox Essential Plugins"
  SetOutPath $INSTDIR\plugins\bbAnalog
	File /r "plugins\bbAnalog\"
  SetOutPath $INSTDIR\plugins\bbColor3dc
	File /r "plugins\bbColor3dc\"
  SetOutPath $INSTDIR\plugins\bbIconBox
	File /r "plugins\bbIconBox\"
  SetOutPath $INSTDIR\plugins\bbInterface
	File /r "plugins\bbInterface\"
  SetOutPath $INSTDIR\plugins\bbKeys
	File /r "plugins\bbKeys\"
  SetOutPath $INSTDIR\plugins\bbLeanBar
	File /r "plugins\bbLeanBar\"
  SetOutPath $INSTDIR\plugins\bbLeanSkin
	File /r "plugins\bbLeanSkin\"
  SetOutPath $INSTDIR\plugins\bbSlit
	File /r "plugins\bbSlit\"
SectionEnd

Section /o "BlackBox Extended Plugin Set I."
  SetOutPath $INSTDIR\plugins\BBAnalogEx
	File /r "plugins\BBAnalogEx\"
  SetOutPath $INSTDIR\plugins\bbCalendar
	File /r "plugins\bbCalendar\"
  SetOutPath $INSTDIR\plugins\BBDigitalEx
	File /r "plugins\BBDigitalEx\"
  SetOutPath $INSTDIR\plugins\bbFoomp
	File /r "plugins\bbFoomp\"
  SetOutPath $INSTDIR\plugins\bbLeanBar+
	File /r "plugins\bbLeanBar+\"
  SetOutPath $INSTDIR\plugins\BBMagnify
	File /r "plugins\BBMagnify\"
  SetOutPath $INSTDIR\plugins\BBPager
	File /r "plugins\BBPager\"
  SetOutPath $INSTDIR\plugins\bbRecycleBin
	File /r "plugins\bbRecycleBin\"
  SetOutPath $INSTDIR\plugins\BBRSS
	File /r "plugins\BBRSS\"
  SetOutPath $INSTDIR\plugins\BBStyle
	File /r "plugins\BBStyle\"
  SetOutPath $INSTDIR\plugins\BBSysMeter
	File /r "plugins\BBSysMeter\"
  SetOutPath $INSTDIR\plugins\bbWorkspaceWheel
	File /r "plugins\bbWorkspaceWheel\"
  SetOutPath $INSTDIR\plugins\SystemBarEx
	File /r "plugins\SystemBarEx\"
SectionEnd

Section /o "BlackBox Extended Plugin Set II."
  SetOutPath $INSTDIR\plugins\BB8Ball
	File /r "plugins\BB8Ball\"
  SetOutPath $INSTDIR\plugins\bbInterface_iTunes
	File /r "plugins\bbInterface_iTunes\"
  SetOutPath $INSTDIR\plugins\BBMessageBox
	File /r "plugins\BBMessageBox\"
  SetOutPath $INSTDIR\plugins\BBXO
	File /r "plugins\BBXO\"
SectionEnd




