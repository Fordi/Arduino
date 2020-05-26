/*
  NES controller pinout:
               _________
             /          |
            /        O  | Ground
           /            |
     +VCC  |  O      O  | Clock
           |            |
     N.C.  |  O      O  | Latch
           |            |
     N.C.  |  O      O  | Data Out (To NES)
           |____________|
*/

#define SHIFT_DELAY 20 // microseconds to hold a clock pulse

#define DPAD_AXIS

// NES Pins
#define NES_CLK 2
#define NES_LCH 3
#define NES_DAT 4

#ifndef DPAD_AXIS
  // Joystick.hat accepts degrees clockwise from `up`, in 45 degree increments, or -1 for centered.
  int hat_map[] = [
    /* RLDU */
    /* 0000 */ -1,
    /* 0001 */ 0,
    /* 0010 */ 180,
    /* 0011 */ -1, // Up + Down = illegal; ignore vertical
    /* 0100 */ 270,
    /* 0101 */ 315,
    /* 0110 */ 225,
    /* 0111 */ 270, // Up + Down = illegal; ignore vertical
    /* 1000 */ 90,
    /* 1001 */ 45,
    /* 1010 */ 135,
    /* 1011 */ 90,  // Up + Down = illegal; ignore vertical
    /* 1100 */ -1, // Left + Right = illegal; ignore horizontal
    /* 1101 */ 0, // Left + Right = illegal; ignore horizontal
    /* 1110 */ 180, // Left + Right = illegal; ignore horizontal
    /* 1111 */ -1 // ignore all
  ];
#endif
// USB buttons for: A, B, Select, Start  
int[] button_map = [2, 3,      9,    10];

// Pulse a pin (clock or latch)
void pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(shiftDelay);
  digitalWrite(pin, LOW);
}

void setup() {
  // configure pins
  pinMode(NES_CLK, OUTPUT);
  pinMode(NES_LCH, OUTPUT);
  pinMode(NES_DAT, INPUT);

  digitalWrite(NES_CLK, LOW);      // NES control lines idle low
  digitalWrite(NES_LCH, LOW);

  Joystick.useManualSend(true);     // Do not automatically send USB packet upon state update (decreases latency)
}

void loop() {
  pulse(NES_LCH);
  // A, B, Select, Start
  for (int x = 0; x < 4; x++) { // read in the 4 controller buttons that were latched into the 4021
    Joystick.button(button_map[x], digitalRead(NES_DAT));
    pulse(NES_CLK);
  }
  uint8_t dpad = 0;
  // Up, Down, Left, Right (`dpad` has bits 3..0 = Right, Left, Down, Up)
  for (int x = 0; x < 4; x++) { // read in the 4 switches for the d-pad state.
    bitWrite(dpad, x, !digitalRead(nesData));
  }
  #ifndef DPAD_AXIS
    Joystick.hat(hat_map[dpad]);
  #else
    Joystick.X(512 - (bitRead(dpad, 2) * 512 + bitRead(dpad, 3) * 511));
    Joystick.Y(512 - (bitRead(dpad, 0) * 512 + bitRead(dpad, 1) * 511));
  #endif
  Joystick.send_now();
}
