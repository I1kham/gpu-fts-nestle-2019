TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	rheaAlipayChina\
	SocketBridge \
	rheaAppLib \
	rheaExternalSerialAPI\
	GPU


# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
