#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>

volatile int accelRDY=0;
Adafruit_LSM303 lsm;
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
int sensorValue = 0;        // value read from the pot
int n=0;
double vel_est=0;
double accel_est=0;

void setup(void)
{
  Serial.begin(115200);
  pinMode(2,INPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,0);
  /* Initialise the sensors */
  
  if(!lsm.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("no LSM303 detected");
    while(1){
      digitalWrite(13,1);
      delay(100);
      digitalWrite(13,0);
      delay(100);
    }
  }
  attachInterrupt(0,LSM_RDY,FALLING);
}

void loop(void)
{
  /* Get a new sensor event */
  if(accelRDY==1){
    lsm.read();
    /* Display the results (acceleration is measured in m/s^2) */
    accel_est=(((float)((int)lsm.accelData.x/(800))/10));
    vel_est+=((accel_est)*9.81/15);
    Serial.print("X: "); Serial.print(lsm.accelData.x/(8000)+0.02); Serial.print(" ");
    Serial.print("Y: "); Serial.print(lsm.accelData.y/8000-0.02);       Serial.print(" ");
    Serial.print("Z: "); Serial.print(lsm.accelData.z/8000-0.01);     Serial.print(" ");
    Serial.print("accel_est:"); Serial.print(accel_est);Serial.print(" ");
    Serial.print("v_x: "); Serial.println(vel_est);
    /*Serial.print("Mag X: "); Serial.print(lsm.magData.x);     Serial.print(" ");
    Serial.print("Y: "); Serial.print(lsm.magData.y);         Serial.print(" ");
    Serial.print("Z: "); Serial.println(lsm.magData.z);       Serial.print(" ");//*/
    // read the analog joint angles:
    for(n=0;n<=5;n++){
      sensorValue = analogRead(analogInPin+n);            
      Serial.print("s" );  
      Serial.print(n);
      Serial.print(':');
      Serial.print(sensorValue);   
      Serial.print(",");
    }
    digitalWrite(13,0);
    accelRDY=0;
  }
}

void LSM_RDY(void){
  accelRDY=1;
  digitalWrite(13,1);
 }
