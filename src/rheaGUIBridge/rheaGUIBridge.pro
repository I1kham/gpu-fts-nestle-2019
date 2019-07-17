#-------------------------------------------------
#
# Project created by QtCreator 2019-07-12T10:46:40
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib

INCLUDEPATH += $$PWD/../rheaCommonLib

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
}

##message("GUIBridge: define: $$DEFINES")
##message("GUIBridge: config: $$CONFIG")



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

message ("rheaGUIBridge: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	TARGET = "$${PATH_TO_LIB}/$${CONFIG_NAME}_rheaGUIBridge"


#rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB} -l$${FULL_LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_LIB}
		unix:!macx: PRE_TARGETDEPS += $${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a


SOURCES += \
    CmdHandler.cpp \
    CmdHandler_ajaxReq.cpp \
    CmdHandler_eventReq.cpp \
    GUIBridge.cpp \
    GUIBridgeServer.cpp \
    CmdHandler/CmdHandler_ajaxReqSelAvailability.cpp \
    CmdHandler/CmdHandler_ajaxReqSelPrices.cpp \
    CmdHandler/CmdHandler_eventReqCPUMessage.cpp \
    CmdHandler/CmdHandler_eventReqCreditUpdated.cpp \
    CmdHandler/CmdHandler_eventReqSelAvailability.cpp \
    CmdHandler/CmdHandler_eventReqSelPrices.cpp \
    CmdHandler/CmdHandler_eventReqSelStatus.cpp \
    CmdHandler/CmdHandler_eventReqStartSel.cpp \
    CmdHandler/CmdHandler_eventReqStopSel.cpp

HEADERS += \
    CmdHandler.h \
    CmdHandler_eventReq.h \
    CommandHandlerList.h \
    GUIBridge.h \
    GUIBridgeEnumAndDefine.h \
    GUIBridgeServer.h \
    CmdHandler_ajaxReq.h \
    CmdHandler/CmdHandler_ajaxReqSelAvailability.h \
    CmdHandler/CmdHandler_ajaxReqSelPrices.h \
    CmdHandler/CmdHandler_eventReqCPUMessage.h \
    CmdHandler/CmdHandler_eventReqCreditUpdated.h \
    CmdHandler/CmdHandler_eventReqSelAvailability.h \
    CmdHandler/CmdHandler_eventReqSelPrices.h \
    CmdHandler/CmdHandler_eventReqSelStatus.h \
    CmdHandler/CmdHandler_eventReqStartSel.h \
    CmdHandler/CmdHandler_eventReqStopSel.h
unix {
    target.path = /usr/lib
    INSTALLS += target
}
