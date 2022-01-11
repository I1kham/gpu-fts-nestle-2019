TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -std=c++0x -pthread -Wno-unused-result -Wno-expansion-to-defined
QMAKE_CFLAGS += -std=gnu++0x -pthread

#direttive specifiche per quando compilo per Rockchip
contains(DEFINES, PLATFORM_ROCKCHIP) {
        CONFIG(debug, debug|release) {
                CONFIG_NAME="ROCKCHIP_DEBUG"
        }

        CONFIG(release, debug|release) {
                CONFIG_NAME="ROCKCHIP_RELEASE"
        }
}


THIS_EXE_NAME="GPUPreBoot"

message ("$${THIS_EXE_NAME}: configuration is $${CONFIG_NAME}")
        PATH_TO_ROOT = "/data/dev/gpu-fts-nestle-2019"
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

QMAKE_CXXFLAGS += -Wno-deprecated-copy
CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}

SOURCES += \
    ../../src/GPUPreBoot/main.cpp

HEADERS += \
    ../../src/GPUPreBoot/main.h
