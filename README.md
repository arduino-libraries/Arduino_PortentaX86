# PortentaX86: A portable, open-source 8086 PC emulator for Portenta H7
X86 emulator derived from Faux86

## Features
- 8086 and 80186 instruction set emulation
- CGA / EGA / VGA emulation is mostly complete
- PC speaker, Adlib and Soundblaster sound emulation
- Serial mouse emulation

## Usage
By default PortentaX86 boots from a floppy image dosboot.img which in the emulator is mounted as drive A.
You should copy a floppy image in internal QSPI "ota" partition to have it running.
An handy method to load the floppy image is by running "AccessFlashAsUSBDisk" sketch and copying the file in the bigger partition.

## Credits
Faux86 was originally based on the Fake86 emulator by Mike Chambers. 
http://fake86.rubbermallet.org
A lot of the code has been shuffled around or rewritten in C++ but the core CPU emulation remains mostly the same.




  