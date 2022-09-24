# C++ Windows Bluetooth LE Driver

Note : This is a work in progress

## What is it for ?

This project is a C++ Client / Driver for using Windows 10 Bluetooth LE API.

It provides the following operations :
- Get Devices
- Get Device Services
- Get Service Characteristics
- Get / Set a Characteristic value

## How to use ?

It is based on JNI technology : the project contains the C++ implementation of a Java interface 
https://github.com/flake9025/java-bluetooth-win

The result of the build is a DLL library

To compile, you  will need:
- Microsoft Visual Studio
- Microsoft Windows 10 SDK

Microsoft documentation and samples are available here :
[Windows Bluetooth BLE](https://github.com/Microsoft/Windows-universal-samples/tree/main/Samples/BluetoothLE).


