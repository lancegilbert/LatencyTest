TEMPLATE = app
TARGET = latencytest

CONFIG += c++11

CONFIG -= flat

QT = core gui widgets

INCLUDEPATH += "$$_PRO_FILE_PWD_/"
INCLUDEPATH += "$$_PRO_FILE_PWD_/external/ASIOSDK2/ASIOSDK2.3"

SOURCES += 	./app/LTApplication.cpp \
			./app/LTMainWindow.cpp \
			./midi/LTWindowsMIDI.cpp \
			./midi/LTMIDIDevice.cpp \
			./audio/LTAudioDevice.cpp \
			./audio/LTWindowsASIO.cpp

HEADERS += 	./app/LTApplication.h \
			./app/LTMainWindow.h \
			./midi/LTWindowsMIDI.h\
			./midi/LTMIDIDevice.h \
			./audio/LTAudioDevice.h \
			./audio/LTWindowsASIO.h

win32*:contains(QMAKE_HOST.arch, x86_64): {

	LIBS += Winmm.lib

    CONFIG(debug, debug|release) {
		DESTDIR = ./bin/x64/debug
		LIBS += ../external/asiosdk/lib/x64/asiosdkd.lib
    } 
    else {
    	DESTDIR = ./bin/x64/release
    	LIBS += ../external/asiosdk/lib/x64/asiosdk.lib
    }
}

FORMS += \
    ui/LTMainWindowUI.ui

