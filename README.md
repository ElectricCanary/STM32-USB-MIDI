# STM32-USB-MIDI

This is a USB class compliant MIDI interface for the STM32F411 "Blackpill".

Under "Source" you'll find the core files that make the interface work.
The .elf file allows to test on a board without having to compile the project yourself.
Finally, the whole project can be unzipped and imported in STM32CubeIDE.


#Device Configuration Tool

If using the Device Configuration Tool, you must leave active the "USB_OTG_FS" under "Connectivity" and the "USB_DEVICE" under "Middleware".
Once the code is generated, you must delete the folders : "Middlewares" and "USB_DEVICE".
These are not needed since the whole USB interface is already in the "Core" folder.
You'll also need to delete the " #include "usb_interface.h" " at the beginning of "main.c" (line 22).
