/*

 Arduino code to drive 3 simulator actuators on Arduino Mega

*/

// Represents one actuator
struct Actuator {
  // Arduino pins:
  int endPin; // Inverted, use pulldown resistor
  int pulsePin1;
  int pulsePin2;
  int directionPin;
  int speedPin;
  // Data:
  int enabled; // 0 = disabled
  int direction; // 0 = down, 1 = up
  int speed; // 0-255, speed command to move to direction
  int activeDirection; // 0 = down, 1 = up
  int endStop; // 1 = end stop active
  int homed; // Actuator position is known 
  int maxPosition; // Maximum position value for this actuator
  volatile int pulseBits; // bit 0 = pin 1, bit 1 = pin 2
  int position; // In pulses, see maxPosition. Invalid when homing. 0 = full down
  int velocity; // Measured velocity, positive down
  volatile int velocityCounter; // Counts pulses for velocity 
};

const int ACT_COUNT = 3; // Number of actuators
Actuator act[ACT_COUNT];

const int ledPin = 13;       // the pin that the LED is attached to

unsigned int loopCounter = 0;

// Speeds, in speed units (0-255)
const int defaultSpeed = 100;
const int homingSpeed = 60;

int systemReady = 0; // 1 = all homed

void setup() {
  // initialize serial communication:
  Serial.begin(9600);

  // for  PWM frequency of 3921.16 Hz D9 & D10
  // TCCR2B = TCCR2B & B11111000 | B00000011;  
  
  // for PWM frequency of 3921.16 Hz D11 & D12
  // TCCR1B = TCCR1B & B11111000 | B00000011; 
  //        e   p1  p2  d   s   e  d  s   ad e  h  mp pb po ve vc
  act[0] = {30,  2,  3, 33, 10, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0};
  act[1] = {40, 18, 19, 43, 11, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0};
  act[2] = {50, 20, 21, 53, 12, 1, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0};

  act[0].maxPosition = 3100;
  act[1].maxPosition = 3100;
  act[2].maxPosition = 3100;
  
  for(int a=0;a<ACT_COUNT;a++) {
    if(act[a].enabled) {
      pinMode(act[a].endPin, INPUT);
      pinMode(act[a].pulsePin1, INPUT_PULLUP);
      pinMode(act[a].pulsePin2, INPUT_PULLUP);
      pinMode(act[a].directionPin, OUTPUT);
      pinMode(act[a].speedPin, OUTPUT);
      Serial.print("Actuator ");
      Serial.print(a);
      Serial.println(" initialized");
    }
  }

  attachInterrupt(digitalPinToInterrupt(act[0].pulsePin1), pinInterrupt0_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(act[0].pulsePin2), pinInterrupt0_2, CHANGE);

  attachInterrupt(digitalPinToInterrupt(act[1].pulsePin1), pinInterrupt1_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(act[1].pulsePin2), pinInterrupt1_2, CHANGE);

  attachInterrupt(digitalPinToInterrupt(act[2].pulsePin1), pinInterrupt2_1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(act[2].pulsePin2), pinInterrupt2_2, CHANGE);
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);

  cli();
// Timer:
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TCNT0  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR0A = (16*10^6) / (10*64) - 1;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  sei();
}

int printedTurns = 0;

// "Normal" operation loop for actuator a
// Actuator has been homed and system is running
void normalLoop(int a) {
   // Normal operations
   act[a].speed = defaultSpeed;
   /*
   if(act[a].position >= act[a].maxPosition)
      act[a].direction = 0; // Drive down
      */
   if(act[a].position >= 2000) {
      act[a].direction = 0; // Drive down
      act[a].speed = 0;
   }
   if(act[a].position < 10)
      act[a].direction = 1; // Drive up  
}

// Homing actuator a
void homingLoop(int a) {
   if(act[a].endStop) {
      act[a].direction = 1;
      act[a].position = 0;
      if(!act[a].homed) {
        act[a].homed = 1;
        act[a].speed = 0; // Wait down until all homed
        Serial.print("*** Actuator homed: ");
        Serial.println(a);
      }
   } else {
      act[a].direction = 0;
      act[a].speed = homingSpeed;    
   }
}

