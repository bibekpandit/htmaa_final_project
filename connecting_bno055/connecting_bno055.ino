#include<math.h>
#include<string.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Adafruit_BNO055 bno = Adafruit_BNO055(55);


const uint8_t LOOP_PERIOD = 10; //milliseconds
uint32_t primary_timer = 0;
float x,y,z; //variables for grabbing x,y,and z values

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
