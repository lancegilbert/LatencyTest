TEMPLATE = app
TARGET = LatencyTest

CONFIG += c++11

CONFIG -= flat

QT = core gui widgets

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
    } 
    else {
    	DESTDIR = ./bin/x64/release
    }
}

FORMS += \
    ui/LTMainWindowUI.ui

