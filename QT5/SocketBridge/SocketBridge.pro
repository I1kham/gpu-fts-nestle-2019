#-------------------------------------------------
#
# Project created by QtCreator 2019-10-14T10:24:49
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl link_prl


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


THIS_LIBRARY_NAME="SocketBridge"

message ("$${THIS_LIBRARY_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_LIB}/$${CONFIG_NAME}_$${THIS_LIBRARY_NAME}"


#depends on rheaDB libray
LIBRARY_NAME="rheaDB"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		unix:!macx: PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

#depends on rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}


SOURCES += \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBC.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBE.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBQ.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelAvailability.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelPrices.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqClientList.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUMessage.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCreditUpdated.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelAvailability.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelPrices.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStartSel.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStopSel.cpp \
    ../../src/SocketBridge/CmdHandler.cpp \
    ../../src/SocketBridge/CmdHandler_ajaxReq.cpp \
    ../../src/SocketBridge/CmdHandler_eventReq.cpp \
    ../../src/SocketBridge/DBList.cpp \
    ../../src/SocketBridge/IdentifiedClientList.cpp \
    ../../src/SocketBridge/SocketBridge.cpp \
    ../../src/SocketBridge/SocketBridgeFileT.cpp \
    ../../src/SocketBridge/SocketBridgeServer.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUbtnProgPressed.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUIniParam.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqDataAudit.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFile.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFileTimestamp.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqWriteLocalVMCDataFile.cpp

HEADERS += \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBC.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBE.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBQ.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelAvailability.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelPrices.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqClientList.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUMessage.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCreditUpdated.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelAvailability.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelPrices.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStartSel.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStopSel.h \
    ../../src/SocketBridge/CmdHandler.h \
    ../../src/SocketBridge/CmdHandler_ajaxReq.h \
    ../../src/SocketBridge/CmdHandler_eventReq.h \
    ../../src/SocketBridge/CommandHandlerList.h \
    ../../src/SocketBridge/DBList.h \
    ../../src/SocketBridge/IdentifiedClientList.h \
    ../../src/SocketBridge/SocketBridge.h \
    ../../src/SocketBridge/SocketBridgeEnumAndDefine.h \
    ../../src/SocketBridge/SocketBridgeFileT.h \
    ../../src/SocketBridge/SocketBridgeFileTEnumAndDefine.h \
    ../../src/SocketBridge/SocketBridgeServer.h \
    ../../src/SocketBridge/SocketBridgeVersion.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUbtnProgPressed.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUIniParam.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqDataAudit.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFile.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFileTimestamp.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqWriteLocalVMCDataFile.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
