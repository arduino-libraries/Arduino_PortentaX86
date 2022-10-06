#include "Faux86.h"

UART mySerial(PG_14, PG_9);

REDIRECT_STDOUT_TO(mySerial);

static CKernel Kernel;

void setup() {
  mySerial.begin(115200);
  checkM4Binary();
  Kernel.Initialize(); 
  Kernel.Run();
}

void loop() {
}