# STUSB4500 SW drivers and tools (USB Power Delivery)
Autonomous USB-PD controller for Power Sinks / UFP (i.e. on the Devices side) <br/>
Certified by USB-IF consortium for USB Type-C and USB Power Delivery applications. <br/>

## Application summary
By default the USB-C specification only provide 5V from a USB-PD Power Source (Host / DFP) <br/>
This product enables to automatically negociate to the Source a higher Voltage (>5V) up to 100W. <br/>
For instance, if the Power brick can provide 3 power profiles (5V, 9V and 15V) then the STUSB4500 will request the highest voltage available (15V). <br/>
This part can be easily implemented in a battery charger with USB-C input in the application.

The device doesn't need any software to run (it is automous). But it is possible to connect to this device by I2C to take control over the default device behavior, or to get the power information (Voltage/Current) of the attached power source at the other side of the cable. This is what is showed in the demo source of the project's repository.

Info:
----------------
* __Device:__       STUSB4500 > USB PD controller  <br />
* __Manufacturer:__ STMicroelectronics
* __USB power role:__ UFP (Power Sink side)
* __USB data  role:__ independant (works with USB Host or Device - can be USB2.0 or USB3.1 or No data)
* __Typical Application:__ To Get automatically higher power on Vbus at the insertion of USB-C cable.
* __Features:__ Autonomous, configurable, easy to use, small footprint <br />
* __Operating supply:__ 5V to 20V directly from Vbus of the USB-C cable. <br /> Or a lower power (3.3V) for battery supplied applications <br />
* __Package:__ QFN-24, WLCSP MSL1 <br />
* __Part number:__ STUSB4500QTR, STUSB4500BJR
* __Spec:__ USB-C v1.2 , USB PD v2.0 & v3.0

## Presentation
The STUSB4500 is a USB power delivery controller that addresses sink devices. It implements a proprietary algorithm to allow the negotiation of a power delivery contract with a source without MCU support (auto-run mode). PDO profiles are configured in an integrated non-volatile memory. <br/>

The device supports dead battery mode and is suited for sink devices powered from dead battery state and requiring high power charging profile to be fully operational. <br/>

Thanks to its 20 V technology, it implements high voltage features to protect the CC pins against short-circuits to VBUS up to 22 V and to support high voltage on the VBUS pins directly connected to the VBUS power path up to 28 V. <br/>

## Key Features
* Auto-run Type-C™ and USB PD sink controller
* Dead battery mode support
* Up to 3 sink PDO configurable profiles
* Dual high power charging path support
* Integrated VBUS switch gate drivers (PMOS)
* Integrated VBUS voltage monitoring
* Internal and/or external VBUS discharge paths
* Short-to-VBUS protections on CC pins (22 V)
* High voltage capability on VBUS pins (28 V)
* Dual power supply (VSYS and/or VDD):
  * VSYS = [3.0 V; 5.5 V]
  * VDD = [4.1 V; 22 V]
* Debug accessory mode support
* Temperature range: -40 °C up to 105 °C
* ESD: 3 kV HBM - 1.5 kV CDM
* Certified:
  * USB Type-C™ rev 1.2
  * USB PD rev 2.0 (Official Test ID: TID #1000133)
* Interoperable with USB PD rev 3.0


## NVM programming
The chip can be used in standalone with a custom configuration thanks to its internal memory.  <br/>
The Non-Volatile Memory (NVM) contains the STUSB4500 configuration which load automatically at power-up.  <br/>
But the NVM memory is not directly accessible byte per byte. It has to be accessed by block, following a specific sequence.

The NVM programming is done through I2C. <br/>
You can program the chip during manufacturing with any standard programming tool, as long as the tool has access to the I2C interface.

Note that STSW-STUSB003 Software Library is only used to manage the standard registers of STUSB4500, but not the NVM registers block.  <br/>
If you want to program the NVM memory, it is needed to use the STUSB_NVM_Library project.  <br/>
You can request the NVM Library by contacting the manufacturer online support : https://my.st.com/ols


Here are the steps to program the NVM memory:
1. Use the STSW-STUSB002 GUI to configure the STUSB4500 according to your application needs (PDO, Voltage, …)
1. With the GUI, generate the NVM config file (.h) which contains the binary configuration of STUSB45
1. Use the STUSB_NVM_Library source code to write the binary configuration (.h) into the chip NVM memory
1. Reset the chip so that it reloads its latest NVM configuration
