#include "Faux86.h"

UART mySerial(PG_14, PG_9);

REDIRECT_STDOUT_TO(mySerial);

void setup() {
  mySerial.begin(115200);
  checkM4Binary();
  static CKernel Kernel;
  Kernel.Initialize(); 
  Kernel.Run();
}

void loop() {
}
