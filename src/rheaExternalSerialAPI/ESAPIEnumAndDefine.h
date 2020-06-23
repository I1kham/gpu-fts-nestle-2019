#ifndef _ESAPIEnumAndDefine_h_
#define _ESAPIEnumAndDefine_h_

namespace esapi
{
	enum eExternalModuleType
	{
		eExternalModuleType_unknown			= 0x00,
		eExternalModuleType_rasPI_wifi_REST = 0x01
	};

	enum eGPUType
	{
		eGPUType_unknown	= 0x00,
		eGPUType_TS			= 0x01,
		eGPUType_TP			= 0x02
	};

} // namespace esapi

#endif // _ESAPIEnumAndDefine_h_
