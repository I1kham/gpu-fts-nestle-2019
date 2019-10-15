TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	SocketBridge \
	GPU

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
