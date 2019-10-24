TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	SocketBridge \
	rheaAppLib \
	GPU

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
