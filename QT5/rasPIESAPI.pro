TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	rheaExternalSerialAPI \
        rasPISerial


# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
