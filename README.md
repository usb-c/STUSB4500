# STUSB4500 SW drivers and tools (USB Power Delivery)
Autonomous USB-C PD controller for Power Sinks / UFP (i.e. on the Devices side) <br/>
Certified by USB-IF consortium for USB Type-C and USB Power Delivery applications (USB-C & USB PD). <br/>

## Application summary
By default the USB-C specification only provide 5V from a USB-PD Power Source (Host / DFP) <br/>
This product enables to automatically negociate to the Source a higher Voltage (>5V) up to 100W. <br/>
For instance, if the Power brick can provide 4 power profiles (5V, 9V, 15V and 20V), then the STUSB4500 will request the highest voltage available (20V). <br/>
Another example, if the Power brick can provide 4 power profiles (5V, 9V, 15V and 20V) but the Application needs 9V to boot, then the STUSB4500 can be programmed to always request 9V. <br/>
This part can be easily implemented in a battery charger with USB-C input in the application.

The device doesn't need any software to run (it is automous). But it is possible to connect to this device by I2C to take control over the default device behavior, or to get the power information (Voltage/Current) of the attached power source at the other side of the cable. This is what is showed in the demo source code of the project's repository.

Info:
----------------
* __Device:__       STUSB4500 -> USB PD controller  <br />
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


## Programming

### Register Programming
The USB-PD chip has 148 registers (0x94).  <br/>
Use the firmware example contained in this repository (STSW-STUSB003 Software library for STUSB4500) to control the STUSB4500 with a Micro-controller, and take control over the default behavior of the device.
You can for instance with the MCU request any PDO that you want from the DFP (you are no more limited to 3 PDOs).

### NVM programming
The chip can be used in standalone with a custom configuration thanks to its internal memory.  <br/>
The Non-Volatile Memory (NVM) contains the STUSB4500 configuration which load automatically at power-up.  <br/>
But the NVM memory is not directly accessible byte per byte. It has to be accessed by block, following a specific sequence.  <br/>
The NVM size is 40 bytes.  <br/>

The NVM programming is done through I2C. <br/>
You can program the chip during manufacturing with any standard programming tool, as long as the tool has access to the I2C interface.


Here are the steps to program the NVM memory:
1. Use the STSW-STUSB002 GUI to configure the STUSB4500 according to your application needs (PDO, Voltage, Current, Overvoltage protection, …)
1. With the GUI, generate the NVM config file (.h) which contains the binary configuration of STUSB45
1. Use the STUSB_NVM_Library source code to write the binary configuration (.h) into the chip NVM memory
1. Reset the chip so that it reloads its latest NVM configuration


## Evaluation Board
There are different boards available to test the product.

* STUSB4500 evaluation board : STEVAL-ISC005V1   <br/>
https://www.st.com/content/st_com/en/products/evaluation-tools/solution-evaluation-tools/psu-and-converter-solution-eval-boards/steval-isc005v1.html

  * Note : This board requires to be connected on top of a STM32 Nucleo board : NUCLEO-F072RB or NUCLEO-G071RB <br/>
https://www.st.com/en/evaluation-tools/nucleo-f072rb.html <br/>
https://www.st.com/en/evaluation-tools/nucleo-g071rb.html <br/>

<p align="center">
<img src="https://raw.githubusercontent.com/usb-c/STUSB4500/master/Pics/en.steval-isc005v1_image.jpg" width="200">

<img src="https://raw.githubusercontent.com/usb-c/STUSB4500/master/Pics/en.nucleo-G0.jpg" width="200">
</p>

* STUSB4500 reference design : STREF-SCS001V1   <br/>
https://www.st.com/content/st_com/en/products/evaluation-tools/solution-evaluation-tools/psu-and-converter-solution-eval-boards/stref-scs001v1.html

<p align="center">
<img src="https://raw.githubusercontent.com/usb-c/STUSB4500/master/Pics/en.stref-scs001v1_image.JPG" width="200">
</p>

* Nano USB PD to DC Module : CG-NANOPDDC   <br/>
https://www.coolgear.com/product/nano-usb-pd-to-dc-module-pd-negotiation-device

<p align="center">
<img src="https://raw.githubusercontent.com/usb-c/STUSB4500/master/Pics/mini-pd-profile-tester5x1000.jpg" width="200">
</p>


## More

<p align="center">
<a href="http://www.youtube.com/watch?feature=player_embedded&v=pFOu_4AzVVg
" target="_blank"><img src="http://img.youtube.com/vi/pFOu_4AzVVg/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="350" border="10" /></a>
   <a href="http://www.youtube.com/watch?feature=player_embedded&v=Sld2pj-_ho0
" target="_blank"><img src="http://img.youtube.com/vi/Sld2pj-_ho0/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="350" border="10" /></a>
</p>

<!---
[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg)](http://www.youtube.com/watch?v=YOUTUBE_VIDEO_ID_HERE)
--->

## Notes
The STUSB45 is a standalone USB-C PD controller for UFP only (Power SINK). <br/>
It's counter part for DFP only (Power SOURCE) is STUSB47. <br/>
But as these 2 devices has been certified by USB-IF, it mean they are interoperable with any USB-C PD application. <br/>
Plenty scenarios are possible, for instance:  <br/>

| Source ctrl   | USB-C cable   |  Sink ctrl |
|-----------|:---------------:|----------:|
| STUSB47x0 |   <---------->  |   STUSB45 |
| STUSB1600 |   <---------->  |   STUSB45 |
| STUSB1602 |   <---------->  |   STUSB45 |
| STM32G0xx |   <---------->  |   STUSB45 |
| STUSB47x0 |   <---------->  | STM32Fx + STUSB45 |
| STUSB47x0 |   <---------->  | STM32MP1 + STUSB45 |


