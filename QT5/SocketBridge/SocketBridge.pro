#-------------------------------------------------
#
# Project created by QtCreator 2019-10-14T10:24:49
#
#-------------------------------------------------

QT       -= core gui

TEMPLATE = lib
CONFIG += staticlib
CONFIG += create_prl link_prl


#direttive specifiche per quando compilo per yocto embedded
contains(DEFINES, PLATFORM_YOCTO_EMBEDDED) {
	CONFIG(debug, debug|release) {
		CONFIG_NAME="EMBEDDED_DEBUG"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="EMBEDDED_RELEASE"
	}
}


#direttive specifiche per quando compilo su ubuntu desktop
contains(DEFINES, PLATFORM_UBUNTU_DESKTOP) {

	CONFIG(debug, debug|release) {
		CONFIG_NAME="DESKTOP64_DEBUG"
	}

	CONFIG(release, debug|release) {
		CONFIG_NAME="DESKTOP64_RELEASE"
	}
}

#direttive specifiche per quando compilo su rasPI
contains(DEFINES, PLATFORM_RASPI) {

        CONFIG(debug, debug|release) {
                CONFIG_NAME="RASPI_DEBUG"
        }

        CONFIG(release, debug|release) {
                CONFIG_NAME="RASPI_RELEASE"
        }
}

THIS_LIBRARY_NAME="SocketBridge"

message ("$${THIS_LIBRARY_NAME}: configuration is $${CONFIG_NAME}")
	PATH_TO_ROOT = "../../../.."
	PATH_TO_BIN = "$${PATH_TO_ROOT}/bin"
	PATH_TO_SRC = "$${PATH_TO_ROOT}/src"
	PATH_TO_LIB = "$${PATH_TO_ROOT}/lib"
	TARGET = "$${PATH_TO_LIB}/$${CONFIG_NAME}_$${THIS_LIBRARY_NAME}"

#depends on rheaAlipayChina libray
LIBRARY_NAME="rheaAlipayChina"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		unix:!macx: PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

#depends on rheaDB libray
LIBRARY_NAME="rheaDB"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		unix:!macx: LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		unix:!macx: PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

#depends on rheaCommonLib libray
LIBRARY_NAME="rheaCommonLib"
		FULL_LIBRARY_NAME = "$${CONFIG_NAME}_$${LIBRARY_NAME}"
		INCLUDEPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		DEPENDPATH += $${PATH_TO_SRC}/$${LIBRARY_NAME}
		LIBS += -L$${PATH_TO_LIB}/ -l$${FULL_LIBRARY_NAME}
		PRE_TARGETDEPS += "$${PATH_TO_LIB}/lib$${FULL_LIBRARY_NAME}.a"

CONFIG(release, debug|release) {
	QMAKE_CXXFLAGS += -O2
	QMAKE_CFLAGS += -O2
	CONFIG += optimize_full
	QMAKE_CXXFLAGS += -Wno-unused-parameter -Wno-unused-variable
}


SOURCES += \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_abort.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_activate.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_getConnDetail.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_getQR.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_isOnline.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_M_MilkerVer.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x03_SanWashStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x04_SetDecounter.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x06_GetAllDecounterValues.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x07_GetTime.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x08_GetDate.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x09_SetTime.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0A_SetDate.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0B_StatoGruppo.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0C_AttivazioneMotore.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0E_StartImpulseCalc.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0F_SetCalibFactor.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x10_GetPosizioneMacina.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x13_NomiLingueCPU.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x16_ResetEVA.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x18_GetOFFList.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1C_StartModemTest.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1D_ResetEVATotals.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_setLastUsedLangForProgMenu.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_T_VMCDataFileTimestamp.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBC.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBCloseByPath.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBE.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBQ.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSDriveList.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSFileCopy.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSFileList.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqGetCurSelRunning.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqIsManualInstalled.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqMachineTypeAndModel.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelAvailability.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelPrices.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTaskSpawn.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTaskStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTestSelection.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_AliChina_onlineStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x03_CPUSanWashingStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x04_SetDecounter.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x06_GetAllDecounters.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x10_getAperturaVGrind.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x11_SetMotoreMacina.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x14_StartDisintallation.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x15_RecalcFasciaOrariaFV.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_T_VMCDataFileTimestamp.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_V_StartSelAlreadyPaid.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqBtnPressed.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqClientList.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUbtnProgPressed.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUExtendedConfigInfo.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUIniParam.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUMessage.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUProgrammingCmd.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCreditUpdated.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqDataAudit.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqEnterDA3SyncStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqPartialVMCDataFile.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelAvailability.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelPrices.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSetAperturaVGrind.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStartSel.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStopSel.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFile.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqWriteLocalVMCDataFile.cpp \
    ../../src/SocketBridge/CmdHandler.cpp \
    ../../src/SocketBridge/CmdHandler_ajaxReq.cpp \
    ../../src/SocketBridge/CmdHandler_eventReq.cpp \
    ../../src/SocketBridge/DBList.cpp \
    ../../src/SocketBridge/IdentifiedClientList.cpp \
    ../../src/SocketBridge/SocketBridge.cpp \
    ../../src/SocketBridge/SocketBridgeFileT.cpp \
    ../../src/SocketBridge/SocketBridgeServer.cpp \
    ../../src/SocketBridge/SocketBridgeTask.cpp \
    ../../src/SocketBridge/SocketBridgeTaskFactory.cpp \
    ../../src/SocketBridge/SocketBridgeTaskStatus.cpp \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSRheaUnzip.cpp


