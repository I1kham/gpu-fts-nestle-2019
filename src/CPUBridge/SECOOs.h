#ifndef _SECOOs_h_
#define _SECOOs_h_
#include "CPUBridgeEnumAndDefine.h"
#include "../rheaCommonLib/string/rheaUTF8String.h"


namespace secoos
{
	void		getMACAddress (u8 *out, u32 sizeof_out);
	void		getLANAddress (u8 *out, u32 sizeof_out);

	namespace modemLTE
	{
		bool			isEnabled();
		void			enable (bool b);
	}

	namespace wifi
	{
		cpubridge::eWifiMode		getMode();

		void						setModeHotspot();
		void						hotspot_getSSID (u8 *out, u32 sizeof_out);

		void						setModeConnectTo (const u8 *ssid, const u8 *pwd);
		bool						connectTo_isConnected();
		void						connectTo_getIP (u8 *out, u32 sizeof_out);
		void						connectTo_getSSID (u8 *out, u32 sizeof_out);
		void						connectTo_getPwd (u8 *out, u32 sizeof_out);

		u8*							getListOfAvailableSSID (rhea::Allocator *allocator, u32 *out_n);
									//ritorna una stringa composta da un elenco di nomi di SSID separati dal carattere '|'
									//La stringa viene allocata usando [allocator].
									//Ritorna in [out_n] il num di SSID inserite nella stringa di ritorno
									//Attenzione: può ritorna NULL nel qual caso anche [out_n] == 0
	}

} //namespace secoos


#endif //_SECOOs_h_
