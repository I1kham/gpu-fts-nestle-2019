#-------------------------------------------------
#
# Project created by QtCreator 2015-07-14T17:00:17
#
#-------------------------------------------------

QT       += core gui

QT += network
QT += xml
QT += multimedia
QT += multimediawidgets
QT += widgets
QT += serialport

QT += webkit
QT += webkitwidgets
#QT += webengine
#QT += webenginewidgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GPU
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x -pthread -Wno-unused-result -Wno-expansion-to-defined
QMAKE_CFLAGS += -std=gnu++0x -pthread
LIBS += -pthread

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
}


##message("FusionGPU: define: $$DEFINES")
##message("FusionGPU: config: $$CONFIG")



#direttive specifiche per quando compilo per yocto embedded
contains(DEFINES, PLATFORM_YOCTO_EMBEDDED) {
	CONFIG(debug, debug|release) {
		CONFIG_NAME="EMBEDDED_DEBUG"
		BUILD_FOLDER = "build/Embedded-Debug/"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="EMBEDDED_RELEASE"
		BUILD_FOLDER = "build/Embedded-Release/"
	}
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


message ("GPUFusion: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	TARGET = "$${PATH_TO_BIN}/$${CONFIG_NAME}_GPU"


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




SOURCES += main.cpp\
	mainwindow.cpp \
    formboot.cpp \
    formprog.cpp \
    formresetgrncounter.cpp \
    formdebug.cpp \
    history.cpp \
    lang.cpp \
    Utils.cpp

HEADERS  += mainwindow.h \
    header.h \
    formboot.h \
    formprog.h \
    formresetgrncounter.h \
    formdebug.h \
    history.h \
    lang.h \
    Utils.h \
    SelectionAvailiability.h

FORMS    += mainwindow.ui \
    formboot.ui \
    formprog.ui \
    formresetgrncounter.ui \
    formdebug.ui
