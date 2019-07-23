#-------------------------------------------------
#
# Project created by QtCreator 2019-06-12T10:33:17
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
}

##message("rheaCommonLib: define: $$DEFINES")
##message("rheaCommonLib: config: $$CONFIG")


#direttive specifiche per quando compilo per yocto embedded
contains(DEFINES, PLATFORM_YOCTO_EMBEDDED) {
	CONFIG(debug, debug|release) {
		CONFIG_NAME="EMBEDDED_DEBUG"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="EMBEDDED_RELEASE"
	}
}


#direttive specifiche per quando compilo su ubuntu desktop
contains(DEFINES, PLATFORM_UBUNTU_DESKTOP) {

	CONFIG(debug, debug|release) {
		CONFIG_NAME="DESKTOP64_DEBUG"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="DESKTOP64_RELEASE"
	}
}


message ("rheaCommonLib: configuration is $${CONFIG_NAME}")
TARGET = "../../../../lib/$${CONFIG_NAME}_rheaCommonLib"


SOURCES += \
    OS/linux/linuxOS.cpp \
    OS/linux/linuxOSEvent.cpp \
    OS/linux/linuxOSSerialPort.cpp \
    OS/linux/linuxOSSocket.cpp \
    OS/linux/linuxOSWaitableGrp.cpp \
    OS/linux/linuxOSThread.cpp \
    OS/win/winOS.cpp \
    OS/win/winOSSerialPort.cpp \
    OS/win/winOSSocket.cpp \
    OS/win/winOSThread.cpp \
    OS/win/winOSWaitableGrp.cpp \
    Websocket/WebsocketClient.cpp \
    Websocket/WebsocketServer.cpp \
    rhea.cpp \
    rheaBase64.cpp \
    rheaDate.cpp \
    rheaJSONParser.cpp \
    rheaLinearBuffer.cpp \
    rheaLogger.cpp \
    rheaLoggerContext.cpp \
    rheaLogTargetConsole.cpp \
    rheaLogTargetFile.cpp \
    rheaMemory.cpp \
    rheaSha1.cpp \
    rheaString.cpp \
    rheaStringConvert.cpp \
    rheaStringFormat.cpp \
    rheaStringParser.cpp \
    rheaStringParserIter.cpp \
    rheaThread.cpp \
    rheaThreadMsgQ.cpp \
    rheaTime24.cpp \
    rheaBit.cpp

HEADERS += \
    OS/OS.h \
    OS/OSEvent.h \
    OS/OSSerialPort.h \
    OS/OSSocket.h \
    OS/OSWaitableGrp.h \
    OS/linux/linuxOS.h \
    OS/linux/linuxOSSocket.h \
    OS/linux/linuxOSWaitableGrp.h \
    OS/OS.h \
    OS/OSSerialPort.h \
    OS/OSSocket.h \
    OS/OSWaitableGrp.h \
    OS/OSInclude.h \
    OS/linux/linuxOSInclude.h \
    OS/linux/linuxOSSerialPort.h \
    OS/OSEnum.h \
    OS/win/winOS.h \
    OS/win/winOSInclude.h \
    OS/win/winOSSerialPort.h \
    OS/win/winOSSocket.h \
    OS/win/winOSWaitableGrp.h \
    Websocket/WebsocketServer.h \
    rhea.h \
    rheaAllocator.h \
    rheaAllocatorSimple.h \
    rheaAllocatorTrackingPolicy.h \
    rheaDataTypes.h \
    rheaDate.h \
    rheaDateTime.h \
    rheaEnumAndDefine.h \
    rheaFastArray.h \
    rheaFIFO.h \
    rheaHandleArray.h \
    rheaHandleBaseArray.h \
    rheaHandleUID88.h \
    rheaHandleUID040210.h \
    rheaIntrusiveFreeList.h \
    rheaJSONParser.h \
    rheaLIFO.h \
    rheaLinearBuffer.h \
    rheaLogger.h \
    rheaLogTargetConsole.h \
    rheaLogTargetFile.h \
    rheaMemory.h \
    rheaString.h \
    rheaThread.h \
    rheaThreadSafePolicy.h \
    rheaTime24.h \
    rheaUtils.h \
    rheaWaitableFIFO.h \
    rheaBit.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
