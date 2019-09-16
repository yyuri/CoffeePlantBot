# The coffee plantbot - A coffee machine that grow strawberries


Coffee Plantbot is a lil arduino program to turn a coffee machine into a device that care of plants

## Installation

### Materials:

- A Cheap mositure sensor
- LED lighting
- I2C OLED 0.96'' screen
- 3 Relays
- Arduino board (I used nano)

### Instalation:

Connect the relays to:   
  - The water pump
  - The growing lights
  - And the moisture sensor

## Code:

- It uses Switch_lib to control relays and Adafruit library for the 0.96'' OLED ( You'll need to have it downloaded on the Arduino library manager. )

- Adapt the PIN definition to your needs:
```C
// ANALOG INPUTS  PIN DEFINITION//

int moisture_sensor = A2;

// DIGITAL INPUTS  PIN DEFINITION//

int lights_p = 11;  // Lights relay
int r1_p = 10;      // R1 relay
int r2_p = 9;       // R2 relay
int moisture_p = 8; // Hsensor relay
```
- Conect the OLED screen to the I2C ports. In my Arduino nano were A4 and A5

### First run:

Once all connected, during the setup the program will do some test as follow:

- Turn on the Screen
- Turn ON/OFF the lights
- Turn ON/OFF the pump
- Get first sensor read

If all of this goes well the program will start to work.

