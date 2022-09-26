#include "pch.h"
#include "BLEDeviceService.h"

using namespace winrt;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

BLEDeviceService::BLEDeviceService() 
	: mp_ds(0)
{
}

BLEDeviceService::~BLEDeviceService()
{
	{
		std::map < std::wstring, GattCharacteristic*>::iterator l_ptr, l_end;
		for (l_end = m_uuid_to_characteristic.end(), l_ptr = m_uuid_to_characteristic.begin(); l_ptr != l_end; ++l_ptr)
		{
			delete l_ptr->second;
		}
	}
	mp_ds->Session().MaintainConnection(false);
	//delete mp_ds;
}

GattDeviceService* BLEDeviceService::getGattDeviceService()
{
	return mp_ds;
}

void BLEDeviceService::setGattDeviceService(GattDeviceService* ds)
{
	mp_ds = ds;
}

std::map < std::wstring, GattCharacteristic*> BLEDeviceService::getUuidToCharacteristic()
{
	return m_uuid_to_characteristic;
}

void BLEDeviceService::setUuidToCharacteristic(std::map < std::wstring, GattCharacteristic*> uuid_to_characteristic)
{
	m_uuid_to_characteristic = uuid_to_characteristic;
}
