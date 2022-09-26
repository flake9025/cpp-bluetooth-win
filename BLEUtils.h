#pragma once
#ifndef BLE_UTILS_H
#define BLE_UTILS_H

#include <iostream>
#include <iomanip>
#include <jni.h>
#include <sstream>

using namespace winrt;
using namespace Windows::Devices::Bluetooth::GenericAttributeProfile;

/*
* Java String to C++ Wide String
*/
std::wstring Java_To_WStr(JNIEnv* env, jstring string);

/*
* UUID to Wide String
* Used with BLE Services and GATT Characteristics
* ex: 00001801-0000-1000-8000-00805F9B34FB
*/
void guidTowstring(guid& ar_guid, std::wstring& ar_wstring);

/*
* GATT Enum to Wide String
*/
std::wstring gcs_to_wstring(GattCommunicationStatus status);

#endif

// Utility Functions
#if defined(_DEBUG)
#define MARK std::wcerr << __func__ << ':' << __LINE__ << std::endl;
#define DEBUG_TRACE(reason) std::wcerr << __func__ << ':' << __LINE__ << ' ' << reason << std::endl;
#else
#define MARK
#define DEBUG_TRACE(reason)
#endif