                    Central Peripheral Device Sample Code
                    =====================================

NOTE: If you use this sample application for your own purposes, follow
      the licensing agreement specified in Software_Use_Agreement.rtf
      in the home directory of the installed RSL10 Evaluation and
      Development Kit (EDK).

Overview
--------
This sample project performs both the central role and peripheral role.

It prepares battery service client and custom service client for the central
role of the application. It prepares battery service server and custom service
server for the peripheral role of the application.

At the initiation of the application, it starts with the peripheral role.
As a peripheral device, if an address is available at DEVICE_INFO_BLUETOOTH_ADDR
in non-volatile memory three (NVR3), it starts undirected connectable 
advertisements with the device's public address. If this address is not 
defined (all 1s or 0s), it uses a pre-defined, private Bluetooth(R) 
address (PRIVATE_BDADDR) located in ble_std.h.

For the peripheral role, any central device can scan, connect, and perform 
service discovery, receive battery value notifications or read the battery 
value. The central device has the ability to read and write custom attributes. 
The RSL10 ADC is used to read the battery level value every 200 ms when there 
is a kernel timer event. An average is calculated over every 16 reads and if 
this average value changes, a flag is set to send the battery level 
notification. In addition, if the custom service notification is enabled, it 
sends a notification with an incremental value, every six seconds. 

If the peripheral role establishes a connection with a central device or even 
if it fails to do so within the first 10 seconds of its advertisements, the 
application switches its role to central and sends start connection commands to
put the device in the directed connectable mode.

The above also applies to the central role of the application.
If the central role establishes a connection with a peripheral device or even 
if it fails to do so within the first 10 seconds of its connection requests,
the application switches its role to peripheral and starts advertising.

This sample project has six states before all services are enabled. They
are: initialization (APPM_INIT), create database (APPM_CREATE_DB),
ready (APPM_READY), advertising (APPM_ADVERTISING), connecting 
(APPM_CONNECTING), connected (APPM_CONNECTED).

APPM_INIT        - Application initialized and configured into an idle state 
APPM_CREATE_DB   - Application has configured the Bluetooth stack and it is 
                   adding the battery service client and databases into the
                   Bluetooth stack.
APPM_READY       - Application has added the desired standard and custom 
                   services or profiles into the Bluetooth GATT database and
                   handlers.
APPM_ADVERTISING - The device starts advertising based on the sample project. 
APPM_CONNECTING  - The device is waiting for a direct connection establishment.
APPM_CONNECTED   - Connection established with a compatible device.

Peripheral Role
---------------
After establishing connection, the BLE_SetServiceState function is called in 
which, for any profiles/services that need to be enabled, an enable request 
message is sent to the corresponding profile of the Bluetooth stack.
Once the response is received, for the specific profile, a flag is set to 
indicate to the application that it is enabled and ready to use.  

Central Role
------------
Once the connection toward a peripheral devices is established, the central 
device starts the battery service and custom service discovery. If the battery
service is discovered then the variable basc_support_env.enable is set and the
application sends a read request for the battery level value every five seconds,
using a kernel timer that sets a flag for this purpose. 

If the custom service and related characteristic are discovered, the application
sends a write request for one of the custom attributes every two seconds. 
For the custom service, if the custom service UUID and the two characteristics 
UUID are discovered, the variable cs_env.state is set to CS_ALL_ATTS_DISCOVERED.


This sample project is structured as follows:

The source code exists in a "code" folder, all application-related include
header files are in the "include" folder and the main() function "app.c" is 
located in the parent directory.

Code
----
    app_init.c    - All initialization functions are called here, but the 
                    implementation is in the respective C files
    app_process.c - Message handlers for application
    ble_basc.c    - Support functions and message handlers pertaining to Battery
                    Service Client
    ble_bass.c    - Support functions and message handlers pertaining to Battery
                    Service Server
    ble_custom.c  - Support functions and message handlers pertaining to Custom
                    Service Server
    ble_std.c     - Support functions and message handlers pertaining to
                    Bluetooth low energy technology

Include
-------
    app.h        - Overall application header file
    ble_basc.h   - Header file for Battery Service Client
    ble_bass.h   - Header file for Battery Service Server
    ble_custom.h - Header file for Custom Service Server
    ble_std.h    - Header file for Bluetooth low energy standard
    
Hardware Requirements
---------------------
This application can be executed on any Evaluation and Development Board 
with no external connections required.

Importing a Project
-------------------
To import the sample code into the Evaluation Development Kit (EDK), refer to 
the "RSL10 Sample Code User's Guide" for more information.

Select the project configuration according to the required optimization level. 
Use "Debug" configuration for optimization level "None" or "Release"
configuration for optimization level "More" or "O2".

Verification
------------
To verify if the central role is functioning correctly, use RSL10 or another 
third-party peripheral application coupled with a Bluetooth sniffer application 
to ensure that a connection is established. If this application has established 
a link with RSL10 peripheral server application, DIO0 and DIO1 will toggle 
every 5 and 2 seconds respectively. 

To verify if the peripheral role is functioning correctly, use RSL10 or another 
third-party central device application to establish a connection. To show how an
application can send notifications, for every 30 timer expirations, a 
notification request flag is set and the application sends an incremental value 
of the first attribute to a peer device.

In addition to establishing a connection, this application can be used to 
read/write characteristics and receive notifications.

Alternatively, while the Bluetooth application manager is in the state 
"APPM_ADVERTISING" (for peripheral role) or "APPM_CONNECTING" (for central role),
the LED on the Evaluation and Development Board (DIO6) is blinking. It turns 
solid once the link is established, and it goes to the APPM_CONNECTED state.

===============================================================================
Copyright (c) 2017 Semiconductor Components Industries, LLC
(d/b/a ON Semiconductor).