void loop() {
  // Logic to test if system is ready
  int allReady = 1;
  if(!systemReady) {
    for(int a=0;a<ACT_COUNT;a++) {
       if(act[a].enabled && !act[a].homed) allReady = 0;
    }
  }

  // Loop each actuator
  for(int a=0;a<ACT_COUNT;a++) {
     if(act[a].enabled) {
       act[a].endStop = !digitalRead(act[a].endPin);
       if(systemReady) {
         normalLoop(a);
       } else {
         if(!act[a].homed) 
           homingLoop(a);        
       }
       if(act[a].endStop) {
          act[a].direction = 1;
          act[a].position = 0;
       }
       digitalWrite(act[a].directionPin, act[a].direction);
       if(act[a].direction == act[a].activeDirection) {
         analogWrite(act[a].speedPin, act[a].speed);
       } else {
         analogWrite(act[a].speedPin, 0);
         if(act[a].velocity == 0) {
            act[a].activeDirection = act[a].direction;
            Serial.print("*** Safe direction change ");
            Serial.println(a);
         }
       }
     }
  }
  
  // Logic to test if system is ready
  if(!systemReady && allReady) {
     systemReady = 1;
     Serial.println("*** All homed, system ready!");
  }

  // Debug output
  if(loopCounter > 9000) {
    if(systemReady) {
      Serial.println("---------------- System ready");
    } else {
      Serial.println("---------------- HOMING...");
    }
    for(int a=0;a<ACT_COUNT;a++) {
      if(act[a].enabled) {
        Serial.println("----------------");
        Serial.print("Actuator ");
        Serial.print(a);
        if(act[a].homed) {
          Serial.println(" homed");
        } else {
          Serial.println(" homing...");
        }
        Serial.print("Position: ");
        Serial.println(act[a].position);
        /*
        Serial.print("Direction: ");
        Serial.println(act[a].direction);
        Serial.print("Velocity: ");
        Serial.println(act[a].velocity);
        Serial.print("Speed cmd: ");
        Serial.println(act[a].speed);
        Serial.print("End stop: ");
        Serial.println(act[a].endStop);
        */
      }
    }
    loopCounter = 0;
  }
  
  digitalWrite(ledPin, systemReady);
  loopCounter++;
}

// dir 0 = 00 10 11 01
// dir 1 = 00 01 11 10

// Return index of gray value, -1 if invalid
int grayIndex(int value) {
  int gray[] = {0, 1, 3, 2};
  for(int i=0;i <= 3;i++) {
    if(value == gray[i])
      return i;
  }
  Serial.print("Invalid gray:");
  Serial.print(value);
  Serial.println();
  return -1;
}

/* Convert direction gray code from -> to
Returns:
 from == to -> 0 -> No motion
  0  1  2  3
 00 01 11 10 -> 1
 00 10 11 01 -> -1
 2 steps -> -2 -> ERROR
 invalid values -> -3 -> ERROR
*/
int grayDirection(int from, int to) {
  if(from == to) return 0;
  int fromi = grayIndex(from);
  int toi = grayIndex(to);

  if(fromi < 0 || toi < 0) return -3;
  
  int diffi = toi - fromi;

  if(diffi == 1 || diffi == -1) {
    return diffi;
  }
  // Ends.. 0 -> 3
  if(diffi == 3)
    return -1;
  // 3 -> 0
  if(diffi == -3)
    return 1;

  return -2;
}

void updatePin(int actuator, int pin) {
  volatile int oldPulse = act[actuator].pulseBits;
  int pp1 = digitalRead(act[actuator].pulsePin1);
  int pp2 = digitalRead(act[actuator].pulsePin2);
  int newPulse = pp1 | pp2 << 1;
  int dir = grayDirection(oldPulse, newPulse);
  act[actuator].pulseBits = newPulse;
  if(dir == 1 || dir == -1) {
    act[actuator].position -= dir;
    act[actuator].velocityCounter += dir;
    // Serial.print(act[actuator].velocityCounter);
  }
  if(dir == 1 || dir == -1 || dir == 0) return; 
  /* 
  Serial.print("Actuator ");
  Serial.print(actuator);
  Serial.print("pin ");
  Serial.print(pin);
  Serial.print(" from:");
  Serial.print(oldPulse, BIN);
  Serial.print(" to:");
  Serial.print(newPulse, BIN);
  Serial.print(" pins:");
  Serial.print(pp2);
  Serial.print(pp1);
  Serial.print(" dir:");
  Serial.print(dir);
  Serial.println();
  */
}

// Position interrupts for all actuators
void pinInterrupt0_1() {
  updatePin(0, 1);
}

void pinInterrupt0_2() {
  updatePin(0, 2);
}

void pinInterrupt1_1() {
  updatePin(1, 1);
}

void pinInterrupt1_2() {
  updatePin(1, 2);
}

void pinInterrupt2_1() {
  updatePin(2, 1);
}

void pinInterrupt2_2() {
  updatePin(2, 2);
}


// Timer interrupt for velocity counting
volatile int timerInterrupts = 0;

ISR(TIMER0_COMPA_vect){
  timerInterrupts++;
  if(timerInterrupts < 256) return;
  
  for(int a=0;a<ACT_COUNT;a++) {
    if(act[a].enabled) {
       act[a].velocity = act[a].velocityCounter;
       act[a].velocityCounter = 0;
    }
  }
  timerInterrupts = 0;
}
