TEMPLATE = subdirs

SUBDIRS = \
	rheaCommonLib\   
	CPUBridge \
	rheaDB \
	SocketBridge \
	SMU

# build the project sequentially as listed in SUBDIRS !
CONFIG += ordered
