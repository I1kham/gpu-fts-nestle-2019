TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	SocketBridge \
	rheaAppLib \
	GPU\
	rheaAlipayChina

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
