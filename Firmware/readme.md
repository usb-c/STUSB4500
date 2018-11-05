# STSW-STUSB003 open source firmware
Software Library for STUSB4500

## Presentation
The STSW-STUSB003 is a software library enabling the configuration of STUSB4500, and its typical use like dynamic power input management.  <br/>
The Open source software is available to customize end-application software developments.  <br/>
The library includes: the STUSB4500 hardware abstraction layers, drivers, Code example, demo and the user manual.  <br/>

## Key Features
* Open source code
  * Alarm management
  * Send HW and SW reset
  * Access to SINK and SOURCE PDOs
  * Change of SINK PDO
  * Alignment of SINK PDOs to SOURCE PDOs
* Compilation Environment:
  * Toolchain: IAR 8.x: IAR Embedded Workbench for ARM   
  https://www.iar.com/iar-embedded-workbench/#!?architecture=Arm
  * pre-compiled demo binary provided
* Hardware requirements:
  * Any microcontroller with I2C interface
  * STEVAL-ISC005V1: STUSB4500 evaluation board
  * The demo is using NUCLEO-F072RB: STM32 Nucleo-64 development board with ARM Cortex-M0
