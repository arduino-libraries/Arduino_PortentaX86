#include "Faux86.h"

void setup() {
  CKernel Kernel;
  if (!Kernel.Initialize ())
  {
    return EXIT_HALT;
  }

  TShutdownMode ShutdownMode = Kernel.Run ();
}

void loop() {
}
