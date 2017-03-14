TEMPLATE = lib
TARGET = asiosdk

CONFIG += staticlib

QT =

DEFINES -= UNICODE

SOURCES +=	ASIOSDK2.3\common\asio.cpp \
	 		ASIOSDK2.3\host\asiodrivers.cpp \
			ASIOSDK2.3\host\pc\asiolist.cpp

HEADERS +=	ASIOSDK2.3\common\asio.h \
	 		ASIOSDK2.3\host\asiodrivers.h \
			ASIOSDK2.3\host\pc\asiolist.h \
			ASIOSDK2.3\host\ginclude.h 

INCLUDEPATH += 	./ \
				ASIOSDK2.3/ \
				ASIOSDK2.3/common \
				ASIOSDK2.3/host \
				ASIOSDK2.3/host/pc 

win32*:contains(QMAKE_HOST.arch, x86_64): {
	DESTDIR = ./lib/x64

    CONFIG(debug, debug|release) {
		TARGET = $$join(TARGET,,,d)
    } 
}
