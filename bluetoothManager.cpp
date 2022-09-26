#include "pch.h"
#include "BluetoothManager.h"

Discovery* Discovery::smp_dc = NULL;

/********************************************************************************************************
* JNI Functions
*********************************************************************************************************/

JNIEXPORT jobjectArray JNICALL Java_fr_vvlabs_bluetooth_WindowsBluetoothManager_getDevices
(JNIEnv* ap_jenv, jclass)
{
	Discovery* lp_dc = Discovery::getDiscovery();
	lp_dc->start_discovery(ap_jenv);

	if (lp_dc->wait_for_discovery_complete())
	{
		return lp_dc->GetBLEDevices(ap_jenv);
	}
	else
	{
		return NULL;
	}
}

JNIEXPORT jstring JNICALL Java_fr_vvlabs_bluetooth_WindowsBluetoothManager_getDeviceName
(JNIEnv* ap_jenv, jclass, jstring a_id)
{
	std::wstring l_id = Java_To_WStr(ap_jenv, a_id);
	std::wstring l_name = Discovery::getDiscovery()->getName(l_id);

	jstring l_str;
	l_str = ap_jenv->NewString((jchar*)l_name.c_str(), (jsize)l_name.length());

	return l_str;
}

JNIEXPORT jobjectArray JNICALL Java_fr_vvlabs_bluetooth_WindowsBluetoothManager_getDeviceServices
(JNIEnv* ap_jenv, jclass, jstring a_id)
{
	Discovery* lp_dc = Discovery::getDiscovery();
	return lp_dc->GetBLEDeviceServices(ap_jenv, a_id);
}


JNIEXPORT jobjectArray JNICALL Java_fr_vvlabs_bluetooth_WindowsBluetoothManager_getDeviceServiceCharacteristics
(JNIEnv* ap_jenv, jclass, jstring a_id, jstring a_service)

{
	Discovery* lp_dc = Discovery::getDiscovery();
	return lp_dc->GetBLEServiceCharacteristics(ap_jenv, a_id, a_service);
}

JNIEXPORT jbyteArray JNICALL Java_fr_vvlabs_bluetooth_WindowsBluetoothManager_getCharacteristicValue
(JNIEnv* ap_jenv, jclass, jstring a_id, jstring a_service, jstring a_characteristic)
{
	Discovery* lp_dc = Discovery::getDiscovery();
	DataReader l_dr{ nullptr };
	if (lp_dc->GetBLECharacteristicValue(ap_jenv, a_id, a_service, a_characteristic, l_dr))
	{
		std::vector<byte> l_bytes;
		int l_len = l_dr.UnconsumedBufferLength();
		jbyte* lp_jbytes = ::new jbyte[l_len];
		jbyteArray l_ba = ap_jenv->NewByteArray(l_len);
		for (int i = 0; i < l_len; ++i)
			lp_jbytes[i] = l_dr.ReadByte();
		ap_jenv->SetByteArrayRegion(l_ba, 0, l_len, lp_jbytes);
		delete lp_jbytes;
		return l_ba;
	}
	else
		return NULL;
}

JNIEXPORT jboolean JNICALL Java_javelin_javelin_setBLECharacteristicValue
(JNIEnv* ap_jenv, jclass, jstring a_id, jstring a_service, jstring a_characteristic, jbyteArray a_value)
{
	Discovery* lp_dc = Discovery::getDiscovery();
	jbyte* lp_jbytes;
	lp_jbytes = ap_jenv->GetByteArrayElements(a_value, 0);
	DataWriter l_dw;
	l_dw.ByteOrder(ByteOrder::LittleEndian);
	for (int i = 0; i < ap_jenv->GetArrayLength(a_value); ++i)
		l_dw.WriteByte(lp_jbytes[i]);
	ap_jenv->ReleaseByteArrayElements(a_value, lp_jbytes, 0);
	//MARK;
	return lp_dc->SetBLECharacteristicValue(ap_jenv, a_id, a_service, a_characteristic, l_dw);
}

/********************************************************************************************************
* DLL Management
*********************************************************************************************************/

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Discovery::releaseDiscovery();
		break;
	}
	return TRUE;
}
