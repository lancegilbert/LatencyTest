set LTQTDIR=C:\Qt\Qt5.8.0\5.8\msvc2015_64\bin

set PATH=%PATH%;%LTQTDIR%

cd latencytest
call genvs2015proj.bat
cd ..

cd external
call genvs2015proj.bat
cd ..

REM Copy QT runtime libraries into working directory/staging folder

SETLOCAL

set COPYSRCDIR=%LTQTDIR%
set COPYDSTDIR=Deploy\Stage\Win64\Debug

copy %COPYSRCDIR%\Qt5Cored.dll %COPYDSTDIR%\
copy %COPYSRCDIR%\Qt5Guid.dll %COPYDSTDIR%\
copy %COPYSRCDIR%\Qt5Widgetsd.dll %COPYDSTDIR%\

set COPYDSTDIR=Deploy\Stage\Win64\Release


copy %COPYSRCDIR%\Qt5Core.dll %COPYDSTDIR%\
copy %COPYSRCDIR%\Qt5Gui.dll %COPYDSTDIR%\
copy %COPYSRCDIR%\Qt5Widgets.dll %COPYDSTDIR%\

ENDLOCAL

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
qmake -tp vc latencytest_sln.pro