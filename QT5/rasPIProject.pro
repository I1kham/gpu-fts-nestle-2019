TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	rheaAlipayChina\
	SocketBridge \
	rheaAppLib \
        rasPI


# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
