#include <Arduino.h>
#include <Wire.h>

const uint8_t LOOP_PERIOD = 10;
uint32_t primary_timer = 0;

int redpin = 25; //select the pin for the red LED
int bluepin =12; // select the pin for the  blue LED
int greenpin = 27;// select the pin for the green LED

const int freq = 5000;
const int channel_red = 0;
const int channel_blue = 1;
const int channel_green = 2;
const int resolution = 8;

int val;

void setup() {
  Serial.begin(115200);
  delay(2500);
  Wire.begin();

    // configure LED PWM functionalitites
  ledcSetup(channel_red, freq, resolution);
  ledcSetup(channel_blue, freq, resolution);
  ledcSetup(channel_green, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(redpin, channel_red);
  ledcAttachPin(bluepin, channel_blue);
  ledcAttachPin(greenpin, channel_green);

  
//  pinMode(redpin, OUTPUT);
//  pinMode(bluepin, OUTPUT);
//  pinMode(greenpin, OUTPUT);
}

void loop() 
{
  
//  for(val = 255; val > 0; val--)
//  {
//    ledcWrite(channel_red, val);  //set PWM value for red
//    ledcWrite(channel_blue, 255 - val); //set PWM value for blue
//    ledcWrite(channel_green, 128 - val); //set PWM value for green
//    Serial.println(val); //print current value 
//    delay(15); 
//  }
//  for(val = 0; val < 255; val++)
//  {
//    ledcWrite(channel_red, val);
//    ledcWrite(channel_blue, 255 - val);
//    ledcWrite(channel_green, 128 - val);
//    Serial.println(val);
//    delay(15); 
//  }

  ledcWrite(channel_red, 128);
  ledcWrite(channel_blue, 28);
  ledcWrite(channel_green, 100);
  Serial.println(val);
  delay(15); 

  Serial.println("Working");

  while(millis()-primary_timer < LOOP_PERIOD);
  primary_timer = millis();
}
