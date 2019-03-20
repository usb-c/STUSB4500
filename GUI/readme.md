# STSW-STUSB002 GUI Application
Graphical User Interface for STUSB4500

![Screenshot GUI](https://raw.githubusercontent.com/usb-c/STUSB4500/master/GUI/Pics/STUSB4500_GUI_1.08_offline_screenshot.png)

## Presentation

The STSW-STUSB002 is a free graphical interface (GUI) aimed at customizing seamlessly the STUSB devices through direct access to non-volatile-memory (NVM). <br/>

The tool allows NVM area to be read, configured and written without dedicated software skills, thanks to a graphical interface. <br/>

The utility also provides a basic panel to read and write I²C registers at any device address and any register address, as well as a device dashboard. <br/>

The software can be used with the evaluation boards from the STUSB devices stacked on a NUCLEO-F072RB controller board. The device selection is done automatically at power-up, as far as the Nucleo is flashed and configured in order to act as a USB to I²C bridge (.bin included). <br/>


## Key Features
* Access to the STUSB non-volatile-memory
* Read and write any I2C register address
* Device dashboard (to see chip status in real time)
* Operating system required: Microsoft Windows 7, 8.1, 10

<!---
![Screenshot2 GUI](https://github.com/usb-c/STUSB4500/blob/master/GUI/Pics/STUSB4500_GUI_1.08_online_screenshot.png)
--->
<!---
<img src="https://github.com/usb-c/STUSB4500/blob/master/GUI/Pics/STUSB4500_GUI_1.08_online_screenshot.png" height="400">
--->

<img src="https://raw.githubusercontent.com/usb-c/STUSB4500/master/GUI/Pics/STUSB4500_GUI_1.08_online_screenshot.png" height="500">


## Installation instructions

### Prerequisites:
1. USB Drivers

	   On Windows 10, you don't need to install additional drivers, 
	   because USB CDC drivers are installed automatically. (usbser.sys)

	   On Windows 7, install: STM32 Virtual COM Port Driver (STSW-STM32102)
	   http://www.st.com/en/development-tools/stsw-stm32102.html

	   
1. Install Runtime: Visual C++ 2015 or 2017 or 2019
1. Install Runtime: Visual C++ 2012

### Download Links:

* Microsoft Visual C++ Redistributable 2017 (32bit)  <br>
  https://aka.ms/vs/15/release/vc_redist.x86.exe

* Microsoft Visual C++ 2015 Redistributable Update 3  (vcredist_x86.exe)  <br>
  https://www.microsoft.com/en-us/download/details.aspx?id=53587

* Visual C++ Redistributable for Visual Studio 2012 Update 4  (vc_redist.x86.exe) <br>
  https://www.microsoft.com/en-us/download/details.aspx?id=30679

__Note:__
```
The Visual C++ 2017 Redistributable is a binary-compatible in-place upgrade of the Visual C++ 2015 Redistributable and shares the same registry keys.
So 2017 Redist replaces the 2015.
If you already have VC++ 2017, you cannot install VC++ 2015.
```

## How To use:
1. Connect the STM32 Nucleo board to a computer, using a USB-A to USB-mini-B cable

1. Flash the STM32F072-Nucleo-Board firmware with the binary file included in this package.
   - Simply copy-paste the STM32 binary file (.bin) to the virtual mass storage, 
	which appears when you connect the Nucleo board to the computer via USB.

1. Launch the executable (.exe)

1. Use the GUI to read or program the chip memory.


## Requirements:
- System:   
	- Windows 7, Windows 8.1, Windows 10
	- 1 USB 2.0 FS port
  
- Hardware: 
	- 1 STM32 Nucleo board (STM32F0 or STM32G0)
	- 1 evaluation board, or custom board containing STUSB device.
	- 1 USB type-A cable to type-B mini or micro (to connect the Nucleo-board to the computer)
	- 1 USB type-C cable (to test the device)
 
 

 ## Troubleshoot:
  
* To be able to use the STM32 Nucleo board properly, be sure to program the board with the right firmware (by dragging-dropping the .bin file, or copy-paste the file). <br>
When the Nucleo board is ready, the User_LED (LED2) must be blinking. <br>
If it is not blinking, there is an hardware issue, and the GUI application will not be able to connect to it. <br>

* To check the Hardware connection of the PC with the Nucleo board, you can use the command-line Nucleo utility (HwCheckUtility.exe)
