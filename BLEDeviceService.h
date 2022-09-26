#pragma once
#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include "BlEUtils.h"

class BLEDeviceService
{
private:
	GattDeviceService* mp_ds;
	std::map < std::wstring, GattCharacteristic*> m_uuid_to_characteristic;
public:
    BLEDeviceService();
    ~BLEDeviceService();

    GattDeviceService* getGattDeviceService();
    void setGattDeviceService(GattDeviceService* ds);

    std::map < std::wstring, GattCharacteristic*> getUuidToCharacteristic();
    void setUuidToCharacteristic(std::map < std::wstring, GattCharacteristic*> uuid_to_characteristic);
};

#endif
