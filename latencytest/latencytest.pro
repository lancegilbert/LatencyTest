TEMPLATE = app
TARGET = latencytest

CONFIG += c++11

CONFIG -= flat

QT = core gui widgets

INCLUDEPATH += 	"$$_PRO_FILE_PWD_/" \
				"$$_PRO_FILE_PWD_/../external/rtaudio/rtaudio" \
				"$$_PRO_FILE_PWD_/../external/asiosdk/ASIOSDK2.3" \
				"$$_PRO_FILE_PWD_/../external/asiosdk/ASIOSDK2.3\common" \
				"$$_PRO_FILE_PWD_/../external/asiosdk/ASIOSDK2.3\host" \
				"$$_PRO_FILE_PWD_/../external/asiosdk/ASIOSDK2.3\host\pc"

SOURCES += 	./app/LTApplication.cpp \
			./app/LTMainWindow.cpp \
			./app/LTRowWidget.cpp \
			./midi/LTWindowsMIDI.cpp \
			./midi/LTMIDIDevice.cpp \
			./audio/LTAudioDriver.cpp \
			./audio/LTWindowsASIO.cpp \
			./audio/LTRTAudio.cpp

HEADERS += 	./app/LTApplication.h \
			./app/LTMainWindow.h \
			./app/LTRowWidget.h \
			./midi/LTWindowsMIDI.h\
			./midi/LTMIDIDevice.h \
			./audio/LTAudioDriver.h \
			./audio/LTWindowsASIO.h \
			./audio/LTRTAudio.h

win32*:contains(QMAKE_HOST.arch, x86_64): {

	LIBS += Winmm.lib

    CONFIG(debug, debug|release) {
		DESTDIR = ./bin/x64/debug
		LIBS += ../external/rtaudio/lib/x64/rtaudiod.lib
		LIBS += ../external/asiosdk/lib/x64/asiosdkd.lib
    } 
    else {
    	DESTDIR = ./bin/x64/release
    	LIBS += ../external/rtaudio/lib/x64/rtaudio.lib
    	LIBS += ../external/asiosdk/lib/x64/asiosdk.lib
    }
}

FORMS += \
    ui/LTMainWindowUI.ui \
    ui/LTRowWidgetUI.ui

