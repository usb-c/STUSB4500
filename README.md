# STUSB4500 SW drivers and tools (USB Power Delivery)
Autonomous USB-C PD controller for Power Sinks / UFP (i.e. on the Devices side) <br/>
Simple solution for Barrel Connector Replacement, using instead a universal USB connector.
Certified by USB-IF consortium for USB Type-C and USB Power Delivery applications (USB-C & USB PD). <br/>

## Application summary
By default a USB Type-C port on your hardware can get only 5V from a USB-PD Power Source (Host / DFP) <br/>
This product enables to automatically negociate with the Source a higher Voltage (>5V) up to 100W (20V@5A). <br/>
For instance, if the Power brick can provide 4 power profiles (5V, 9V, 15V and 20V), then the STUSB4500 will request the highest voltage available (20V). <br/>
Another example, if the Power brick can provide 4 power profiles (5V, 9V, 15V and 20V) but the Application needs 9V to boot, then the STUSB4500 can be programmed to always request 9V. <br/>
This part can be easily implemented in a battery charger with USB-C input in the application.

The device doesn't need any software to run (it is automous). But it is possible to connect to this device by I2C to take control over the default device behavior, or to get the power information (Voltage/Current) of the attached power source at the other side of the cable. This is what is showed in the demo source code of the project's repository (firmware_src).

Info:
----------------
* __Device:__       STUSB4500 -> USB PD controller  <br />
* __Manufacturer:__ STMicroelectronics
* __Typical Application:__ To get automatically higher power on Vbus at the insertion of USB-C cable.
* __USB power role:__ Power SINK
* __USB data  role:__ independant (works with No Data, or with UFP (USB Device) that can be USB2.0 or USB3.1)   <br />
Note: If using the USB-C connector for both Power and Data (with STUSB4500 controller for Power communication on one side, and a USB Data controller on the other side), then the USB-C specification requires for a Power Sink application, that its initial data role (at cable attachment) is always UFP (Device).  <br />
Similarly, the initial data role of a Power Source application is always DFP (Host). <br />
To change the role, it is needed to send a USB PD command: PowerRole_Swap or DataRole_Swap.

* __Features:__ Autonomous, configurable, easy to use, small footprint <br />
* __Operating supply:__ 5V to 20V directly from Vbus of the USB-C cable. <br /> Or a lower power (3.3V) for battery supplied applications <br />
* __Power Consumption:__ 140 uA only <br />
* __Package:__ QFN-24, WLCSP MSL1 <br />
* __Part number:__ STUSB4500QTR, STUSB4500BJR
* __USB Spec:__ USB Type-C v1.2 , USB PD v2.0 & v3.0

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
The USB-PD chip has 148 programmable registers (0x94).  <br/>
To simplify the implementation, Use the firmware example contained in this repository (STSW-STUSB003 Software library for STUSB4500) to control the STUSB4500 with a Micro-controller, and take control over the default behavior of the device.
You can for instance with the MCU request any PDO that you want from the DFP (you are no more limited to 3 PDOs).

Link: [Source code for this project](Firmware/Project/Src)  <br/>
Link: [Firmware readme](Firmware/readme.md)  <br/>

### NVM programming
The chip can be used in standalone with a custom configuration thanks to its internal memory.  <br/>
The Non-Volatile Memory (NVM) contains the STUSB4500 configuration which load automatically at power-up.  <br/>
But the NVM memory is not directly accessible byte per byte. It has to be accessed by block, following a specific sequence.  <br/>
The NVM size is 40 bytes.  <br/>

The NVM programming is done through I2C. <br/>
You can program the chip during manufacturing flow with any standard programming tool, as long as the tool has access to the I2C interface.

Link: [Source code for this project](NVM_Flasher)  <br/>


#### Here are the steps to program the NVM memory (with your own tools):
1. Use the STSW-STUSB002 GUI to configure the STUSB4500 parameters according to your application needs (PDO, Voltage, Current, Overvoltage protection, …).   <br/>
Note that the GUI can be used offline for the parameters selection (i.e. without being connected physically to STUSB4500).
1. With the GUI, generate the NVM config file (.h) (or .txt) which contains the binary configuration of STUSB45
1. Use the NVM_Library source code (which contains the specific I2C sequence) to create an application which writes the binary configuration (.h) into the chip NVM memory
1. Reset the chip so that it reloads its latest NVM configuration
1. Now the chip is programmed. Each time it starts up, it will load the configuration from the NVM memory.

* Note 1 : In a typical application, the chip's NVM only needs to be programmed once (during manufacturing flow).
* Note 2 : The NVM technology has a limited number of write cycles possible (few thousands). So do not use an application which re-write the NVM all the time.


#### Here is a Faster alternative to program the NVM memory (with the GUI and Nucleo board):
1. Connect the STUSB4500 eval board on top of STM32F072 Nucleo board. And connect the Nucleo board to your computer via USB.
1. Use the STSW-STUSB002 GUI to configure the STUSB4500 parameters according to your application needs (PDO, Voltage, Current, Overvoltage protection, …).   <br/>
1. With the GUI, click the "Write NVM" button to flash the memory
1. Reset the chip so that it reloads its latest NVM configuration


## Evaluation Boards
There are different boards available to test the product.

* STUSB4500 evaluation board : STEVAL-ISC005V1   <br/>
https://www.st.com/content/st_com/en/products/evaluation-tools/solution-evaluation-tools/psu-and-converter-solution-eval-boards/steval-isc005v1.html

  * Note : This board requires to be connected on top of a STM32 Nucleo board : NUCLEO-F072RB or NUCLEO-G071RB <br/>
https://www.st.com/en/evaluation-tools/nucleo-f072rb.html <br/>
https://www.st.com/en/evaluation-tools/nucleo-g071rb.html <br/>
To use the GUI with these boards, flash the STM32 with the corresponding binary file.  <br/>

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


<p align="center">
<a href="http://www.youtube.com/watch?feature=player_embedded&v=0-J0FWWDuRg
" target="_blank"><img src="http://img.youtube.com/vi/0-J0FWWDuRg/0.jpg" 
alt="IMAGE ALT TEXT HERE" width="350" border="10" /></a>
</p>

<!---
[![IMAGE ALT TEXT HERE](http://img.youtube.com/vi/YOUTUBE_VIDEO_ID_HERE/0.jpg)](http://www.youtube.com/watch?v=YOUTUBE_VIDEO_ID_HERE)
--->

## Notes
The STUSB45 is a standalone USB-C PD controller for UFP only (Power SINK). <br/>
Its counter part for DFP only (Power SOURCE) is STUSB47. <br/>
Because these 2 devices have been certified by USB-IF, it means they are interoperable with any USB-C PD application on the market. <br/>
Plenty of scenarios are possible, for instance:  <br/>

| Source ctrl   | USB-C cable   |  Sink ctrl |
|-----------|:---------------:|----------:|
| STUSB47x0 |   <---------->  |   STUSB45 |
| STUSB1600 |   <---------->  |   STUSB45 |
| STUSB1700 |   <---------->  |   STUSB45 |
| STUSB1602 + STM32Fx |   <---------->  |   STUSB45 |
| STUSB1702 + SPC58 |   <---------->  |   STUSB45 |
| STM32G0xx |   <---------->  |   STUSB45 |
| STUSB47x0 |   <---------->  | STM32Fx + STUSB45 |
| STUSB47x0 |   <---------->  | STM32MP1 + STUSB45 |
| STM32MP1 + STUSB1600 |   <---------->  |   STUSB45 |

