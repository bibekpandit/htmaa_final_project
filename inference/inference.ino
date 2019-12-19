#include<math.h>
#include<string.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Adafruit_BNO055 bno = Adafruit_BNO055(55);

// states for doing post request
//#define IDLE 0
//#define SEND 1


// states for the button press counter
//#define START 0
//#define DOWN 1
//#define WAIT_FOR_UP 2
//#define UP 3
//#define COMPLETE_PRESS 4
//#define EXECUTE 5


// motion sates
#define STILL 0
#define WALK 1
#define FALL 2

float WALK_THRESHOLD = 0.07;
float FALL_THRESHOLD = 30.0;
int change_motion_state = 0;

char network[] = "EECS_Labs";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
//float x,y,z; //variables for grabbing x,y,and z values

//const int NUM_OF_VALUES = 50;
//const int VALUES_ARRAY_SIZE = 7*NUM_OF_VALUES + 100;
//char x_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP request
//char y_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP response
//char z_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP request

//const int HTTP_BODY_SIZE = 3*VALUES_ARRAY_SIZE + 100;
//char body[HTTP_BODY_SIZE]; //for body

//Some constants and some resources:
//const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
//const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
//const uint16_t IN_BUFFER_SIZE = 1800; //size of buffer to hold HTTP request
//const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
//char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
//char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const int input_pin = 5;

//int post_state;
//int no_of_instances;
//char human_state[10];
//int post_requests_sent;
int motion_state;

const int COUNTS_TO_AVG = 20;
const int NO_OF_AXES = 3;

//float current[3];
//float previous[3];
//float before_previous[3];
//float average[3];

float accel_data[NO_OF_AXES][COUNTS_TO_AVG];
int data_array_index;


//int DEBOUNCE_DELAY = 60;
//int NEXT_BUTTON_PRESS_DELAY = 1000;
//int num_count; // number of button presses
//int next_button_press_timer;
//int debounce_timer;


//int button_state;
//bool transmit_data;
char screen_text[100];

int redpin = 25; //select the pin for the red LED
int bluepin =12; // select the pin for the  blue LED
int greenpin = 27;// select the pin for the green LED

const int freq = 5000;
const int channel_red = 0;
const int channel_blue = 1;
const int channel_green = 2;
const int resolution = 8;

void setup(void) 
{
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  Wire.begin();
  delay(50); //pause to make sure comms get set up

  pinMode(input_pin, INPUT_PULLUP);
  tft.init(); //initialize the screen
  tft.setRotation(2); //set rotation for our layout
  tft.fillScreen(TFT_BLACK); 
  tft.setTextColor(TFT_BLUE, TFT_BLACK);
  
  Serial.println("Orientation Sensor Test"); Serial.println("");
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);
  Serial.println("Setup array");
  for (int i=0; i<NO_OF_AXES; i++){
    for (int j=0; j<COUNTS_TO_AVG; j++){
      accel_data[i][j] = 0;
    }
  }

  Serial.println("Initialize Variables");
//  post_state = IDLE;
//  no_of_instances = 0;
//  post_requests_sent = 0;
//
//  num_count = 0;
//  next_button_press_timer = 0;
//  debounce_timer = 0;

  data_array_index = 1; // set to 1 so that when reaches 0, one set of data is available

//  button_state = START;
//  transmit_data = false;

      // configure LED PWM functionalitites
  ledcSetup(channel_red, freq, resolution);
  ledcSetup(channel_blue, freq, resolution);
  ledcSetup(channel_green, freq, resolution);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(redpin, channel_red);
  ledcAttachPin(bluepin, channel_blue);
  ledcAttachPin(greenpin, channel_green);
  
  motion_state = STILL;

  screen_text[0] = '\0';
//  tft.fillScreen(TFT_BLACK); 
  tft.setCursor(0,0);
  strcpy(screen_text, "State: STILL  ");
  tft.println(screen_text);
  Serial.println("Ready for looping");
}
 
void loop() 
{  
  imu::Vector<3> accelerometer = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

  accel_data[0][data_array_index] = accelerometer.x();
  accel_data[1][data_array_index] = accelerometer.y();
  accel_data[2][data_array_index] = accelerometer.z();

//  Serial.println(accel_data[0][data_array_index]);
  

//  Serial.print("X: ");
//  Serial.print(accel_data[data_array_index][0]);
//  Serial.print(" Y: ");
//  Serial.print(accel_data[data_array_index][1]);
//  Serial.print(" Z: ");
//  Serial.print(accel_data[data_array_index][0]);
//  Serial.println("");

//  int no_button_presses = button_presses(digitalRead(input_pin));

  if (digitalRead(input_pin)==0){
    motion_state = STILL;
    change_motion_state = 0;
    ledcWrite(channel_red, 255);
    ledcWrite(channel_blue, 255);
    ledcWrite(channel_green, 0);
    
  }

  if(data_array_index==0){
    float x_variance =  variance(accel_data[0], COUNTS_TO_AVG);
    float y_variance =  variance(accel_data[1], COUNTS_TO_AVG);
    float z_variance =  variance(accel_data[2], COUNTS_TO_AVG);
    Serial.print("z var: ");
    Serial.print(z_variance);
    Serial.println("");
    calculate_motion_state(x_variance, y_variance, z_variance);
    Serial.println(motion_state);
    Serial.print("screen text: ");
    Serial.println(screen_text);
  }

  data_array_index = (data_array_index + 1) % COUNTS_TO_AVG;
  
  
  while (millis()-primary_timer<LOOP_PERIOD); //wait for primary timer to increment
  primary_timer = millis();
}

void calculate_motion_state(float x_variance, float y_variance, float z_variance){
//  if (motion_state == FALL){
//    return;
//  }
  
  if (x_variance > 50 &&
      y_variance > 25 &&
      z_variance > 30){
    motion_state = FALL;
//    strncpy(screen_text, "State: fall    ", sizeof(screen_text));
//    tft.setCursor(0,0);
//    tft.println("fall");
     ledcWrite(channel_red, 0);
     ledcWrite(channel_blue, 255);
     ledcWrite(channel_green, 255);  
    return;
  }

  switch(motion_state){
    case STILL:
      if (z_variance > WALK_THRESHOLD){
        change_motion_state += 1;
      }
      else{
        change_motion_state = 0;
      }

      if (change_motion_state >= 4){
        motion_state = WALK;
        change_motion_state = 0;
//        strncpy(screen_text, "State: WALK    ", sizeof(screen_text));
        
//        tft.setCursor(0,0);
//        tft.println("walk");   
       ledcWrite(channel_red, 255);
       ledcWrite(channel_blue, 0);
       ledcWrite(channel_green, 255);     
      }
      break;

    case WALK:
      if (z_variance <= WALK_THRESHOLD){
        change_motion_state += 1;
      }
      else{
        change_motion_state = 0;
      }

      if (change_motion_state >= 2){
        motion_state = STILL;
        change_motion_state = 0;
        strncpy(screen_text, "State: STILL    ", sizeof(screen_text));
//        tft.setCursor(0,0);
        tft.println("still");  
        ledcWrite(channel_red, 255);
        ledcWrite(channel_blue, 255);
        ledcWrite(channel_green, 0); 
      }
      break;
  }
  
}

float variance(float data_instances[], int array_length){
  float sum = 0;
  float squared_sum = 0;
  for (int i=0; i<array_length; i++){
    sum += data_instances[i];
  }
  float mean = (float)sum/array_length;

  for (int i=0; i<array_length; i++){
    squared_sum += (data_instances[i] - mean)*(data_instances[i] - mean);
  }

  return (float)squared_sum/(array_length-1);
}
