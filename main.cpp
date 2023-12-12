#include "mbed.h"

// LCD header file
#include "LCD.h"

// Chrono for Timers
#include <chrono>

using namespace std::chrono;

PortOut motor(PortC, 0xf00);
PortOut leds(PortC, 0xff);

unsigned int motorCW[] = {0x300, 0x600, 0xc00, 0x900};

Timer measuredTimeBetweenInterrupts;

// Interrupts
InterruptIn onOff(PA_1);

bool volatile _on = false;

bool checkTimeBetweenInterrupts() {
  if (measuredTimeBetweenInterrupts.elapsed_time() > 10ms) {
    measuredTimeBetweenInterrupts.reset();
    return true;
  } else
    return false;
}

void isr_onOff_toggle() {
  if (checkTimeBetweenInterrupts())
    _on = !_on;
}

void prepareInterupts() {
  // On/Off toggle
  onOff.mode(PullDown);
  onOff.rise(&isr_onOff_toggle);
  onOff.enable_irq();
}

// main() runs in its own thread in the OS
int main() {
  measuredTimeBetweenInterrupts.start();
  prepareInterupts();
  while (true) {
    for (char i = 0; i < 4; i++) {
      if (_on) {
        motor = motorCW[i];
        thread_sleep_for(3);
      } else {
        motor = 0;
      }
    }
  }
}
