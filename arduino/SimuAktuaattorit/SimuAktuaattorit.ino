/*

 Arduino code to drive 3 simulator actuators on Arduino Mega

*/

// Represents one actuator
struct Actuator {
  // Pins:
  int endPin; // Inverted, use pulldown resistor
  int pulsePin1;
  int pulsePin2;
  int directionPin;
  int speedPin;
  // Data:
  int direction; // 0 = down, 1 = up
  int endStop;
};

const int ACT_COUNT = 1;
Actuator act[3];

const int ledPin = 13;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
unsigned int loopCounter = 0;

void setup() {
  act[0] = {30, 31, 32, 33, 10, 0 ,0};
  act[1] = {40, 41, 42, 43, 11, 0, 0};
  act[2] = {50, 51, 52, 53, 12, 0, 0};
  for(int a;a<ACT_COUNT;a++) {
      pinMode(act[a].endPin, INPUT);
      pinMode(act[a].pulsePin1, INPUT);
      pinMode(act[a].pulsePin2, INPUT);
      pinMode(act[a].directionPin, OUTPUT);
  }
  // initialize the LED as an output:
  pinMode(ledPin, OUTPUT);
  // initialize serial communication:
  Serial.begin(9600);
}


void loop() {
  for(int a=0;a<ACT_COUNT;a++) {
     act[a].endStop = !digitalRead(act[a].endPin);
     if(act[a].endStop) {
        act[a].direction = 1;
        buttonPushCounter++;
     }
     digitalWrite(act[a].directionPin, act[a].direction);
     analogWrite(act[a].speedPin, 50); // About 1 Volt, should turn motor
  }
  buttonPushCounter++;

  // Debug output
  if(loopCounter > 300) {
    for(int a=0;a<ACT_COUNT;a++) {
        // act[a].direction = !act[a].direction;
        Serial.println("----------------");
        Serial.print("Actuator ");
        Serial.print(a);
        Serial.println(" status:");
        Serial.print("Direction: ");
        Serial.println(act[a].direction);
        Serial.print("End stop: ");
        Serial.println(act[a].endStop);
    }
    loopCounter = 0;
  }

  // Blinkenlichts
  if (buttonPushCounter % 4 == 0) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  loopCounter++;
  delay(5);
}
