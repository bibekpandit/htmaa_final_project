#include<math.h>
#include<string.h>
#include <WiFi.h> //Connect to WiFi Network
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Adafruit_BNO055 bno = Adafruit_BNO055(55);

#define IDLE 0
#define SEND 1

#define START 0
#define DOWN 1
#define WAIT_FOR_UP 2
#define UP 3
#define COMPLETE_PRESS 4
#define EXECUTE 5

char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = ""; //Password for 6.08 Lab

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
float x,y,z; //variables for grabbing x,y,and z values

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1800; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

const int NUM_OF_VALUES = 50;
const int VALUES_ARRAY_SIZE = 7*NUM_OF_VALUES + 100;
char x_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP request
char y_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP response
char z_values[VALUES_ARRAY_SIZE]; //char array buffer to hold HTTP request

const int HTTP_BODY_SIZE = 3*VALUES_ARRAY_SIZE + 100;
char body[HTTP_BODY_SIZE]; //for body
const int input_pin = 5;

int post_state;
int no_of_instances;
char human_state[10];

float current[3];
float previous[3];
float before_previous[3];
float average[3];

int num_count = 0;
int DEBOUNCE_DELAY = 60;
int NEXT_BUTTON_PRESS_DELAY = 1000;
int next_button_press_timer = 0;
int debounce_timer = 0;


int button_state = START;
bool transmit_data = false;
char screen_text[100];


void setup(void) 
{
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  Wire.begin();
  delay(50); //pause to make sure comms get set up

  WiFi.begin(network); //attempt to connect to wifi
  uint8_t count = 0; //count used for Wifi check times
  Serial.print("Attempting to connect to ");
  Serial.println(network);
  while (WiFi.status() != WL_CONNECTED && count<6) {
    delay(500);
    Serial.print(".");
    count++;
  }
  delay(2000);
  if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
    Serial.println("CONNECTED!");
    Serial.println(WiFi.localIP().toString() + " (" + WiFi.macAddress() + ") (" + WiFi.SSID() + ")");
    delay(500);
  } else { //if we failed to connect just Try again.
    Serial.println("Failed to Connect :/  Going to restart");
    Serial.println(WiFi.status());
    ESP.restart(); // restart the ESP (proper way)
  }
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

  for (int i=0; i<3; i++){
    current[i] = 0;
    previous[i] = 0;
    before_previous[i] = 0;
    average[i] = 0;
  }

  post_state = IDLE;
  no_of_instances = 0;
}
 
void loop() 
{  
  imu::Vector<3> accelerometer = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

  current[0] = accelerometer.x();
  current[1] = accelerometer.y();
  current[2] = accelerometer.z();

  for (int i=0; i<3; i++){
    average[i] = (current[i] + previous[i] + before_previous[i])/3;
  }
  
  /* Display the floating point data */
//  Serial.print("X: ");
//  Serial.print(average[0]);
//  Serial.print(" Y: ");
//  Serial.print(average[1]);
//  Serial.print(" Z: ");
//  Serial.print(average[2]);
//  Serial.println("");

  int no_button_presses = button_presses(digitalRead(input_pin));

  switch(no_button_presses){
    case 0:
      // do nothing
      break;
    case 1:
      // clear cache
      x_values[0] = '\0';
      y_values[0] = '\0';
      z_values[0] = '\0';
//      memset(x_values, 0, sizeof(x_values));
//      memset(y_values, 0, sizeof(y_values));
//      memset(z_values, 0, sizeof(z_values));
      
      strncpy(human_state, "idle", sizeof(human_state));
      transmit_data = false;
      strncpy(screen_text, "State: idle  ", sizeof(screen_text));
      tft.println(screen_text);
      break;
    case 2:
      strncpy(human_state, "walk", sizeof(human_state));
      transmit_data = true;
      strncpy(screen_text, "State: walk ", sizeof(screen_text));
      tft.println(screen_text);
      break;
    case 3:
      strncpy(human_state, "lie", sizeof(human_state));
      transmit_data = true;
      strncpy(screen_text, "State: lie    ", sizeof(screen_text));
      tft.println(screen_text);
      break;
    case 4:
      strncpy(human_state, "fall", sizeof(human_state));
      transmit_data = true;
      strncpy(screen_text, "State: fall    ", sizeof(screen_text));
      tft.println(screen_text);
      break;
    case 5:
      strncpy(human_state, "sit", sizeof(human_state));
      transmit_data = true;
      strncpy(screen_text, "state: sit    ", sizeof(screen_text));
      tft.println(screen_text);
      break;
  }

  if(transmit_data){
    post_reporter_fsm(average[0], average[1], average[2]);
  }
  
  
  for (int i=0; i<3; i++){
    before_previous[i] = previous[i];
    previous[i] = current[i];
  }
  
  while (millis()-primary_timer<LOOP_PERIOD); //wait for primary timer to increment
  primary_timer =millis();
}

