#include <mpu9255_esp32.h>
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


char network[] = "MIT";  //SSID for 6.08 Lab
char password[] = "iesc6s08"; //Password for 6.08 Lab

const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
float x,y,z; //variables for grabbing x,y,and z values

//Some constants and some resources:
const int RESPONSE_TIMEOUT = 6000; //ms to wait for response from host
const int POSTING_PERIOD = 6000; //periodicity of getting a number fact.
const uint16_t IN_BUFFER_SIZE = 1000; //size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; //size of buffer to hold HTTP response
char request_buffer[IN_BUFFER_SIZE]; //char array buffer to hold HTTP request
char response_buffer[OUT_BUFFER_SIZE]; //char array buffer to hold HTTP response

float current[3];
float previous[3];
float before_previous[3];
float average[3];

void setup(void) 
{
  Serial.begin(115200); //for debugging if needed.
  delay(50); //pause to make sure comms get set up
  Wire.begin();
  delay(50); //pause to make sure comms get set up
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
}
 
void loop(void) 
{  
  imu::Vector<3> accelerometer = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);

  current[0] = accelerometer.x();
  current[1] = accelerometer.y();
  current[2] = accelerometer.z();

  for (int i=0; i<3; i++){
    average[i] = (current[i] + previous[i] + before_previous[i])/3;
  }
  
  /* Display the floating point data */
  Serial.print("X: ");
  Serial.print(average[0]);
  Serial.print(" Y: ");
  Serial.print(average[1]);
  Serial.print(" Z: ");
  Serial.print(average[2]);
  Serial.println("");

  for (int i=0; i<3; i++){
    before_previous[i] = previous[i];
    previous[i] = current[i];
  }
  
  delay(100);
}

void post_reporter_fsm(uint8_t button1){

    // perform post
    char body[100]; //for body
    sprintf(body,"state=%s&x=%f&y=%f&z=%f",state, x, y, z);//generate body, posting to User, 1 step
    int body_len = strlen(body); //calculate body length (for header reporting)
    sprintf(request_buffer,"POST https://guarded-river-48081.herokuapp.com/post HTTP/1.1\r\n");
    strcat(request_buffer,"Host: guarded-river-48081.herokuapp.com\r\n");
    strcat(request_buffer,"Content-Type: application/x-www-form-urlencoded\r\n");
    sprintf(request_buffer+strlen(request_buffer),"Content-Length: %d\r\n", body_len); //append string formatted to end of request buffer
    strcat(request_buffer,"\r\n"); //new line from header to body
    strcat(request_buffer,body); //body
    strcat(request_buffer,"\r\n"); //header
    Serial.println(request_buffer);
    do_http_request("608dev.net", request_buffer, response_buffer, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT,true);
    Serial.println(response_buffer); //viewable in Serial Terminal
      
}
