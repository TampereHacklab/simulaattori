/*

 Arduino code to drive 3 simulator actuators on Arduino Mega

*/

// Represents one actuator
struct Actuator {
  int endPin;
  int pulsePin1;
  int pulsePin2;
  int directionPin;
  int direction; // 0 = down, 1 = up
  int endStop;
};

const int ACT_COUNT = 3;
Actuator act[ACT_COUNT];

const int ledPin = 13;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
unsigned int loopCounter = 0;

void setup() {
  act[0] = {2, 3, 4, 11, 0, 0};
  act[1] = {5, 6, 7, 12, 0, 0};
  act[2] = {8, 9, 10, 22, 0, 0};
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
     act[a].endStop = digitalRead(act[a].endPin);
     if(act[a].endStop) {
        act[a].direction = 1;
        buttonPushCounter++;
     }
     digitalWrite(act[a].directionPin, act[a].direction);
  }
  buttonPushCounter++;

  // Debug output
  if(loopCounter > 100) {
    for(int a=0;a<ACT_COUNT;a++) {
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
