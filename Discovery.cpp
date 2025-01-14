#include "pch.h"
#include "Discovery.h"

Discovery::Discovery()
		: m_lock_std()
		, m_id_to_bd()
		, m_discovery_complete()
{
	std::cout << "Windows Bluetooth Manager : starting devices discovery..." << std::endl;
	m_discovery_complete.reset();
};

Discovery::~Discovery()
{
	//concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	if (m_deviceWatcher != nullptr)
	{
		//MARK;
		// Unregister the event handlers.
		m_deviceWatcher.Added(m_deviceWatcherAddedToken);
		m_deviceWatcher.Updated(m_deviceWatcherUpdatedToken);
		m_deviceWatcher.Removed(m_deviceWatcherRemovedToken);
		m_deviceWatcher.EnumerationCompleted(m_deviceWatcherEnumerationCompletedToken);
		m_deviceWatcher.Stopped(m_deviceWatcherStoppedToken);

		// Stop the watcher.
		m_deviceWatcher.Stop();
		m_deviceWatcher = nullptr;
	}
	std::cout << "Windows Bluetooth Manager : end discovery." << std::endl;
	return;
#if 0
	{
		//MARK;
		std::map < std::wstring, event_token>::iterator l_ptr, l_end;
		for (l_end = m_uuid_to_notification_token.end(), l_ptr = m_uuid_to_notification_token.begin(); l_ptr != l_end; ++l_ptr)
		{
			GattCharacteristic* lp_gc = m_uuid_to_notification_characteristic[l_ptr->first];
			if (lp_gc)
			{
				lp_gc->ValueChanged(l_ptr->second);
			}
		}
	}

	{
		//MARK;
		std::map<std::wstring, BLEDevice*>::iterator l_end, l_ptr;
		for (l_end = m_id_to_bd.end(), l_ptr = m_id_to_bd.begin(); l_ptr != l_end; ++l_ptr)
		{
			delete l_ptr->second;
		}
	}

	{
		//MARK;
		std::map < std::wstring, std::queue<std::string*> >::iterator l_ptr, l_end;
		for (l_end = m_uuid_to_changed_value.end(), l_ptr = m_uuid_to_changed_value.begin(); l_ptr != l_end; ++l_ptr)
		{
			while (!l_ptr->second.empty())
			{
				std::string* lp_bytes = l_ptr->second.front();
				l_ptr->second.pop();
				delete lp_bytes;
			}
		}
	}

	std::cout << "Windows Bluetooth Manager : end" << std::endl;
#endif
}

void Discovery::releaseDiscovery()
{
	//MARK;
	if (smp_dc)
	{
		//MARK;
		delete smp_dc;
	}
}

void Discovery::add_di(std::wstring& ar_id, Windows::Devices::Enumeration::DeviceInformation ar_di)
{
	//MARK;
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	BLEDevice* lp_bd = ::new BLEDevice();
	lp_bd->setDeviceInformation(::new Windows::Devices::Enumeration::DeviceInformation(ar_di));
	m_id_to_bd[ar_id] = lp_bd;
	//DEBUG_TRACE("-- Adding");
	display_di(lp_bd->getDeviceInformation());
}

void Discovery::update_di(std::wstring& ar_id, Windows::Devices::Enumeration::DeviceInformationUpdate ar_diu)
{
	//MARK;
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	m_id_to_bd[ar_id]->getDeviceInformation()->Update(ar_diu);
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[ar_id]->getDeviceInformation();
	//DEBUG_TRACE("-- Updating");
	display_di(lp_di);
}

void Discovery::remove_di(std::wstring& ar_id)
{
	//MARK;
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[ar_id]->getDeviceInformation();
	//            DEBUG_TRACE("-- Deleting");
	//            display_di(lp_di);
}

std::wstring Discovery::getName(std::wstring& ar_id)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	if (m_id_to_bd.find(ar_id) == m_id_to_bd.end()) return std::wstring();
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[ar_id]->getDeviceInformation();
	if (!lp_di) return std::wstring();
	std::wstring l_name(lp_di->Name().c_str());
	return l_name;
}

