#pragma once
#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <concrt.h>
#include <queue>
#include "BLEDevice.h"

using namespace winrt;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Windows::Devices::Bluetooth;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage::Streams;

class Discovery
{
private:
	static Discovery* smp_dc;

	concurrency::critical_section m_lock_std;
	std::map<std::wstring, BLEDevice*> m_id_to_bd;
	concurrency::event m_discovery_complete;

	Windows::Devices::Enumeration::DeviceWatcher m_deviceWatcher{ nullptr };
	event_token m_deviceWatcherAddedToken;
	event_token m_deviceWatcherUpdatedToken;
	event_token m_deviceWatcherRemovedToken;
	event_token m_deviceWatcherEnumerationCompletedToken;
	event_token m_deviceWatcherStoppedToken;

	std::map < std::wstring, event_token> m_uuid_to_notification_token;
	std::map < std::wstring, concurrency::event> m_uuid_to_notification_event;
	std::map < std::wstring, GattCharacteristic*> m_uuid_to_notification_characteristic;
	std::map < std::wstring, std::queue<std::string*> > m_uuid_to_changed_value;
public:
	Discovery();
	~Discovery();

	static void releaseDiscovery();

	void add_di(std::wstring& ar_id, Windows::Devices::Enumeration::DeviceInformation ar_di);
	void update_di(std::wstring& ar_id, Windows::Devices::Enumeration::DeviceInformationUpdate ar_diu);
	void remove_di(std::wstring& ar_id);
	std::wstring getName(std::wstring& ar_id);
	static void display_di(Windows::Devices::Enumeration::DeviceInformation* ap_di);

	void start_discovery(JNIEnv* ap_jenv);
	bool wait_for_discovery_complete();
	void clear_discovery_complete();
	void signal_discovery_complete();
	static Discovery* getDiscovery();

	jobjectArray GetBLEDevices(JNIEnv* ap_jenv);
	IAsyncOperation<bool> GetBLEDeviceUWP(Windows::Devices::Bluetooth::BluetoothLEDevice& ar_bled, hstring& ar_id);
	bool GetBLEDevice(BLEDevice* ap_bd);
	
	IAsyncOperation<bool> GetGattServicesUWP(Windows::Devices::Bluetooth::BluetoothLEDevice* ap_bled, GattDeviceServicesResult& ar_gdsr);
	bool GetGattServices(BLEDevice* ap_bd, GattDeviceServicesResult& ar_gdsr);
	jobjectArray GetBLEDeviceServices(JNIEnv* ap_jenv, jstring a_id);
	
	IAsyncOperation<bool> GetGattCharacteristicsUWP(GattDeviceService* ap_gds, GattCharacteristicsResult& ar_gcr);
	bool GetGattCharacteristics(BLEDeviceService* ap_gds, GattCharacteristicsResult& ar_gcr, std::wstring& ar_service);
	jobjectArray GetBLEServiceCharacteristics(JNIEnv* ap_jenv, jstring a_id, jstring a_service);
	
	IAsyncOperation<bool> GetGattCharacteristicValueUWP(GattCharacteristic* ap_gc, GattReadResult& ar_grr, DataReader& ar_dr);
	bool GetGattCharacteristicValue(GattCharacteristic* ap_gc, GattReadResult& ar_grr, DataReader& ar_dr);
	
	bool GetBLECharacteristicValue(JNIEnv* ap_jenv, jstring a_id, jstring a_service, jstring a_characteristic, DataReader& ar_dr);
	IAsyncOperation<bool> SetGattCharacteristicValueUWP(GattCharacteristic* ap_gc, GattWriteResult& ar_gwr, DataWriter& ar_dw);

	bool SetGattCharacteristicValue(GattCharacteristic* ap_gc, GattWriteResult& ar_gwr, DataWriter& ar_dw);
	bool SetBLECharacteristicValue(JNIEnv* ap_jenv, jstring a_id, jstring a_service, jstring a_characteristic, DataWriter& ar_dw);

	static void DeviceWatcher_Added(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformation deviceInfo);
	static void DeviceWatcher_Updated(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate);
	static void DeviceWatcher_Removed(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate);
	static void DeviceWatcher_EnumerationCompleted(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&);
	static void DeviceWatcher_Stopped(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&);
	
	static void Characteristic_ValueChanged(GattCharacteristic const& ar_gc, GattValueChangedEventArgs a_args);
	void signal_characteristic_valuechanged(std::wstring& ar_gc_uuid, DataReader& ar_dr);

	IAsyncOperation<bool> WriteClientCharacteristicConfigurationDescriptorAsync(GattCharacteristic* ap_gc, GattClientCharacteristicConfigurationDescriptorValue a_cccdValue);

};

#endif