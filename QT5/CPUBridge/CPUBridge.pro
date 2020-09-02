#-------------------------------------------------
#
# Project created by QtCreator 2019-10-14T11:58:23
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

#direttive specifiche per quando compilo su rasPI
contains(DEFINES, PLATFORM_RASPI) {

        CONFIG(debug, debug|release) {
                CONFIG_NAME="RASPI_DEBUG"
        }

        CONFIG(release, debug|release) {
                CONFIG_NAME="RASPI_RELEASE"
        }
}



THIS_LIBRARY_NAME="CPUBridge"

message ("$${THIS_LIBRARY_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_LIB}/$${CONFIG_NAME}_$${THIS_LIBRARY_NAME}"


#depends on rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		unix:!macx: PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"


CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}

SOURCES += \
    ../../src/CPUBridge/CPUBridge.cpp \
    ../../src/CPUBridge/CPUBridgeServer.cpp \
    ../../src/CPUBridge/CPUChannelCom.cpp \
    ../../src/CPUBridge/CPUChannelFakeCPU.cpp \
    ../../src/CPUBridge/DA3.cpp \
    ../../src/CPUBridge/EVADTSParser.cpp \
    ../../src/CPUBridge/lang.cpp


HEADERS += \
    ../../src/CPUBridge/CPUBridge.h \
    ../../src/CPUBridge/CPUBridgeEnumAndDefine.h \
    ../../src/CPUBridge/CPUBridgeServer.h \
    ../../src/CPUBridge/CPUBridgeVersion.h \
    ../../src/CPUBridge/CPUChannel.h \
    ../../src/CPUBridge/CPUChannelCom.h \
    ../../src/CPUBridge/CPUChannelFakeCPU.h \
    ../../src/CPUBridge/DA3.h \
    ../../src/CPUBridge/EVADTSParser.h \
    ../../src/CPUBridge/lang.h


unix {
	target.path = /usr/lib
	INSTALLS += target
}
