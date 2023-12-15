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
InterruptIn InterruptOnOff(PA_1);
InterruptIn InterruptRotate(PA_6);

bool volatile _on = false;
bool volatile _rotate = false;

// leds on
void onLEDs(char mask) { leds = leds | mask; }

// leds off
void offLEDs(char mask) { leds = leds & ~mask; }

void setLedOnOff(bool on) {
  offLEDs(3);
  if (on)
    onLEDs(2);
  else
    onLEDs(1);
}

//

bool checkTimeBetweenInterrupts() {
  if (measuredTimeBetweenInterrupts.elapsed_time() > 20ms) {
    measuredTimeBetweenInterrupts.reset();
    return true;
  } else
    return false;
}

void isr_onOff_toggle() {
  if (checkTimeBetweenInterrupts())
    _on = !_on;
  setLedOnOff(_on);
}

void isr_rotate(){

    _rotate = true;
}

void prepareInterupts() {
  // On/Off toggle
  InterruptOnOff.mode(PullDown);
  InterruptOnOff.rise(&isr_onOff_toggle);

  // Rotate

  InterruptRotate.mode(PullDown);
  InterruptRotate.rise(&isr_rotate);
}

// main() runs in its own thread in the OS
int main() {
  setLedOnOff(_on);
  measuredTimeBetweenInterrupts.start();
  prepareInterupts();
  while (true) {
    for (char i = 0; i < 4; i++) {
      if (_rotate) {
        motor = motorCW[i];
        thread_sleep_for(3);
      } else {
        motor = 0;
      }
    }
  }
}
