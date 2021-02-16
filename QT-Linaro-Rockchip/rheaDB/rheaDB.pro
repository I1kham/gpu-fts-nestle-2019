TEMPLATE = lib
QT       -= core gui
CONFIG += staticlib
CONFIG += create_prl link_prl


#direttive specifiche per quando compilo per Rockchip
contains(DEFINES, PLATFORM_ROCKCHIP) {
        CONFIG(debug, debug|release) {
                CONFIG_NAME="ROCKCHIP_DEBUG"
        }

        CONFIG(release, debug|release) {
                CONFIG_NAME="ROCKCHIP_RELEASE"
        }
}

THIS_LIBRARY_NAME="rheaDB"

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
        QMAKE_CXXFLAGS += -Wno-zero-as-null-pointer-constant
        QMAKE_CXXFLAGS += -Wno-unused-parameter
        QMAKE_CXXFLAGS += -Wno-unused-variable
        QMAKE_CFLAGS += -Wno-implicit-fallthrough
        QMAKE_CFLAGS += -Wno-maybe-uninitialized
        CONFIG += optimize_full
}
LIBS += -ldl

SOURCES += \
    ../../src/rheaDB/SQLite3/SQLInterface_SQLite.cpp \
    ../../src/rheaDB/SQLRst.cpp \
    ../../src/rheaDB/SQLite3/sqlite3.c



HEADERS += \
    ../../src/rheaDB/SQLite3/SQLInterface_SQLite.h \
    ../../src/rheaDB/SQLite3/sqlite3.h \
    ../../src/rheaDB/SQLInterface.h



unix {
    target.path = /usr/lib
    INSTALLS += target
}
