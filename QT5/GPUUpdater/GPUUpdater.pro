#-------------------------------------------------
#
# Project created by QtCreator 2019-10-15T10:19:27
#
#-------------------------------------------------

QT       += core gui
QT       += webkitwidgets
CONFIG	+= c++14
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x -pthread -Wno-unused-result -Wno-expansion-to-defined -Wcomment
QMAKE_CFLAGS += -std=gnu++0x -pthread -Wcomment
LIBS += -pthread

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


THIS_EXE_NAME="GPUUpdater"

message ("$${THIS_EXE_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_BIN}/$${CONFIG_NAME}_$${THIS_EXE_NAME}"

#depends on rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"


CONFIG(release, debug|release) {
	#QMAKE_CXXFLAGS += -O2
	#QMAKE_CFLAGS += -O2
	#CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}

SOURCES += \
    ../../src/GPUUpdater/main.cpp