void Discovery::display_di(Windows::Devices::Enumeration::DeviceInformation* ap_di)
{
	std::wstring l_id(ap_di->Id().c_str());
	std::wstring l_name(ap_di->Name().c_str());
	//DEBUG_TRACE("Id: " << l_id << " Name: " << l_name)
}

void Discovery::start_discovery(JNIEnv* ap_jenv)
{
	clear_discovery_complete();

	auto requestedProperties = single_threaded_vector<hstring>({ L"System.Devices.Aep.DeviceAddress", L"System.Devices.Aep.IsConnected", L"System.Devices.Aep.Bluetooth.Le.IsConnectable" });

	// BT_Code: Query for Bluetooth and BLE devices
	// https://learn.microsoft.com/fr-fr/windows/uwp/devices-sensors/aep-service-class-ids

	//hstring aqsAllBluetoothLEDevices = L"(System.Devices.Aep.ProtocolId:=\"{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}\" OR System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";
	hstring aqsAllBluetoothLEDevices = L"(System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\")";

	m_deviceWatcher =
		Windows::Devices::Enumeration::DeviceInformation::CreateWatcher(
			aqsAllBluetoothLEDevices,
			requestedProperties,
			DeviceInformationKind::AssociationEndpoint);

	// Register event handlers before starting the watcher.
	m_deviceWatcherAddedToken = m_deviceWatcher.Added(&Discovery::DeviceWatcher_Added);
	m_deviceWatcherUpdatedToken = m_deviceWatcher.Updated(&Discovery::DeviceWatcher_Updated);
	m_deviceWatcherRemovedToken = m_deviceWatcher.Removed(&Discovery::DeviceWatcher_Removed);
	m_deviceWatcherEnumerationCompletedToken = m_deviceWatcher.EnumerationCompleted(&Discovery::DeviceWatcher_EnumerationCompleted);
	m_deviceWatcherStoppedToken = m_deviceWatcher.Stopped(&Discovery::DeviceWatcher_Stopped);

	// Start the watcher. Active enumeration is limited to approximately 10 seconds.
	// This limits power usage and reduces interference with other Bluetooth activities.
	// To monitor for the presence of Bluetooth LE devices for an extended period,
	// use the BluetoothLEAdvertisementWatcher runtime class. See the BluetoothAdvertisement
	// sample for an example.
	m_deviceWatcher.Start();
}

bool Discovery::wait_for_discovery_complete()
{
	if (m_discovery_complete.wait(10000) != 0)
		return false;
	else
		return true;
}

void Discovery::clear_discovery_complete()
{
	m_discovery_complete.reset();
}

void Discovery::signal_discovery_complete()
{
	m_discovery_complete.set();
}

Discovery* Discovery::getDiscovery()
{
	if (!smp_dc)
	{
		smp_dc = new Discovery();
	}
	return smp_dc;
}

jobjectArray Discovery::GetBLEDevices(JNIEnv* ap_jenv)
{
	std::map<std::wstring, BLEDevice*>::iterator l_end, l_ptr;
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);

	jstring l_str;
	jobjectArray l_ids = 0;
	jsize l_len = (jsize)m_id_to_bd.size();

	l_ids = ap_jenv->NewObjectArray(l_len, ap_jenv->FindClass("java/lang/String"), 0);

	int l_count = 0;
	for (l_end = m_id_to_bd.end(), l_ptr = m_id_to_bd.begin(); l_ptr != l_end; ++l_ptr)
	{
		std::wstring l_id((l_ptr->second)->getDeviceInformation()->Id().c_str());
		l_str = ap_jenv->NewString((jchar*)l_id.c_str(), (jsize)l_id.length());
		ap_jenv->SetObjectArrayElement(l_ids, l_count++, l_str);
	}

	return l_ids;
}

IAsyncOperation<bool> Discovery::GetBLEDeviceUWP(Windows::Devices::Bluetooth::BluetoothLEDevice& ar_bled, hstring& ar_id)
{
	Windows::Devices::Bluetooth::BluetoothLEDevice l_bld = co_await BluetoothLEDevice::FromIdAsync(ar_id);
	ar_bled = l_bld;
	co_return true;
}

