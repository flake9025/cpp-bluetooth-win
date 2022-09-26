#include "pch.h"
#include "BLEUtils.h"

/*
* Java String to C++ Wide String
*/
std::wstring Java_To_WStr(JNIEnv *env, jstring string)
{
	std::wstring value;
	const jchar* raw = env->GetStringChars(string, 0);
	jsize len = env->GetStringLength(string);
	value.assign(raw, raw + len);
	env->ReleaseStringChars(string, raw);
	return value;
}

/*
* UUID to Wide String
* Used with BLE Services and GATT Characteristics
* ex: 00001801-0000-1000-8000-00805F9B34FB
*/
void guidTowstring(guid& ar_guid, std::wstring& ar_wstring)
{
	std::wstringstream l_uuid;

	l_uuid.fill('0');
	// Uppercase and Hexadecimal
	l_uuid << std::uppercase << std::hex
		<< std::setw(8)
		<< ar_guid.Data1
		<< '-' << std::setw(4) << ar_guid.Data2
		<< '-' << std::setw(4) << ar_guid.Data3
		<< '-' << std::setw(2) << (unsigned int)(ar_guid.Data4[0])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[1])
		<< '-' << std::setw(2) << (unsigned int)(ar_guid.Data4[2])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[3])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[4])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[5])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[6])
		<< std::setw(2)
		<< (unsigned int)(ar_guid.Data4[7])
		<< std::flush;

	ar_wstring = l_uuid.str();
}

/*
* GATT Enum to Wide String
*/
std::wstring gcs_to_wstring(GattCommunicationStatus status)
{
	switch (status)
	{
	case GattCommunicationStatus::Success: return std::wstring(L"Success");
	case GattCommunicationStatus::Unreachable: return std::wstring(L"Unreachable");
	case GattCommunicationStatus::ProtocolError: return std::wstring(L"ProtocolError");
	case GattCommunicationStatus::AccessDenied: return std::wstring(L"AccessDenied");
	default: return std::wstring(L"Unknown");
	}
}