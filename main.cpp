#include "mbed.h"

PortOut motor(PortC, 0xf00);
unsigned int motorCW[] = {0x300, 0x600, 0xc00, 0x900};

// main() runs in its own thread in the OS
int main()
{
    while (true) {

        for(char i = 0; i<sizeof(motorCW); i++){
            motor = motorCW[i];
            thread_sleep_for(2);
        }

    }
}
            