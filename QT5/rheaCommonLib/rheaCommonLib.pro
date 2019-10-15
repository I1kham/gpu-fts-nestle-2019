#-------------------------------------------------
#
# Project created by QtCreator 2019-06-12T10:33:17
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl link_prl


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

THIS_LIBRARY_NAME="rheaCommonLib"

message ("$${THIS_LIBRARY_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_LIB}/$${CONFIG_NAME}_$${THIS_LIBRARY_NAME}"

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}


SOURCES += \
    ../../src/rheaCommonLib/mtrand.cpp \
    ../../src/rheaCommonLib/rhea.cpp \
    ../../src/rheaCommonLib/rheaAllocator.cpp \
    ../../src/rheaCommonLib/rheaBase64.cpp \
    ../../src/rheaCommonLib/rheaBit.cpp \
    ../../src/rheaCommonLib/rheaDate.cpp \
    ../../src/rheaCommonLib/rheaDateTime.cpp \
    ../../src/rheaCommonLib/rheaJSONParser.cpp \
    ../../src/rheaCommonLib/rheaLinearBuffer.cpp \
    ../../src/rheaCommonLib/rheaLogger.cpp \
    ../../src/rheaCommonLib/rheaLoggerContext.cpp \
    ../../src/rheaCommonLib/rheaLogTargetConsole.cpp \
    ../../src/rheaCommonLib/rheaLogTargetFile.cpp \
    ../../src/rheaCommonLib/rheaMemory.cpp \
    ../../src/rheaCommonLib/rheaMemoryTracker.cpp \
    ../../src/rheaCommonLib/rheaSha1.cpp \
    ../../src/rheaCommonLib/rheaStaticBuffer.cpp \
    ../../src/rheaCommonLib/rheaString.cpp \
    ../../src/rheaCommonLib/rheaStringConvert.cpp \
    ../../src/rheaCommonLib/rheaStringFormat.cpp \
    ../../src/rheaCommonLib/rheaStringParser.cpp \
    ../../src/rheaCommonLib/rheaStringParserIter.cpp \
    ../../src/rheaCommonLib/rheaThread.cpp \
    ../../src/rheaCommonLib/rheaThreadMsgQ.cpp \
    ../../src/rheaCommonLib/rheaTime24.cpp \
    ../../src/rheaCommonLib/rheaUtils.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOS.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOSEvent.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOSSerialPort.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOSSocket.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOSThread.cpp \
    ../../src/rheaCommonLib/OS/linux/linuxOSWaitableGrp.cpp \
    ../../src/rheaCommonLib/OS/win/winOS.cpp \
    ../../src/rheaCommonLib/OS/win/winOSSerialPort.cpp \
    ../../src/rheaCommonLib/OS/win/winOSSocket.cpp \
    ../../src/rheaCommonLib/OS/win/winOSThread.cpp \
    ../../src/rheaCommonLib/OS/win/winOSWaitableGrp.cpp \
    ../../src/rheaCommonLib/Protocol/IProtocol.cpp \
    ../../src/rheaCommonLib/Protocol/IProtocolChannell.cpp \
    ../../src/rheaCommonLib/Protocol/ProtocolChSocketTCP.cpp \
    ../../src/rheaCommonLib/Protocol/ProtocolConsole.cpp \
    ../../src/rheaCommonLib/Protocol/ProtocolSocketServer.cpp \
    ../../src/rheaCommonLib/Protocol/ProtocolWebsocket.cpp \
    ../../src/rheaCommonLib/SimpleLogger/StdoutLogger.cpp


