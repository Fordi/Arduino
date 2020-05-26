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

  SNES controller pinout:
               ________________
              | 0 0 0 0 | 0 0 0 ) 
               ----------------      
                + C L D   N N G
                V L A A   C C N
                  K T T       D
                    C A
                    H
  
  Shift register output
    (NES) [A, B, Select, Start, Up, Down, Left, Right]
    (SNES) [B, Y, Select, Start, Up, Down, Left, Right, A, X, L, R]
    
Button positions
NES
 ___________________________________________________
|        _ Up                                       |
|Left  _| |_          Select  Start                 | 
|     |_   _| Right   (___)   (___)        (_)  (_) |
|       |_|                                 B    A  |
|        Down                                       |
|___________________________________________________|

SNES
 L                                                 R
     =========_________________________=========
=====     _ Up                                  =====
|Left  _| |_          Select  Start           (_)X  | 
|     |_   _| Right   (___)   (___)         Y   (_) |
|       |_|                                (_)   A  |
\        Down        ______________          B(_)   /
 \_________________/                \______________/

XInput

 5                                                 6
     =========_________________________=========
=====     _ Up                                  =====
|Left  _| |_            9      10             (_)4  | 
|     |_   _| Right   (___)   (___)         1   (_) |
|       |_|                                (_)   3  |
\        Down        ______________          2(_)   /
 \_________________/                \______________/

*/
// Configuration

// Uncomment to pass DPAD info to the X/Y axes.  Leave commented out to map to the Hat.
// #define DPAD_AXIS

// NES/SNES Pins
#define NES_CLK 2
#define NES_LCH 3
#define NES_DAT 4

// If uncommented, this pin has an NES/SNES mode switch.  If not, controller is treated
// as an SNES controller, and an NES controller plugged in will register A/B as B/Y.
// #define MODE_PIN 5

// If uncommented, this pin has a HAT/AXIS mode switch.  If not, controller is treated
// as whatever the default for dpad_mode is below
// #define DPAD_PIN 6


// microseconds to hold a clock pulse.  The datasheet indicates that the shift register can support a clock as high 
// as 3 MHz at 5V, so this could be even lower, theoretically.  All the timings are in nanoseconds as well, so this
// should be fine.  https://www.ti.com/lit/ds/symlink/cd4021b-q1.pdf?ts=1590503512862
#define SHIFT_DELAY 1

#define MODE_NES 0
#define MODE_SNES 1

#define MODE_AXIS 0
#define MODE_HAT 1

// Default to SNES mode, since the NES controller will just spit `1` for the remaining buttons
int mode = MODE_SNES;

// Default to Hat mode.  Switch this to MODE_AXIS if you want that instead.
int dpad_mode = MODE_HAT;

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

int[][] button_map = {
// USB buttons for: A, B, Select, Start  on NES
                  { 3, 2,      9,    10 },
// USB buttons for: B, Y, Select, Start, A, X, LT, RT on SNES
                  { 2, 1,      9,    10, 3, 4,  5,  6 }
};

// Pulse a pin (clock or latch)
void pulse(uint8_t pin) {
  digitalWrite(pin, HIGH);
  delayMicroseconds(SHIFT_DELAY);
  digitalWrite(pin, LOW);
  delayMicroseconds(SHIFT_DELAY);
}

void setup() {
  // configure pins
  pinMode(NES_CLK, OUTPUT);
  pinMode(NES_LCH, OUTPUT);
  pinMode(NES_DAT, INPUT);
  #ifdef MODE_PIN
    pinMode(MODE_PIN, INPUT_PULLUP);
  #endif
  #ifdef DPAD_PIN
    pinMode(DPAD_PIN, INPUT_PULLUP);
  #endif

  digitalWrite(NES_CLK, LOW);      // NES control lines idle low
  digitalWrite(NES_LCH, LOW);

  Joystick.useManualSend(true);     // Do not automatically send USB packet upon state update (decreases latency)
}

void loop() {
  #ifdef MODE_PIN
    mode = digitalRead(MODE_PIN);
  #endif
  #ifdef DPAD_PIN
    dpad_mode = digitalRead(DPAD_PIN);
  #endif
  pulse(NES_LCH);
  // A, B, Select, Start for NES
  // B, Y, Select, Start for SNES
  // `button_map` handles this.
  for (int x = 0; x < 4; x++) { // read in the 4 controller buttons
    Joystick.button(button_map[mode][x], !digitalRead(NES_DAT));
    pulse(NES_CLK);
  }
  uint8_t dpad = 0;
  // Up, Down, Left, Right (`dpad` has bits 3..0 = Right, Left, Down, Up)
  for (int x = 0; x < 4; x++) { // read in the 4 switches for the d-pad state.
    bitWrite(dpad, x, !digitalRead(nesData));
    pulse(NES_CLK);
  }
  if (mode == MODE_SNES) {
    // A, X, L, R for SNES
    for (int x = 4; x < 8; x++) { // read in the remaining 4 controller buttons
      Joystick.button(button_map[mode][x], !digitalRead(NES_DAT));
      pulse(NES_CLK);
    }
  }
  if (dpad_mode == MODE_AXIS) {
    Joystick.X(512 - (bitRead(dpad, 2) * 512 + bitRead(dpad, 3) * 511));
    Joystick.Y(512 - (bitRead(dpad, 0) * 512 + bitRead(dpad, 1) * 511));
    Joystick.hat(-1);
  } else {
    Joystick.X(512);
    Joystick.Y(512);
    Joystick.hat(hat_map[dpad]);
  }
  Joystick.send_now();
}
