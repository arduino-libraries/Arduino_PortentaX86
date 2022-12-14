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
#include "kernel.h"
#include "CircleHostInterface.h"
#include "faux86/VM.h"
#include "CircleDeviceDisk.h"
#include "FatFsDisk.h"
#include "SDRAM.h"
#include <new>
#include "RPC.h"
#include "Keymap.h"

// Embedded disks / ROMS
#if USE_EMBEDDED_BOOT_FLOPPY
#include "../data/dosboot.h"
#endif
#if USE_EMBEDDED_ROM_IMAGES
#include "../data/asciivga.h"
#include "../data/pcxtbios.h"
#include "../data/videorom.h"
#include "../data/rombasic.h"
#endif

#define REQUIRED_KERNEL_MEMORY (0x2000000)

#if KERNEL_MAX_SIZE < REQUIRED_KERNEL_MEMORY
//#error "Not enough kernel space: change KERNEL_MAX_SIZE in circle/sysconfig.h to at least 32 megabytes and recompile circle"
#endif

#define AUDIO_SAMPLE_RATE 44100

static const char FromKernel[] = "kernel";

using namespace Faux86;

CKernel::CKernel (void) : wifi_data(&root, 1), ota_data(&root, 2), wifi_data_fs("wlan"), ota_data_fs("fs")

{
}

CKernel::~CKernel (void)
{
}

static Faux86::VM* _vm = NULL;

uint8_t keys[3] = {0, 0, 0};

rtos::Mutex btn_mtx;

void on_key(uint8_t k1, uint8_t k2, uint8_t k3) {
	if (_vm) {

		for(int n = 0; n < 8; n++)
		{
			int mask = 1 << n;
			bool wasPressed = (keys[0] & mask) != 0;
			bool isPressed = (k1 & mask) != 0;
			if(!wasPressed && isPressed)
			{
				_vm->input.handleKeyDown(modifier2xtMapping[n]);
			}
			else if(wasPressed && !isPressed)
			{
				_vm->input.handleKeyUp(modifier2xtMapping[n]);
			}
		}

/*
		if (k2 != 0) {
			_vm->input.handleKeyDown(usb2xtMapping[k2]);
		} else {
			_vm->input.handleKeyUp(usb2xtMapping[keys[1]]);			
		}
*/

		if (k3 != 0) {
			_vm->input.handleKeyDown(usb2xtMapping[k3]);
		} else {
			_vm->input.handleKeyUp(usb2xtMapping[keys[2]]);			
		}

		//printf("received %x %x %x\n", k1, k2, k3);
		keys[0] = k1;
		keys[1] = k2;
		keys[2] = k3;
	}
}

uint8_t _prev_btn = 0;
uint8_t _prev_x = 0;
uint8_t _prev_y = 0;

void on_mouse(uint8_t btn, int8_t x, int8_t y) {
	if (_vm) {

		//printf("received %x %d %d\n", btn, x, y);

		if (_prev_btn != btn) {
			if (btn & 0x1) {
				//printf("left pressed\n");
				_vm->mouse.handleButtonDown(SerialMouse::ButtonType::Left);
			} else {
				if (_prev_btn & 0x1) {
					//printf("left released\n");
					_vm->mouse.handleButtonUp(SerialMouse::ButtonType::Left);
				}
			}
			if (btn & 0x2) {
				//printf("right pressed\n");
				_vm->mouse.handleButtonDown(SerialMouse::ButtonType::Right);
			} else {
				if (_prev_btn & 0x2) {
					//printf("right released\n");
					_vm->mouse.handleButtonUp(SerialMouse::ButtonType::Right);
				}
			}
		}

		_prev_btn = btn;
		_vm->mouse.handleMove(x, y);
	}
}

void DG_Init();
#include "usb_phy_api.h"

bool CKernel::Initialize ()
{
	bool bOK = true;

	// Video begin
	// USBHOST begin
	get_usb_phy()->deinit();
	NVIC_DisableIRQ(OTG_HS_IRQn);

	DG_Init();

	wifi_data_fs.mount(&wifi_data);
	ota_data_fs.mount(&ota_data);

	RPC.begin();
	delay(100);

	RPC.bind("on_key", on_key);
	RPC.bind("on_mouse", on_mouse);

	if(bOK)
	{
		HostInterface = new CircleHostInterface();
		vmConfig = new Config(HostInterface);
		
		vmConfig->audio.sampleRate = 44100;

		log(Log, "Attempting to load system files\n");
		
		vmConfig->biosFile = HostInterface->openFile("/wlan/pcxtbios.bin");
		vmConfig->videoRomFile = HostInterface->openFile("/wlan/videorom.bin");
		vmConfig->asciiFile = HostInterface->openFile("/wlan/asciivga.dat");
		vmConfig->diskDriveA = HostInterface->openFile("/wlan/dosboot.img");

#if USE_EMBEDDED_ROM_IMAGES
		if(!vmConfig->biosFile)
			vmConfig->biosFile = new EmbeddedDisk(pcxtbios, sizeof(pcxtbios));
		if(!vmConfig->videoRomFile)
			vmConfig->videoRomFile = new EmbeddedDisk(videorom, sizeof(videorom));
		if(!vmConfig->asciiFile)
			vmConfig->asciiFile = new EmbeddedDisk(asciivga, sizeof(asciivga));
		if(!vmConfig->romBasicFile)
			vmConfig->romBasicFile = new EmbeddedDisk(rombasic, sizeof(rombasic));
#endif
#if USE_EMBEDDED_BOOT_FLOPPY
		if(!vmConfig->diskDriveA)
			vmConfig->diskDriveA = new EmbeddedDisk(dosboot, sizeof(dosboot));
#endif

		if(!vmConfig->diskDriveA) {
			auto dos = Faux86::FatFsDisk::open("/fs/dosboot.img");
			vmConfig->diskDriveA = dos;
		}

		if(!vmConfig->diskDriveB) {
			auto bdrive = Faux86::FatFsDisk::open("/fs/disk1.img");
			vmConfig->diskDriveB = bdrive;
		}

		if(!vmConfig->diskDriveC) {
			auto cdrive = Faux86::FatFsDisk::open("/fs/disk2.img");
			vmConfig->diskDriveC = cdrive;
		}

		vmConfig->diskDriveD = new CircleDeviceDisk(&ota_data);

		// TODO
		//vmConfig->parseCommandLine(argc, argv);
		
		log(Log, "Creating VM\n");
		void* place = ea_malloc(sizeof(Faux86::VM));
		memset(place, 0, sizeof(Faux86::VM));

		vm = new (place) Faux86::VM(vmConfig);
		bOK = vm->init(vmConfig);
		_vm = vm;

		log(Log, "Init VM\n");
	}
	
	return bOK;
}

void DG_DrawFrame();
int getNextFrameBuffer();

TShutdownMode CKernel::Run (void)
{
	while (vm->simulate())
	{
		HostInterface->tick(*vm);
		//m_Timer.MsDelay (0);
	}

	return ShutdownHalt;
}

