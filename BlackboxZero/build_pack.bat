set path="C:\Program Files\WinRAR\";%path%

set PREFIX="c:"
set OUTDIR=c:/_builds

rem set BUILD="bbZero.vs32.dbg"
rem set BUILD="bbZero.vs32_xp.dbg"
rem set BUILD="bbZero.vs64.dbg"

set BUILD="bbZero.vs32"
winrar a -z%OUTDIR%/comment.txt -afzip -df %OUTDIR%/%BUILD%.zip %PREFIX%/%BUILD%

set BUILD="bbZero.vs64"
winrar a -z%OUTDIR%/comment.txt -afzip -df %OUTDIR%/%BUILD%.zip %PREFIX%/%BUILD%

set BUILD="bbZero.vs32_xp"
winrar a -z%OUTDIR%/comment.txt -afzip -df %OUTDIR%/%BUILD%.zip %PREFIX%/%BUILD%

pause