bool Discovery::GetBLEDevice(BLEDevice* ap_bd)
{
	Windows::Devices::Enumeration::DeviceInformation* lp_di = ap_bd->getDeviceInformation();
	if (!lp_di) return false;
	if (ap_bd->getBLEDevice()) return true;

	Windows::Devices::Bluetooth::BluetoothLEDevice l_bld{ nullptr };
	IAsyncOperation<bool> l_ret = GetBLEDeviceUWP(l_bld, lp_di->Id());
	if (!l_ret.get())
	{
		std::wcerr << "Fail to get BLE device" << std::endl;
		return false;
	}

	ap_bd->setBLEDevice(::new Windows::Devices::Bluetooth::BluetoothLEDevice(l_bld));

	return true;
}

IAsyncOperation<bool> Discovery::GetGattServicesUWP(Windows::Devices::Bluetooth::BluetoothLEDevice* ap_bled,
	GattDeviceServicesResult& ar_gdsr)
{
	ar_gdsr = co_await ap_bled->GetGattServicesAsync(BluetoothCacheMode::Uncached);
	if (ar_gdsr.Status() != GattCommunicationStatus::Success)
	{
		co_return false;
	}
	else
	{
		co_return true;
	}
}

bool Discovery::GetGattServices(BLEDevice* ap_bd, GattDeviceServicesResult& ar_gdsr)
{
	IAsyncOperation<bool> l_ret = GetGattServicesUWP(ap_bd->getBLEDevice(), ar_gdsr);
	if (!l_ret.get())
	{
		std::wcerr << "Fail to get GATT services" << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

jobjectArray Discovery::GetBLEDeviceServices(JNIEnv* ap_jenv, jstring a_id)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);

	std::wstring l_id = Java_To_WStr(ap_jenv, a_id);
	if (m_id_to_bd.find(l_id) == m_id_to_bd.end()) return NULL;
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[l_id]->getDeviceInformation();
	if (!lp_di) return NULL;
	Windows::Devices::Bluetooth::BluetoothLEDevice l_bld{ nullptr };

	if (m_id_to_bd[l_id]->getBLEDevice())
	{
		m_id_to_bd[l_id]->getBLEDevice()->Close();
		delete m_id_to_bd[l_id]->getBLEDevice();
		m_id_to_bd[l_id]->setBLEDevice(NULL);
	}
	bool l_ret = GetBLEDevice(m_id_to_bd[l_id]);
	if (l_ret != true) return NULL;
	GattDeviceServicesResult l_gdsr{ nullptr };
	l_ret = GetGattServices(m_id_to_bd[l_id], l_gdsr);
	if (l_ret != true) {
		std::wcerr << "Failed to get BLE services" << std::endl;
		return NULL;
	}

	jstring l_str;
	jobjectArray l_svcs = 0;
	jsize l_len = (jsize)l_gdsr.Services().Size();

	l_svcs = ap_jenv->NewObjectArray(l_len, ap_jenv->FindClass("java/lang/String"), 0);
	for (unsigned int i = 0; i < l_gdsr.Services().Size(); ++i)
	{
		std::wstring l_uuid;
		guidTowstring(l_gdsr.Services().GetAt(i).Uuid(), l_uuid);
		if (!m_id_to_bd[l_id]->getUuidToService()[l_uuid])
		{
			m_id_to_bd[l_id]->getUuidToService()[l_uuid] = new BLEDeviceService();
		}
		GattDeviceService* lp_gds = ::new GattDeviceService(l_gdsr.Services().GetAt(i));
		m_id_to_bd[l_id]->getUuidToService()[l_uuid]->setGattDeviceService(lp_gds);
		if (lp_gds->Session().CanMaintainConnection())
		{
			lp_gds->Session().MaintainConnection(true);
		}
		l_str = ap_jenv->NewString((jchar*)l_uuid.c_str(), (jsize)l_uuid.length());
		ap_jenv->SetObjectArrayElement(l_svcs, i, l_str);
	}
	return l_svcs;
}

