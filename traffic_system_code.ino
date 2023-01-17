/* Name: Shreyansh Thakral 
   Date: April 15, 2021
   Teacher: Mr. Wong
   This program is designed to run the systems at and around, a T-intersection. Subsystems include: 
   A Traffic light system (with traffic lights, a crossing button, pedestran lights, a street light and  an LDR) and a Parking Gate system (with a servo motor, buzzer, ultrasonic sensor, IR sensor and IR remote)
*/
#include <Servo.h>
#include <IRremote.h>

int trafficCount, gateCount, secondTimer; // global counter variables
long sensorDist; // variable to hold obj dist. from ultrasonic sensor (in cm)
boolean modifiedCycle = false; // indicates whether modified traffic light cycle is in affect
boolean buttonPressed = false; // stores whether button has been recently pressed (since the last modified cycle ended)
boolean inGateSequence = false; // indicates whether the gate sequence is in affect
boolean overrideDetected = false; // indicates whether an override is occuring
Servo myservo; // create new Servo obj.
IRrecv irrecv(3); // create new IR reciever obj. and indicate the pin
decode_results results; // variable to hold codes recieved by IR sensor
  
const int trafficR1 = 7; // the synchronus red traffic lights pin
const int trafficY1 = 6;  // the synchronus yellow traffic lights pin
const int trafficG1 = 5;  // the synchronus green traffic lights pin

const int trafficR2 = 10; // the asynchronus red traffic light pin
const int trafficY2 = 9; // the asynchronus yellow traffic light pin
const int trafficG2 = 8; // the asynchronus green traffic light pin

const int pedestrianR = 12; // the pedestrian red light pin
const int pedestrianG = 11; // the pedestrian green light pin
const int pushButton = 4; // pin for button to change the light timing

const int streetlight = 13; // pin for street light (white led)
const int photoresistor = A0; // pin for street light (white led)

const int ultrasonic = A2; // pin for ultrasonic sensor
const int buzzer = 2; // pin for buzzer
const int servoPin = A1; // pin for servo motor


void setup()
{
  pinMode(trafficR1, OUTPUT);
  pinMode(trafficY1, OUTPUT);
  pinMode(trafficG1, OUTPUT);
  pinMode(trafficR2, OUTPUT);
  pinMode(trafficY2, OUTPUT);
  pinMode(trafficG2, OUTPUT);
  pinMode(pedestrianR, OUTPUT);
  pinMode(pedestrianG, OUTPUT);
  pinMode(pushButton, INPUT);
  pinMode(photoresistor, INPUT);
  pinMode(streetlight, OUTPUT);
  pinMode(buzzer, OUTPUT);
  irrecv.enableIRIn();
  myservo.attach(servoPin);
  myservo.write(0);
  Serial.begin(9600);
}

void loop()
{
  if (digitalRead(pushButton) == 1) // if button is pressed
  {
    buttonPressed = true; // update boolean
  }
  
  if (modifiedCycle) 
    trafficLightsMod(); // modified cycle is in effect
  else 
    trafficLightsReg(); // modified cycle is not in effect
  
  if (secondTimer == 0 || secondTimer == 500 ) // every half second: update ultrasonic sensor distance and check for override 
  {
  	updateSensorDist();
    updateOverrideDetected();
    if (((sensorDist >= 57 && sensorDist <= 87)|| overrideDetected) && inGateSequence == false) // check whether a gate sequence needs to be started (range is skewd to conteract consistent interference in simulation)
      inGateSequence = true;  
  }
  else if (secondTimer == 1000) // one second has passed, reset secondTimer
    secondTimer = -1;
    
  if (inGateSequence)
  	gateSequence();
    
  updateStreetlight();
  delay(1); // 1 millisecond delay
  trafficCount++; // increment trafficCount
  secondTimer++; // increment second timer
}

void trafficLightsReg ()
{
  // use if-blocks to carry out changes at specific times in the cycle
  if (trafficCount < 3000 && buttonPressed)   // if button is pressed before green light comes on
  {
    if (trafficCount > 750) // button is still pressed before green light and should switch to modified cycle, however, in modified cycle asynchronus yellow light would already be on at this point, thus set counter to 750 so that full modified duration is given to the upcoming asynchronous yellow and green lights 
      trafficCount = 749;
    modifiedCycle = true; // switch to modified cycle
  }
  else if (trafficCount == 0)
  {   
    digitalWrite(trafficR1, HIGH);
    digitalWrite(trafficG2, HIGH);
    digitalWrite(pedestrianR, HIGH);
    digitalWrite(pedestrianG, LOW);
  }
  
  else if (trafficCount == 1500)
  {
     digitalWrite(trafficG2, LOW);
     digitalWrite(trafficY2, HIGH);
  }
  
  else if (trafficCount == 3000)
  {
    digitalWrite(trafficR1, LOW);
    digitalWrite(trafficY2, LOW);
    digitalWrite(trafficR2, HIGH);
    digitalWrite(trafficG1, HIGH);
  }
  
  else if (trafficCount == 4500)
  {
    digitalWrite(trafficG1, LOW);
    digitalWrite(trafficY1, HIGH);   
  }
  
  else if (trafficCount == 6000)
  {
    digitalWrite(trafficR2, LOW);
    digitalWrite(trafficY1, LOW);
    if (buttonPressed == true) // button has been pressed while light was green, modify next cycle
    {
      modifiedCycle = true;
    }
    trafficCount = -1;
  }
}

