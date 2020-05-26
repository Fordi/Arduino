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

// NES Pins
#define NES_CLK 2
#define NES_LCH 3
#define NES_DAT 4

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
  for (int x = 0; x <= 7; x++) { // read in the 8 controller buttons that were latched into the 4021
    Joystick.button(x + 1, digitalRead(NES_DAT));
    pulse(NES_CLK);
  }
  Joystick.send_now();
}
