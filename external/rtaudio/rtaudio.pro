TEMPLATE = lib
TARGET = rtaudio

CONFIG += staticlib

QT =

# DEFINES -= UNICODE

SOURCES +=	rtaudio\RtAudio.cpp

HEADERS +=	rtaudio\RtAudio.h

INCLUDEPATH += 	./ \
				./rtaudio \
				../asiosdk/ASIOSDK2.3/ \
				../asiosdk/ASIOSDK2.3/common \
				../asiosdk/ASIOSDK2.3/host \
				../asiosdk/ASIOSDK2.3/host/pc \
				./rtaudio/include \

win32*:contains(QMAKE_HOST.arch, x86_64): {
	DEFINES += 	__WINDOWS_DS__ \
				__WINDOWS_ASIO__ \
				__WINDOWS_WASAPI__
				
	DESTDIR = ./lib/x64

    CONFIG(debug, debug|release) {
		TARGET = $$join(TARGET,,,d)
    } 
}