HEADERS += \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_abort.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_activate.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_getConnDetail.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_getQR.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_AliChina_isOnline.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_M_MilkerVer.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x03_SanWashStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x04_SetDecounter.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x06_GetAllDecounterValues.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x07_GetTime.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x08_GetDate.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x09_SetTime.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0A_SetDate.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0B_StatoGruppo.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0C_AttivazioneMotore.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0E_StartImpulseCalc.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x0F_SetCalibFactor.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x10_GetPosizioneMacina.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x13_NomiLingueCPU.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x16_ResetEVA.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x18_GetOFFList.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1C_StartModemTest.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1D_ResetEVATotals.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_setLastUsedLangForProgMenu.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReq_T_VMCDataFileTimestamp.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBC.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBCloseByPath.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBE.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqDBQ.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSDriveList.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSFileCopy.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSFileList.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqGetCurSelRunning.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqIsManualInstalled.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqMachineTypeAndModel.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelAvailability.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqSelPrices.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTaskSpawn.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTaskStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqTestSelection.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_AliChina_onlineStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x03_CPUSanWashingStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x04_SetDecounter.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x06_GetAllDecounters.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x10_getAperturaVGrind.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x11_SetMotoreMacina.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x14_StartDisintallation.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_P0x15_RecalcFasciaOrariaFV.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_T_VMCDataFileTimestamp.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReq_V_StartSelAlreadyPaid.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqBtnPressed.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqClientList.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUbtnProgPressed.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUExtendedConfigInfo.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUIniParam.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUMessage.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUProgrammingCmd.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCPUStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqCreditUpdated.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqDataAudit.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqEnterDA3SyncStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqPartialVMCDataFile.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelAvailability.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelPrices.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSelStatus.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqSetAperturaVGrind.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStartSel.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqStopSel.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqVMCDataFile.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_eventReqWriteLocalVMCDataFile.h \
    ../../src/SocketBridge/CmdHandler.h \
    ../../src/SocketBridge/CmdHandler_ajaxReq.h \
    ../../src/SocketBridge/CmdHandler_eventReq.h \
    ../../src/SocketBridge/CommandHandlerList.h \
    ../../src/SocketBridge/DBList.h \
    ../../src/SocketBridge/IdentifiedClientList.h \
    ../../src/SocketBridge/SocketBridge.h \
    ../../src/SocketBridge/SocketBridgeEnumAndDefine.h \
    ../../src/SocketBridge/SocketBridgeFileT.h \
    ../../src/SocketBridge/SocketBridgeFileTEnumAndDefine.h \
    ../../src/SocketBridge/SocketBridgeServer.h \
    ../../src/SocketBridge/SocketBridgeTask.h \
    ../../src/SocketBridge/SocketBridgeTaskFactory.h \
    ../../src/SocketBridge/SocketBridgeVersion.h \
    ../../src/SocketBridge/CmdHandler/CmdHandler_ajaxReqFSRheaUnzip.h



unix {
    target.path = /usr/lib
    INSTALLS += target
}
