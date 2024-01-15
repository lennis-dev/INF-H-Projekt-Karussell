#include "mbed.h"

// LCD header file
#include "LCD.h"

// Define motor speeds
#define MOTOR_STOP 50
#define MOTOR_SUPER_SLOW 45
#define MOTOR_SLOW 40
#define MOTOR_MEDIUM 20
#define MOTOR_FAST 10
#define MOTOR_SUPER_FAST 5

// Define time intervals
#define TIME_SPEED_CHANGE 100ms
#define TIME_SPEED_WALK_LIGHT 250ms
#define WALK_LIGHT_SIZE 6

// Define patterns for walk light and motor rotation
unsigned char const walkLight[] = {0b1, 0b10, 0b1000, 0b100000, 0b10000, 0b100};
unsigned int const motorCW[] = {0x300, 0x600, 0xc00, 0x900};

// Define interrupts for on/off switch, rotation, and emergency stop
InterruptIn InterruptOnOff(PA_1);
InterruptIn InterruptRotate(PA_6);
InterruptIn InterruptEmergency(PA_10);

// Create a LCD object
lcd mylcd;
Timer measuredTimeBetweenInterrupts;
Ticker tickerSpeedControl;
Ticker tickerWalkLight;

Timeout timeouts[5];

// Define ports for motor and LEDs
PortOut motor(PortC, 0xf00);
PortOut leds(PortC, 0xff);

// Define digital inputs for mode selection
DigitalIn modeSelect[] = {PB_0, PB_1, PB_2};

// Define volatile variables for on/off state, rotation state, emergency state,
// and off after stop state
bool volatile _on = false;
bool volatile _rotate = false;
bool volatile _emergency = false;
bool volatile _offAfterStop = false;

// Define variables for mode, speed, new speed, and walk light index
char _mode = 0;
char _speed = MOTOR_SUPER_FAST;
char _newSpeed;
char _walkLightIndex = 0;

// Function to clear the LCD
void lcdClear() {
  mylcd.clear();
  mylcd.printf("                ");
}

// Function to turn on LEDs
void onLEDs(char mask) { leds = leds | mask; }

// Function to turn off LEDs
void offLEDs(char mask) { leds = leds & ~mask; }

// Function to get the state of LEDs
char getLEDs(char mask = 0xff) { return leds & mask; }

// Function to set the on/off LED
void setLedOnOff(bool on) {
  offLEDs(3);
  if (on)
    onLEDs(2);
  else
    onLEDs(1);
}

// Function to set the walk light
void setWalkLight(char mask) {
  onLEDs(mask << 2);
  offLEDs(~mask << 2);
}

// Function to update the walk light
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

// Function to check the time between interrupts
bool checkTimeBetweenInterrupts() {
  if (measuredTimeBetweenInterrupts.elapsed_time() > 20ms) {
    measuredTimeBetweenInterrupts.reset();
    return true;
  } else
    return false;
}

// Function to update the speed
void tickChangeSpeed() {
  if (_speed == _newSpeed) {
    return;
  } else if (_speed <= _newSpeed) {
    _speed++;
  } else if (_speed >= _newSpeed) {
    _speed--;
  }
}

// Function to change the speed
void changeSpeed(char newSpeed) { _newSpeed = newSpeed; }

// Function to slow stop
void slowStop() { changeSpeed(MOTOR_STOP); }

// Function to change speed to slow
void changeSpeedSlow() { changeSpeed(MOTOR_SLOW); }

// Function to change speed to medium
void changeSpeedMedium() { changeSpeed(MOTOR_MEDIUM); }

// Function to change speed to fast
void changeSpeedFast() { changeSpeed(MOTOR_FAST); }

// Function to set toddler mode
void modeToddler() {
  _speed = MOTOR_SUPER_SLOW;
  changeSpeed(MOTOR_SLOW);
  timeouts[0].attach(&slowStop, 3min);
  mylcd.clear();
  mylcd.printf("     Toddler    ");
}

// Function to set kids mode
void modeKids() {
  _speed = MOTOR_SUPER_SLOW;
  changeSpeed(MOTOR_SLOW);
  timeouts[0].attach(&changeSpeedMedium, 30s);
  timeouts[1].attach(&changeSpeedSlow, 150s);
  timeouts[2].attach(&slowStop, 180s);
  mylcd.clear();
  mylcd.printf("      Kids      ");
}

// Function to set action mode
void modeAction() {
  _speed = MOTOR_SUPER_SLOW;
  changeSpeed(MOTOR_SLOW);
  timeouts[0].attach(&changeSpeedMedium, 10s);
  timeouts[1].attach(&changeSpeedFast, 30s);
  timeouts[2].attach(&changeSpeedMedium, 150s);
  timeouts[3].attach(&changeSpeedSlow, 170s);
  timeouts[4].attach(&slowStop, 3min);
  mylcd.clear();
  mylcd.printf("     Action     ");
}

// Interrupt service routine for on/off toggle
void isr_onOff_toggle() {
  InterruptOnOff.disable_irq();
  if (checkTimeBetweenInterrupts()) {
    if (_on) {
      timeouts[0].detach();
      timeouts[1].detach();
      timeouts[2].detach();
      timeouts[3].detach();
      timeouts[4].detach();
      if (!_rotate) {
        _on = false;
        setLedOnOff(_on);
        lcdClear();
      } else {
        slowStop();
        _offAfterStop = true;
      }
    } else {
      _on = true;
      setLedOnOff(_on);
    }
  }
  InterruptOnOff.enable_irq();
}

// Interrupt service routine for rotation
void isr_rotate() {
  if (_on && _rotate == false) {
    _rotate = true;
    if (modeSelect[0])
      modeToddler();
    else if (modeSelect[1])
      modeKids();
    else if (modeSelect[2])
      modeAction();
    else
      _rotate = false;
  }
}

// Interrupt service routine for emergency stop
void isr_emergency() { _emergency = true; }

// Function to prepare interrupts
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

// Function to handle emergency stop
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
  setLedOnOff(_on);
  measuredTimeBetweenInterrupts.start();
  tickerSpeedControl.attach(&tickChangeSpeed, TIME_SPEED_CHANGE);
  tickerWalkLight.attach(&tickWalkLight, TIME_SPEED_WALK_LIGHT);
  prepareInterupts();
  lcdClear();
  while (true) {
    if (_emergency) {
      emergency();
    }
    for (char i = 0; i < 4; i++) {
      if (_emergency) {
        break;
      }
      if (_speed == MOTOR_STOP) {
        _speed = MOTOR_STOP + 1;
        _newSpeed = MOTOR_STOP + 1;
        _rotate = false;
        if (_offAfterStop) {
          _offAfterStop = false;
          _on = false;
          setLedOnOff(false);
        }
        lcdClear();
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