IAsyncOperation<bool> Discovery::GetGattCharacteristicsUWP(GattDeviceService* ap_gds,
	GattCharacteristicsResult& ar_gcr)
{
	ar_gcr = co_await ap_gds->GetCharacteristicsAsync(BluetoothCacheMode::Uncached);
	if (ar_gcr.Status() != GattCommunicationStatus::Success)
	{
		co_return false;
	}
	else
	{
		co_return true;
	}
}

bool Discovery::GetGattCharacteristics(BLEDeviceService* ap_gds, GattCharacteristicsResult& ar_gcr, std::wstring& ar_service)
{
	IAsyncOperation<bool> l_ret = GetGattCharacteristicsUWP(ap_gds->getGattDeviceService(), ar_gcr);

	if (!l_ret.get())
	{
		std::wcerr << "Failed to get gatt svcs characteristics" << std::endl;
		return false;
	}
	else
	{
		return true;
	}
}

jobjectArray Discovery::GetBLEServiceCharacteristics(JNIEnv* ap_jenv, jstring a_id, jstring a_service)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);

	std::wstring l_id = Java_To_WStr(ap_jenv, a_id);
	std::wstring l_service = Java_To_WStr(ap_jenv, a_service);

	if (m_id_to_bd.find(l_id) == m_id_to_bd.end()) return NULL;

	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[l_id]->getDeviceInformation();
	if (!lp_di) return NULL;
	if (m_id_to_bd[l_id]->getUuidToService().find(l_service) == m_id_to_bd[l_id]->getUuidToService().end()) return NULL;

	GattCharacteristicsResult l_gcr{ nullptr };
	bool l_ret = GetGattCharacteristics(m_id_to_bd[l_id]->getUuidToService()[l_service], l_gcr, l_service);
	if (l_ret != true) {
		std::wcerr << "Fail to get BLE Service Characteristics" << std::endl;
		return NULL;
	}

	jstring l_str;
	jobjectArray l_characteristics = 0;
	jsize l_len = (jsize)l_gcr.Characteristics().Size();

	l_characteristics = ap_jenv->NewObjectArray(l_len, ap_jenv->FindClass("java/lang/String"), 0);

	for (unsigned int i = 0; i < l_gcr.Characteristics().Size(); ++i)
	{
		std::wstring l_uuid;
		guidTowstring(l_gcr.Characteristics().GetAt(i).Uuid(), l_uuid);
		m_id_to_bd[l_id]->getUuidToService()[l_service]->getUuidToCharacteristic()[l_uuid] =
			::new GattCharacteristic(l_gcr.Characteristics().GetAt(i));
		l_str = ap_jenv->NewString((jchar*)l_uuid.c_str(), (jsize)l_uuid.length());
		ap_jenv->SetObjectArrayElement(l_characteristics, i, l_str);
	}
	return l_characteristics;
}

IAsyncOperation<bool> Discovery::GetGattCharacteristicValueUWP(GattCharacteristic* ap_gc,
	GattReadResult& ar_grr, DataReader& ar_dr)
{
	ar_grr = co_await ap_gc->ReadValueAsync(BluetoothCacheMode::Uncached);
	if (ar_grr.Status() != GattCommunicationStatus::Success)
	{
		std::wcerr << "Failed to read Gatt Characteristic d Due to " << gcs_to_wstring(ar_grr.Status()) << std::endl;
		co_return false;
	}
	else
	{
		ar_dr = DataReader::FromBuffer(ar_grr.Value());
		co_return true;
	}
}

bool Discovery::GetGattCharacteristicValue(GattCharacteristic* ap_gc, GattReadResult& ar_grr, DataReader& ar_dr)
{
	std::wstring l_uuid;
	guidTowstring(ap_gc->Uuid(), l_uuid);

	IAsyncOperation<bool> l_ret = GetGattCharacteristicValueUWP(ap_gc, ar_grr, ar_dr);
	if (!l_ret.get())
	{
		std::wcerr << "Failed to get gatt svcs characteristic value" << std::endl;
		return false;
	}
	else
		return true;
}