HEADERS += \
    ../../src/rheaCommonLib/mtrand.h \
    ../../src/rheaCommonLib/rhea.h \
    ../../src/rheaCommonLib/rheaAllocator.h \
    ../../src/rheaCommonLib/rheaAllocatorSimple.h \
    ../../src/rheaCommonLib/rheaAllocatorTrackingPolicy.h \
    ../../src/rheaCommonLib/rheaBit.h \
    ../../src/rheaCommonLib/rheaBufferView.h \
    ../../src/rheaCommonLib/rheaCriticalSection.h \
    ../../src/rheaCommonLib/rheaDataTypes.h \
    ../../src/rheaCommonLib/rheaDate.h \
    ../../src/rheaCommonLib/rheaDateTime.h \
    ../../src/rheaCommonLib/rheaEnumAndDefine.h \
    ../../src/rheaCommonLib/rheaEvent.h \
    ../../src/rheaCommonLib/rheaFastArray.h \
    ../../src/rheaCommonLib/rheaFIFO.h \
    ../../src/rheaCommonLib/rheaHandleArray.h \
    ../../src/rheaCommonLib/rheaHandleBaseArray.h \
    ../../src/rheaCommonLib/rheaHandleUID88.h \
    ../../src/rheaCommonLib/rheaHandleUID040210.h \
    ../../src/rheaCommonLib/rheaIntrusiveFreeList.h \
    ../../src/rheaCommonLib/rheaJSONParser.h \
    ../../src/rheaCommonLib/rheaLIFO.h \
    ../../src/rheaCommonLib/rheaLinearBuffer.h \
    ../../src/rheaCommonLib/rheaLogger.h \
    ../../src/rheaCommonLib/rheaLogTargetConsole.h \
    ../../src/rheaCommonLib/rheaLogTargetFile.h \
    ../../src/rheaCommonLib/rheaMemory.h \
    ../../src/rheaCommonLib/rheaMemoryTracker.h \
    ../../src/rheaCommonLib/rheaNetBufferView.h \
    ../../src/rheaCommonLib/rheaRandom.h \
    ../../src/rheaCommonLib/rheaStaticBuffer.h \
    ../../src/rheaCommonLib/rheaString.h \
    ../../src/rheaCommonLib/rheaThread.h \
    ../../src/rheaCommonLib/rheaThreadSafePolicy.h \
    ../../src/rheaCommonLib/rheaTime24.h \
    ../../src/rheaCommonLib/rheaUtils.h \
    ../../src/rheaCommonLib/rheaWaitableFIFO.h \
    ../../src/rheaCommonLib/OS/linux/linuxOS.h \
    ../../src/rheaCommonLib/OS/linux/linuxOSInclude.h \
    ../../src/rheaCommonLib/OS/linux/linuxOSSerialPort.h \
    ../../src/rheaCommonLib/OS/linux/linuxOSSocket.h \
    ../../src/rheaCommonLib/OS/linux/linuxOSWaitableGrp.h \
    ../../src/rheaCommonLib/OS/win/winOS.h \
    ../../src/rheaCommonLib/OS/win/winOSInclude.h \
    ../../src/rheaCommonLib/OS/win/winOSSerialPort.h \
    ../../src/rheaCommonLib/OS/win/winOSSocket.h \
    ../../src/rheaCommonLib/OS/win/winOSWaitableGrp.h \
    ../../src/rheaCommonLib/OS/OS.h \
    ../../src/rheaCommonLib/OS/OSEnum.h \
    ../../src/rheaCommonLib/OS/OSInclude.h \
    ../../src/rheaCommonLib/OS/OSSerialPort.h \
    ../../src/rheaCommonLib/OS/OSSocket.h \
    ../../src/rheaCommonLib/OS/OSWaitableGrp.h \
    ../../src/rheaCommonLib/Protocol/IProtocol.h \
    ../../src/rheaCommonLib/Protocol/IProtocolChannell.h \
    ../../src/rheaCommonLib/Protocol/Protocol.h \
    ../../src/rheaCommonLib/Protocol/ProtocolChSocketTCP.h \
    ../../src/rheaCommonLib/Protocol/ProtocolConsole.h \
    ../../src/rheaCommonLib/Protocol/ProtocolSocketServer.h \
    ../../src/rheaCommonLib/Protocol/ProtocolWebsocket.h \
    ../../src/rheaCommonLib/SimpleLogger/ISimpleLogger.h \
    ../../src/rheaCommonLib/SimpleLogger/NullLogger.h \
    ../../src/rheaCommonLib/SimpleLogger/StdoutLogger.h
    
unix {
    target.path = /usr/lib
    INSTALLS += target
}