void trafficLightsMod ()
{
  // modification of trafficLights method for the crossing cycle
  if (trafficCount == 0)
  {
    digitalWrite(trafficR1, HIGH);
    digitalWrite(trafficG2, HIGH);
    digitalWrite(pedestrianR, HIGH);
    digitalWrite(pedestrianG, LOW);
  }
  
  else if (trafficCount == 750)
  {
     digitalWrite(trafficG2, LOW);
     digitalWrite(trafficY2, HIGH);
  }
  
  else if (trafficCount == 1500)
  {
    digitalWrite(trafficR1, LOW);
    digitalWrite(trafficY2, LOW);
    digitalWrite(trafficG2, LOW);
    digitalWrite(trafficR2, HIGH);
    digitalWrite(trafficG1, HIGH);
    digitalWrite(pedestrianG, HIGH);
    digitalWrite(pedestrianR, LOW);
  }
  
  else if (trafficCount == 4500)
  {
    digitalWrite(trafficR1, LOW);
    digitalWrite(trafficG1, LOW);
    digitalWrite(trafficY1, HIGH); 
    digitalWrite(trafficY2, LOW);
    digitalWrite(pedestrianR, HIGH);
    digitalWrite(pedestrianG, LOW);
  }
  
  else if (trafficCount == 5250)
  {
    digitalWrite(trafficR2, LOW);
    digitalWrite(trafficY1, LOW);
    modifiedCycle = false;
    buttonPressed = false;
    trafficCount = -1;
  }
}

void gateSequence ()
{ 
  if (gateCount == 0 && overrideDetected == true) // if gate was opened through override, skip 2 sec delay
     gateCount == 2000; 
  if (gateCount == 2000) // 2 s after obj. has been detected, start opening gate
  {
    digitalWrite(buzzer, HIGH); // buzzer sounds while gate is opening (appx. 0.2 )
    myservo.write(45); // start opening gate
  }
  else if (gateCount == 2200) // gate is now open
  { 
    digitalWrite(buzzer, LOW); // stop buzzer
  }
  else if (gateCount == 3700)// gate has been open for 1.5 s (excluding openinging time of appx. 0.2 )
  {
    digitalWrite(buzzer, HIGH); // buzzer sounds while gate is closing (appx. 0.5 s) 
    myservo.write(0); // start closing gate
  }
  else if (gateCount == 3900) // end of gate sequence
  {
    digitalWrite(buzzer, LOW); // stop buzzer
    gateCount = -1; // reset count
  	inGateSequence = false; // reset booleans since sequence has ended
    overrideDetected = false;
  }
  gateCount++;
}

void updateStreetlight ()
{
	if (analogRead(photoresistor) > 107) // if-else block updates streetlight status based on photoresistor input
    	digitalWrite(streetlight, HIGH);
    else 
    	digitalWrite(streetlight, LOW);
}

void updateOverrideDetected()
{
  if (irrecv.decode(&results)) // input is deteted
  {
    if (results.value == 16580863){ // power button is pressed
     overrideDetected = true; // an override has occured
    }
    irrecv.resume(); // resume detecting more input
  }
}

void updateSensorDist ()
{
  long duration;
  // sensor outputs a pulse:
  pinMode(ultrasonic, OUTPUT); 
  digitalWrite(ultrasonic, LOW);
  delayMicroseconds(2);
  digitalWrite(ultrasonic, HIGH);
  delayMicroseconds(5);
  digitalWrite(ultrasonic, LOW);
  
  // detect time taken by return pulse using pulseIn()
  pinMode(ultrasonic, INPUT);
  duration = pulseIn(ultrasonic, HIGH);

  // convert the time into a distance and update sensorDist
  sensorDist = duration / 29 / 2; // sound travels at a speed of 29 microseconds per centimeter, thus duration / 29 gives us total distance traveled by the pulse, half of that is distance of the obj.
}