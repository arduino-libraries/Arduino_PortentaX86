#include "Arduino_PortentaX86.h"
#include "mbed.h"

UART mySerial(PG_14, PG_9);

REDIRECT_STDOUT_TO(mySerial);

static CKernel Kernel;
void setup() {
  mySerial.begin(115200);
  Kernel.Initialize(false);
  Kernel.Run();
}

void loop() {
}
