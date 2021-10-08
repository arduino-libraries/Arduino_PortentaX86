/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef _kernel_h
#define _kernel_h

#include "Arduino.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"

#define USE_BCM_FRAMEBUFFER 0
#define USE_EMBEDDED_BOOT_FLOPPY 0
#define USE_EMBEDDED_ROM_IMAGES 1
#define USE_MMC_MOUNTING 0
#define USE_SERIAL_LOGGING 0
#define USE_VCHIQ_SOUND 0

enum TShutdownMode
{
	ShutdownNone,
	ShutdownHalt,
	ShutdownReboot
};

namespace Faux86
{
	class VM;
	class Config;
	class CircleHostInterface;
}

class CKernel
{
public:
	CKernel (void);
	~CKernel (void);

	bool Initialize ();

	TShutdownMode Run (void);

private:
	// do not change this order

	QSPIFBlockDevice 		root;
	mbed::FATFileSystem		m_FileSystem;
	mbed::MBRBlockDevice 	wifi_data;
	mbed::MBRBlockDevice 	ota_data;
	mbed::FATFileSystem 	wifi_data_fs;
	mbed::FATFileSystem 	ota_data_fs;

	Faux86::CircleHostInterface* HostInterface;
	Faux86::Config* vmConfig;
	Faux86::VM* vm;
};

#endif