void post_reporter_fsm(float x, float y, float z){
  switch (post_state){
    case IDLE:
    
      if (no_of_instances < NUM_OF_VALUES - 1){
        sprintf(x_values+strlen(x_values), "%.2f,", x);
        sprintf(y_values+strlen(y_values), "%.2f,", y);
        sprintf(z_values+strlen(z_values), "%.2f,", z);
        no_of_instances++;
      }
      else{
        sprintf(x_values+strlen(x_values), "%.2f", x);
        sprintf(y_values+strlen(y_values), "%.2f", y);
        sprintf(z_values+strlen(z_values), "%.2f", z);
        no_of_instances++;
        post_state = SEND;
      }
      
      break;

    case SEND:
      
      // perform post
      sprintf(body,"state=%s&x=%s&y=%s&z=%s",human_state, x_values, y_values, z_values);//generate body, posting to User, 1 step
      
      int body_len = strlen(body); //calculate body length (for header reporting)
      Serial.println(body_len);
      sprintf(request_buffer,"POST https://guarded-river-48081.herokuapp.com/post HTTP/1.1\r\n");
      strcat(request_buffer,"Host: guarded-river-48081.herokuapp.com\r\n");
      strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
      sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
      strcat(request_buffer,"\r\n"); //new line from header to body
      strcat(request_buffer,body); //body
      strcat(request_buffer,"\r\n"); //header
      Serial.println(request_buffer);
      do_http_request("guarded-river-48081.herokuapp.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
      Serial.println(response_buffer); //viewable in Serial Terminal
      tft.println(response_buffer); //viewable on Screen
      
      memset(request_buffer, 0, sizeof(request_buffer));
      memset(response_buffer, 0, sizeof(response_buffer));
      memset(x_values, 0, sizeof(x_values));
      memset(y_values, 0, sizeof(y_values));
      memset(z_values, 0, sizeof(z_values));
      no_of_instances = 0;
      post_state = IDLE;
      break;
  }
      
}

int button_presses(uint8_t input){
  switch(button_state){
    case START:
      num_count = 0; // Reset counter
      if (input == 0){
        debounce_timer = millis();
        button_state = DOWN;
      }
      break;
    case DOWN:
      //pause_timer
      if (millis() - debounce_timer < DEBOUNCE_DELAY) break;
      if (input == 1){
        // coming to DOWN might be a debounce error
        if (num_count == 0){
          button_state = START;
        }
        else{
          button_state = COMPLETE_PRESS;
        }
      }
      else if(input == 0){
        button_state = WAIT_FOR_UP;
      }
      break;
    case WAIT_FOR_UP:
      if (input == 1){
        debounce_timer = millis();
        button_state = UP;
      }
      break;
    case UP:
      if (millis() - debounce_timer < DEBOUNCE_DELAY) break; 
      if (input == 0){
        // debouncing error prompted change of state
          button_state = WAIT_FOR_UP;
      }
      else if(input == 1){
        num_count++;
        next_button_press_timer = millis();
        button_state = COMPLETE_PRESS;
      }
      break;
      
    case COMPLETE_PRESS:
      if (millis() - next_button_press_timer >= NEXT_BUTTON_PRESS_DELAY){
        button_state = EXECUTE;
      }
      else{
        if (input == 0){
          debounce_timer = millis();
          button_state = DOWN;
        }
      }
      break;

    case EXECUTE:
      tft.fillScreen(TFT_BLACK); 
      tft.setCursor(0,0);
      sprintf(screen_text, "Count: %d  ", num_count);
      tft.println(screen_text);
      button_state = START;
      return num_count;
  }
  return 0;
  
}

void get_request(){
  strcat(request_buffer, "GET https://guarded-river-48081.herokuapp.com/ HTTP/1.1\r\n");
  strcat(request_buffer,"Host: guarded-river-48081.herokuapp.com\r\n"); //add more to the end
  strcat(request_buffer,"\r\n"); //add blank line!
  //submit to function that performs GET.  It will return output using response_buffer char array
  do_http_request("guarded-river-48081.herokuapp.com", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
  Serial.println(response_buffer); //print to serial monitor
  tft.fillScreen(TFT_BLACK); //black out TFT Screen
  tft.drawString(response_buffer, 0, 0, 1); //viewable on Screen
}
