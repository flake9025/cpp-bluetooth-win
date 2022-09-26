#include "pch.h"
#include "BLEDevice.h"

BLEDevice::BLEDevice()
		: mp_di(0)
		, mp_bled(0)
{
}

BLEDevice::~BLEDevice()
{
	{
		std::map < std::wstring, BLEDeviceService*>::iterator l_ptr, l_end;
		for (l_end = m_uuid_to_service.end(), l_ptr = m_uuid_to_service.begin(); l_ptr != l_end; ++l_ptr)
		{
			delete l_ptr->second;
		}
	}
	if (mp_bled)
	{
		// mp_bled->Close();
		delete mp_bled;
	}
	delete mp_di;
}

DeviceInformation* BLEDevice::getDeviceInformation()
{
    return mp_di;
}

void BLEDevice::setDeviceInformation(DeviceInformation* di)
{
    mp_di = di;
}

BluetoothLEDevice* BLEDevice::getBLEDevice()
{
    return mp_bled;
}

void BLEDevice::setBLEDevice(BluetoothLEDevice* bled)
{
	mp_bled = bled;
}

std::map < std::wstring, BLEDeviceService*> BLEDevice::getUuidToService()
{
    return m_uuid_to_service;
}

void BLEDevice::setUuidToService(std::map < std::wstring, BLEDeviceService*> uuid_to_service)
{
	m_uuid_to_service = uuid_to_service;
}