bool Discovery::GetBLECharacteristicValue(JNIEnv* ap_jenv, jstring a_id, jstring a_service, jstring a_characteristic, DataReader& ar_dr)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	std::wstring l_id = Java_To_WStr(ap_jenv, a_id);
	std::wstring l_service = Java_To_WStr(ap_jenv, a_service);
	std::wstring l_char = Java_To_WStr(ap_jenv, a_characteristic);

	if (m_id_to_bd.find(l_id) == m_id_to_bd.end()) return false;
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[l_id]->getDeviceInformation();
	if (!lp_di) return false;
	if (m_id_to_bd[l_id]->getUuidToService().find(l_service) == m_id_to_bd[l_id]->getUuidToService().end()) return false;
	BLEDeviceService* lp_bs = m_id_to_bd[l_id]->getUuidToService()[l_service];
	if (!lp_bs) return false;
	if (lp_bs->getUuidToCharacteristic().find(l_char) == lp_bs->getUuidToCharacteristic().end()) return false;
	GattCharacteristic* lp_gc = lp_bs->getUuidToCharacteristic()[l_char];
	if (!lp_gc) return false;
	GattReadResult l_grr{ nullptr };
	bool l_ret = GetGattCharacteristicValue(lp_gc, l_grr, ar_dr);
	return l_ret;
}

IAsyncOperation<bool> Discovery::SetGattCharacteristicValueUWP(GattCharacteristic* ap_gc,
	GattWriteResult& ar_gwr, DataWriter& ar_dw)
{
	ar_gwr = co_await ap_gc->WriteValueWithResultAsync(ar_dw.DetachBuffer());
	if (ar_gwr.Status() != GattCommunicationStatus::Success)
	{
		std::wcerr << "Failed to write Gatt Characteristic due to " <<
			gcs_to_wstring(ar_gwr.Status()) << std::endl;
		co_return false;
	}
	else
	{
		co_return true;
	}
}

bool Discovery::SetGattCharacteristicValue(GattCharacteristic* ap_gc, GattWriteResult& ar_gwr, DataWriter& ar_dw)
{
	IAsyncOperation<bool> l_ret = SetGattCharacteristicValueUWP(ap_gc, ar_gwr, ar_dw);
	if (!l_ret.get())
	{
		std::wcerr << "Failed to set gatt svcs characteristic value" << std::endl;
		return false;
	}
	else
		return true;
}

bool Discovery::SetBLECharacteristicValue(JNIEnv* ap_jenv, jstring a_id, jstring a_service, jstring a_characteristic, DataWriter& ar_dw)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	std::wstring l_id = Java_To_WStr(ap_jenv, a_id);
	std::wstring l_service = Java_To_WStr(ap_jenv, a_service);
	std::wstring l_char = Java_To_WStr(ap_jenv, a_characteristic);

	if (m_id_to_bd.find(l_id) == m_id_to_bd.end()) return false;
	Windows::Devices::Enumeration::DeviceInformation* lp_di = m_id_to_bd[l_id]->getDeviceInformation();
	if (!lp_di) return false;
	if (m_id_to_bd[l_id]->getUuidToService().find(l_service) == m_id_to_bd[l_id]->getUuidToService().end()) return false;
	BLEDeviceService* lp_bs = m_id_to_bd[l_id]->getUuidToService()[l_service];
	if (!lp_bs) return false;
	if (lp_bs->getUuidToCharacteristic().find(l_char) == lp_bs->getUuidToCharacteristic().end()) return false;
	GattCharacteristic* lp_gc = lp_bs->getUuidToCharacteristic()[l_char];
	if (!lp_gc) return false;
	GattWriteResult l_gwr{ nullptr };
	return SetGattCharacteristicValue(lp_gc, l_gwr, ar_dw);
}

void Discovery::DeviceWatcher_Added(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformation deviceInfo)
{
	//MARK;
	if (!smp_dc) return;
	std::wstring l_id(deviceInfo.Id().c_str());
	smp_dc->add_di(l_id, deviceInfo);
}

