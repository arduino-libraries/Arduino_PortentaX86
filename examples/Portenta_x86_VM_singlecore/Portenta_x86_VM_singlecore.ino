#include "Arduino_PortentaX86.h"
#include "mbed.h"

REDIRECT_STDOUT_TO(Serial);

static CKernel Kernel;
void setup() {
  Serial.begin(115200);
  Kernel.Initialize(false);
  Kernel.Run();
}

void loop() {
}
