#if defined (ARDUINO_PORTENTA_H7_M7)
#include "FlashIAP.h"
#include "fw.h"

#define M4_FW_ADDR   (0x8100000)
mbed::FlashIAP flash;

void applyUpdate(uint32_t address)
{
  long len = m4_fw_bin_len;

  flash.init();

  const uint32_t page_size = flash.get_page_size();
  char *page_buffer = new char[page_size];
  uint32_t addr = address;
  uint32_t next_sector = addr + flash.get_sector_size(addr);
  bool sector_erased = false;
  size_t pages_flashed = 0;
  uint32_t percent_done = 0;

  while (true) {

    if (page_size * pages_flashed > len) {
      break;
    }

    // Erase this page if it hasn't been erased
    if (!sector_erased) {
      flash.erase(addr, flash.get_sector_size(addr));
      sector_erased = true;
    }

    // Program page
    flash.program(&m4_fw_bin[page_size * pages_flashed], addr, page_size);

    addr += page_size;
    if (addr >= next_sector) {
      next_sector = addr + flash.get_sector_size(addr);
      sector_erased = false;
    }

    if (++pages_flashed % 3 == 0) {
      uint32_t percent_done_new = page_size * pages_flashed * 100 / len;
      if (percent_done != percent_done_new) {
        percent_done = percent_done_new;
      }
    }
  }

  delete[] page_buffer;

  flash.deinit();
}

void checkM4Binary() {
  // compare byte to byte M4_FW_ADDR and m4_fw_bin
  bool differs = false;
  for (int i = 0; i < m4_fw_bin_len; i++) {
    if (m4_fw_bin[i] != *((uint8_t*)M4_FW_ADDR+i)) {
      differs = true;
      break;
    }
  }

  if (differs) {
    printf("Reflashing USBHost stack on M4 core\n");
    applyUpdate(M4_FW_ADDR);
  }
}

#endif