void Discovery::DeviceWatcher_Updated(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate)
{
	//MARK;
	if (!smp_dc) return;
	std::wstring l_id(deviceInfoUpdate.Id().c_str());
	smp_dc->update_di(l_id, deviceInfoUpdate);
}

void Discovery::DeviceWatcher_Removed(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Devices::Enumeration::DeviceInformationUpdate deviceInfoUpdate)
{
	//MARK;
	if (!smp_dc) return;
	std::wstring l_id(deviceInfoUpdate.Id().c_str());
	smp_dc->remove_di(l_id);
}

void Discovery::DeviceWatcher_EnumerationCompleted(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&)
{
	//MARK;
	if (!smp_dc) return;
	smp_dc->signal_discovery_complete();
}

void Discovery::DeviceWatcher_Stopped(Windows::Devices::Enumeration::DeviceWatcher sender, Windows::Foundation::IInspectable const&)
{
	//MARK;
	if (!smp_dc) return;
	smp_dc->signal_discovery_complete();
}

void Discovery::Characteristic_ValueChanged(GattCharacteristic const& ar_gc, GattValueChangedEventArgs a_args)
{
	//MARK;
	if (!smp_dc) return;
	std::wstring l_gc_uuid;
	guidTowstring(ar_gc.Uuid(), l_gc_uuid);
	//DEBUG_TRACE("Got value changed on " << l_gc_uuid);
	DataReader l_dr = DataReader::FromBuffer(a_args.CharacteristicValue());
	//MARK;
	smp_dc->signal_characteristic_valuechanged(l_gc_uuid, l_dr);
	//DEBUG_TRACE("Handled value changed on " << l_gc_uuid);
}

void Discovery::signal_characteristic_valuechanged(std::wstring& ar_gc_uuid, DataReader& ar_dr)
{
	concurrency::critical_section::scoped_lock l_lock(m_lock_std);
	//MARK;
	if (m_uuid_to_notification_token.find(ar_gc_uuid) == m_uuid_to_notification_token.end())
	{
		std::wcerr << "Notification token not found" << std::endl;
		return;
	}
	//MARK;
	if (m_uuid_to_notification_event.find(ar_gc_uuid) == m_uuid_to_notification_event.end())
	{
		std::wcerr << "Notification event not found" << std::endl;
		return;
	}
	//MARK;
	std::string* lp_bytes = new std::string();
	int l_len = ar_dr.UnconsumedBufferLength();
	for (int i = 0; i < l_len; ++i)
	{
		(*lp_bytes) += ar_dr.ReadByte();
		//                if (i == 0) std::wcerr << "Got byte 0 -> " << (int)lp_bytes->at(0) << std::endl;
	}
	/*
	//MARK;
	jbyte* lp_jbytes = ::new jbyte[l_len];
	//DEBUG_TRACE("Created jbyte array size " << l_len);
	//MARK;
	//DEBUG_TRACE("Created jbyteArray on " << mp_jenv);
	jbyteArray l_jba = mp_jenv->NewByteArray(l_len);
	//MARK;
	for (int i = 0; i < l_len; ++i)
	{
		//MARK;
		lp_jbytes[i] = ar_dr.ReadByte();
		//MARK;
	}
	//MARK;
	mp_jenv->SetByteArrayRegion(l_jba, 0, l_len, lp_jbytes);
	delete lp_jbytes;
	//MARK;
	*/
	m_uuid_to_changed_value[ar_gc_uuid].push(lp_bytes);
	//MARK;
	m_uuid_to_notification_event[ar_gc_uuid].set();
	//MARK;
}

IAsyncOperation<bool> Discovery::WriteClientCharacteristicConfigurationDescriptorAsync(GattCharacteristic* ap_gc,
	GattClientCharacteristicConfigurationDescriptorValue a_cccdValue)
{
	GattCommunicationStatus l_gcs = co_await ap_gc->WriteClientCharacteristicConfigurationDescriptorAsync(a_cccdValue);
	if (l_gcs != GattCommunicationStatus::Success)
	{
		std::wcerr << "Failed to write cccd due to " <<
			gcs_to_wstring(l_gcs) << std::endl;
		co_return false;
	}
	else
	{
		co_return true;
	}
}