TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++0x -pthread -Wno-unused-result -Wno-expansion-to-defined
QMAKE_CFLAGS += -std=gnu++0x -pthread
LIBS += -pthread

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
}

#direttive specifiche per quando compilo su ubuntu desktop
contains(DEFINES, PLATFORM_UBUNTU_DESKTOP) {

	CONFIG(debug, debug|release) {
		CONFIG_NAME="DESKTOP64_DEBUG"
		BUILD_FOLDER = "build/Desktop-Debug64/"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="DESKTOP64_RELEASE"
		BUILD_FOLDER = "build/Desktop-Release64/"
	}
}


message ("GPUConsole: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	TARGET = "$${PATH_TO_BIN}/$${CONFIG_NAME}_GPUConsole"


#rheaGUIBridgle Library
LIBRARY_NAME="rheaGUIBridge"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB} -l$${FULL_LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_LIB}
		unix:!macx: PRE_TARGETDEPS += $${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a

#rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB} -l$${FULL_LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_LIB}
		unix:!macx: PRE_TARGETDEPS += $${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a




SOURCES += main.cpp \
    Client.cpp

HEADERS += \
    Client.h

