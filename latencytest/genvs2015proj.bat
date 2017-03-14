set PATH=%PATH%;C:\Qt\Qt5.8.0\5.8\msvc2015_64\bin
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
qmake -tp vc latencytest.pro