#-------------------------------------------------
#
# Project created by QtCreator 2021-03-18T14:39:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = GPUBoot
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11


#direttive specifiche per quando compilo per Rockchip
CONFIG(debug, debug|release) {
		CONFIG_NAME="ROCKCHIP_DEBUG"
}

CONFIG(release, debug|release) {
		CONFIG_NAME="ROCKCHIP_RELEASE"
}


THIS_EXE_NAME="GPUBoot"

message ("$${THIS_EXE_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_BIN}/$${CONFIG_NAME}_$${THIS_EXE_NAME}"


SOURCES += \
		../src/GPUBoot/main.cpp \
		../src/GPUBoot/mainwindow.cpp

HEADERS += \
		../src/GPUBoot/mainwindow.h \
		../src/GPUBoot/main.h

FORMS += \
		../src/GPUBoot/mainwindow.ui


