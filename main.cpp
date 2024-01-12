#include "mbed.h"

// LCD header file
#include "LCD.h"

// Chrono for Timers
#include <algorithm>
#include <chrono>
#include <rt_sys.h>
using namespace std::chrono;

#define MOTOR_STOP 50
#define MOTOR_SUPER_SLOW 45
#define MOTOR_SLOW 40
#define MOTO_MEDIUM 20
#define MOTO_FAST 10
#define MOTOR_SUPER_FAST 5

#define TIME_SPEED_CHANGE 100ms
#define TIME_SPEED_WALK_LIGHT 500ms
#define WALK_LIGHT_SIZE 6

unsigned char const walkLight[] = {0b1, 0b10, 0b100, 0b1000, 0b10000, 0b100000};
unsigned int const motorCW[] = {0x300, 0x600, 0xc00, 0x900};

// Interrupts
InterruptIn InterruptOnOff(PA_1);
InterruptIn InterruptRotate(PA_6);
InterruptIn InterruptEmergency(PA_10);

lcd mylcd;
Timer measuredTimeBetweenInterrupts;
Ticker tickerSpeedControl;
Ticker tickerWalkLight;

Timeout timeouts[4];

PortOut motor(PortC, 0xf00);
PortOut leds(PortC, 0xff);

DigitalIn modeSelect[] = {PB_0, PB_1, PB_2};

bool volatile _on = false;
bool volatile _rotate = false;
bool volatile _emergency = false;
bool volatile _offAfterStop = false;

char _mode = 0;
char _speed = MOTOR_SUPER_FAST;
char _newSpeed;
char _walkLightIndex = 0;

void lcdClear() {
  mylcd.clear();
  mylcd.printf("                ");
}

// leds on
void onLEDs(char mask) { leds = leds | mask; }

// leds off
void offLEDs(char mask) { leds = leds & ~mask; }

char getLEDs(char mask = 0xff) { return leds & mask; }

void setLedOnOff(bool on) {
  offLEDs(3);
  if (on)
    onLEDs(2);
  else
    onLEDs(1);
}

void setWalkLight(char mask) {
  onLEDs(mask << 2);
  offLEDs(~mask << 2);
}

void tickWalkLight() {
  if (_rotate && _on) {
    setWalkLight(walkLight[_walkLightIndex]);
    if (_walkLightIndex == WALK_LIGHT_SIZE - 1)
      _walkLightIndex = 0;
    else
      _walkLightIndex++;
  } else {
    setWalkLight(0);
  }
}

//

bool checkTimeBetweenInterrupts() {
  if (measuredTimeBetweenInterrupts.elapsed_time() > 20ms) {
    measuredTimeBetweenInterrupts.reset();
    return true;
  } else
    return false;
}

void tickChangeSpeed() {
  if (_speed == _newSpeed) {
    return;
  } else if (_speed <= _newSpeed) {
    _speed++;
  } else if (_speed >= _newSpeed) {
    _speed--;
  }
}

void changeSpeed(char newSpeed) { _newSpeed = newSpeed; }

void slowStop() {
  changeSpeed(MOTOR_STOP);
}

void modeToddler() {
  _speed = MOTOR_SUPER_SLOW;
  changeSpeed(MOTOR_SUPER_FAST);
  timeouts[0].attach(&slowStop, 20s);
}

void modeKids() {}

void modeAction() {}

void isr_onOff_toggle() {
  if (checkTimeBetweenInterrupts()) {
    if (_on) {
      slowStop();
      _offAfterStop = true;
      setLedOnOff(_on);
    } else {
      _on = true;
      setLedOnOff(_on);
    }
  }
}

void startRotate() {
  if (_rotate)
    return;
  _rotate = true;
  if (modeSelect[0])
    modeToddler();
  if (modeSelect[1])
    modeKids();
  if (modeSelect[2])
    modeAction();
}

void isr_rotate() {
  if (_on && _rotate == false) {
    startRotate();
  }
}

void isr_emergency() { _emergency = true; }

void prepareInterupts() {
  // On/Off toggle
  InterruptOnOff.mode(PullDown);
  InterruptOnOff.rise(&isr_onOff_toggle);

  // Rotate
  InterruptRotate.mode(PullDown);
  InterruptRotate.rise(&isr_rotate);

  // Emergency
  InterruptEmergency.mode(PullDown);
  InterruptEmergency.rise(&isr_emergency);
}

void emergency() {
  motor = 0;
  tickerSpeedControl.detach();
  tickerWalkLight.detach();
  setWalkLight(0);
  InterruptEmergency.disable_irq();
  InterruptOnOff.disable_irq();
  InterruptRotate.disable_irq();
  mylcd.clear();
  mylcd.printf("     NOTHALT    ");
  while (true) {
    setLedOnOff(getLEDs(1));
    thread_sleep_for(200);
  }
}

// main() runs in its own thread in the OS
int main() {
  lcdClear();
  setLedOnOff(_on);
  measuredTimeBetweenInterrupts.start();
  tickerSpeedControl.attach(&tickChangeSpeed, TIME_SPEED_CHANGE);
  tickerWalkLight.attach(&tickWalkLight, TIME_SPEED_WALK_LIGHT);
  prepareInterupts();
  while (true) {
    if (_emergency) {
      emergency();
    }
    for (char i = 0; i < 4; i++) {
      if (_emergency) {
        break;
      }
      if (_speed == MOTOR_STOP) {
        _rotate = false;
        if(_offAfterStop){
            _offAfterStop = false;
            _on = false;
            setLedOnOff(false);
        }
      }
      if (_rotate) {
        motor = motorCW[i];
        thread_sleep_for(_speed);
      } else {
        motor = 0;
      }
    }
  }
}
