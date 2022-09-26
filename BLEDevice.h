#pragma once
#ifndef BLE_DEVICE_H
#define BLE_DEVICE_H

#include "BLEDeviceService.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

class BLEDevice
{
private:
    DeviceInformation* mp_di;
    BluetoothLEDevice* mp_bled;
    std::map < std::wstring, BLEDeviceService*> m_uuid_to_service;
public:
    BLEDevice();
    ~BLEDevice();

    DeviceInformation* getDeviceInformation();
    void setDeviceInformation(DeviceInformation* di);

    BluetoothLEDevice* getBLEDevice();
    void setBLEDevice(BluetoothLEDevice* bled);

    std::map < std::wstring, BLEDeviceService*> getUuidToService();
    void BLEDevice::setUuidToService(std::map < std::wstring, BLEDeviceService*> uuid_to_service);
};

#endif

