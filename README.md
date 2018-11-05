# STUSB4500
Autonomous USB-PD controller for Power Sinks / UFP (i.e. on the Devices side) <br/>
Certified by USB-IF for USB Type-C and USB Power Delivery applications. <br/>

## Application summary
By default the USB-C specification only provide 5V from a USB-PD Power Source (Host / DFP) <br/>
This product enables to automatically negociate to the Source a higher Voltage (>5V) up to 100W. <br/>

For instance, if the Power brick can provide 3 power profiles (5V, 9V and 15V) then the STUSB4500 will request the highest voltage available (15V).

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
