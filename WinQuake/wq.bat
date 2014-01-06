@echo off
if x%1==xfast goto fast
if x%1==xsafe goto safe
if x%1==xverysafe goto verysafe
if x%1==xfastvid goto fastvid
if x%1==xfastsnd goto fastsnd
if x%1==xmax goto max

echo -------------------------------------------------------------------
echo Options for running WinQuake:
echo  wq max:      all features on, but doesn't work on all systems
echo  wq fast:     maximum speed, but doesn't work on all systems
echo  wq fastvid:  maximum video speed, but safer, probably slower sound
echo  wq fastsnd:  maximum sound speed, but safer, probably slower video
echo  wq safe:     very likely to run, but may be slower
echo  wq verysafe: almost sure to run, but probably slower, and no sound
echo -------------------------------------------------------------------
goto done

:max
winquake -dinput %2 %3 %4 %5 %6 %7 %8 %9
goto done

:fast
winquake %2 %3 %4 %5 %6 %7 %8 %9
goto done

:fastvid
winquake -wavonly %2 %3 %4 %5 %6 %7 %8 %9
goto done

:fastsnd
winquake -nodirectdraw -nowindirect %2 %3 %4 %5 %6 %7 %8 %9
goto done

:safe
winquake -nodirectdraw -nowindirect -wavonly %2 %3 %4 %5 %6 %7 %8 %9
goto done

:verysafe
winquake -dibonly -nosound -nojoy %2 %3 %4 %5 %6 %7 %8 %9
:done
