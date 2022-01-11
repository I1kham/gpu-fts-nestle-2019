TEMPLATE = lib
QT       -= core gui
CONFIG += staticlib
CONFIG += create_prl link_prl
CONFIG += c++14

#direttive specifiche per quando compilo per Rockchip
contains(DEFINES, PLATFORM_ROCKCHIP) {
        CONFIG(debug, debug|release) {
                CONFIG_NAME="ROCKCHIP_DEBUG"
        }

        CONFIG(release, debug|release) {
                CONFIG_NAME="ROCKCHIP_RELEASE"
        }
}

THIS_LIBRARY_NAME="rheaAppLib"

message ("$${THIS_LIBRARY_NAME}: configuration is $${CONFIG_NAME}")
        PATH_TO_ROOT = "/data/dev/gpu-fts-nestle-2019"
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

#depends on SocketBridge libray
LIBRARY_NAME="SocketBridge"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		unix:!macx: PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

QMAKE_CXXFLAGS += -Wno-deprecated-copy
CONFIG(release, debug|release) {
        QMAKE_CXXFLAGS += -O2
        QMAKE_CXXFLAGS += -Wno-zero-as-null-pointer-constant
        QMAKE_CXXFLAGS += -Wno-unused-parameter
        QMAKE_CXXFLAGS += -Wno-unused-variable
        QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
}

SOURCES += \
    ../../src/rheaAppLib/rheaApp.cpp \
    ../../src/rheaAppLib/rheaAppFileTransfer.cpp \
    ../../src/rheaAppLib/rheaAppUtils.cpp

HEADERS += \
    ../../src/rheaAppLib/rheaApp.h \
    ../../src/rheaAppLib/rheaAppEnumAndDefine.h \
    ../../src/rheaAppLib/rheaAppFileTransfer.h \
    ../../src/rheaAppLib/rheaAppUtils.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
