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
#pragma once

#include "faux86/DriveManager.h"
#include "FATFileSystem.h"

namespace Faux86
{
	class FatFsDisk : public DiskInterface
	{
	private:
		FILE* file;
		FatFsDisk(FILE* inFile) : file(inFile) {}

	public:
		static FatFsDisk* open(const char* path)
		{			
			log(Log, "Attempting to open %s for read", path);

			FILE* file = fopen(path, "rb");
			if (file != NULL)
			{
				log(Log, "Success!");
				return new FatFsDisk(file);
			}
			
			log(Log, "Could not open file %s", path);
			
			return nullptr;
		}
		
		virtual int read (uint8_t *buffer, unsigned count) override
		{
			unsigned bytesRead = fread(buffer, 1, count, file);
			return bytesRead;
		}
		virtual int write (const uint8_t *buffer, unsigned count) override
		{
			unsigned bytesWritten = fwrite(buffer, 1, count, file);
			return bytesWritten;
		}
		virtual uint64_t seek (uint64_t offset) override
		{
			fseek(file, offset, SEEK_SET);
			return offset;
		}
		virtual uint64_t getSize() override
		{
    		fseek(file, 0, SEEK_END);
    		int size = ftell(file);
    		fseek(file, 0, SEEK_SET);
    		return size;
		}
		virtual bool isValid() override
		{
			return true;
		}
	};
}
