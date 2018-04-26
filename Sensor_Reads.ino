#include <MatrixMath.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303.h>
#define MatDim 4//matrices for frame transformations are 4x4;

volatile int accelRDY=0;
Adafruit_LSM303 lsm;
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to

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
    //Serial.println("no LSM303 detected");
    while(1){
      digitalWrite(13,1);
      delay(100);
      digitalWrite(13,0);
      delay(100);
    }
  }
  attachInterrupt(0,LSM_RDY,FALLING);
}

int sensorValue = 0;       
double vel_est=0;
double accel_est=0;
long int init_accel[3]={0,0,0};
int first_acc_done=0;
int first_acc_count=0;
double init_orient[3]={0,0,0};//RPY
double comp_mag_orient[3]={0,0,0};
int magaxdown=0;//0 is x, 1 is y, 2 is z
double tmp_orient[3]={0,0,0};

//INITIALIZE MATRICES
double Start_Matrix[MatDim][MatDim];//initial orientation to compensate mag angles as rpy
double Accelerometer_Matrix[MatDim][MatDim];//accelerometer as pxpypz
double Magnetometer_Matrix[MatDim][MatDim];//Hold the magnetometer oreintation as rpy
double Comp_Mag[MatDim][MatDim];//hold compensated mag angles
double Comp_Accel[MatDim][MatDim];//hold compensated accelerometer data
double TMP_Matrix[MatDim][MatDim];//temporary matrix for swaps
//END INIT MATRICES

void loop(void)
{
  /* Get a new sensor event */
  if(accelRDY==1){
    lsm.read();
    /* Display the results (acceleration is measured in m/s^2) */
    int accelxyz[3]={(int)lsm.accelData.x,(int)lsm.accelData.y,(int)lsm.accelData.z};
    int magxyz[3]={(int)lsm.magData.x,(int)lsm.magData.y,(int)lsm.magData.z};
    /*get initial orientation using the accelerometer when not moving */
    if(first_acc_done==0){
     if (first_acc_count<10){
       first_acc_count++;
       //Serial.println(first_acc_count);
       for(int k=0;k<3;k++){
         init_accel[k]+=accelxyz[k];
       }         
     }
     else{
       int maxAx=0;
       for(int k=0;k<3;k++){
         init_accel[k]/=10;
         //figure out which axis is alligned with gravity
         //positive z is down.
         if(maxAx<abs(init_accel[k])){
           maxAx=abs(init_accel[k]);
           magaxdown=k;//set 'downwards axis'
         }
       }
     if (magaxdown==0){//x-axis down, this shouldn't ever happen
       while(1){ //busy loop and flash slow
        digitalWrite(13,1);
        delay(1000);
        digitalWrite(13,0);
        delay(1000);
       }
     }
     else if(magaxdown==1){//y axis down, this also shouldn't ever happen
       while(1){//        digitalWrite(13,1);
        delay(1000);
        digitalWrite(13,0);
        delay(1000);//busy loop and flash half slow
        digitalWrite(13,1);
        delay(500);
        digitalWrite(13,0);
        delay(500);
       }
     }
     
     else if(magaxdown==2){//z axis down, this is normal.
     //based on adafruit's 9-DOF library and ST reeference document
     //http://www.st.com/content/ccc/resource/technical/document/design_tip/group0/56/9a/e4/04/4b/6c/44/ef/DM00269987/files/DM00269987.pdf/jcr:content/translations/en.DM00269987.pdf
          
          MeasToRPY((double*)init_orient,(double)init_accel[0],(double)init_accel[1],(double)init_accel[2],0.04);
          /*
          init_orient[0]=atan2((double)init_accel[1],(double)init_accel[2]);//roll
          double Gz2=(double)init_accel[1]*sin(init_orient[0])+(double)init_accel[2]*cos(init_orient[0]);
          init_orient[1]=atan2(-((double)init_accel[0]),Gz2);
          double Gy2=(double)init_accel[2]*sin(init_orient[0])-(double)init_accel[1]*cos(init_orient[0]);
          double Gx2=(double)init_accel[0]*cos(init_orient[1])-(double)init_accel[2]*sin(init_orient[1]);
          init_orient[2]=atan2(Gy2,Gx2);
          */
     }
     for(int k=0;k<3;k++){
     Serial.println(init_orient[k]);   
     }
     first_acc_done=1;  
     }
    }
    
    //get data, estimate velocity, compensate for angle, send to pi
    else{
      /*end initial orientation get*/
      
      //accel_est=(((double)((int)lsm.accelData.x/(819))/10));
      //vel_est+=((accel_est)*9.81/15);
      // read the analog joint angles:
      /*
      for(int n=0;n<=5;n++){
        sensorValue = analogRead(analogInPin+n);            
        Serial.print(sensorValue);   
        Serial.print(",");
      }
      */
      digitalWrite(13,0);
      accelRDY=0;
      //Serial.println();
      
    }
  }
}

void LSM_RDY(void){
  accelRDY=1;
  digitalWrite(13,1);
 }
 //take measured values and convert to rpy angles
 void MeasToRPY(double* RPY_angles, double x_meas, double y_meas, double z_meas,double alpha){
 //takes a pointer to a 3 wide array
 RPY_angles[0]=atan2((double)y_meas,((double)z_meas+(double)x_meas*alpha));//roll
 double Tmpz2=(double)y_meas*sin(RPY_angles[0])+((double)z_meas+(double)x_meas*alpha)*cos(RPY_angles[0]);
 RPY_angles[1]=atan(-((double)x_meas)/Tmpz2);
 double Tmpy2=((double)z_meas+(double)x_meas*alpha)*sin(RPY_angles[0])-(double)y_meas*cos(RPY_angles[0]);
 double Tmpx2=(double)x_meas*cos(RPY_angles[1])-((double)z_meas+(double)x_meas*alpha)*sin(RPY_angles[1]);
 RPY_angles[2]=atan2(Tmpy2,Tmpx2);
 }
/*
          init_orient[0]=atan2((double)init_accel[1],(double)init_accel[2]);//roll
          double Gz2=(double)init_accel[1]*sin(init_orient[0])+(double)init_accel[2]*cos(init_orient[0]);
          init_orient[1]=atan2(-((double)init_accel[0]),Gz2);
          double Gy2=(double)init_accel[2]*sin(init_orient[0])-(double)init_accel[1]*cos(init_orient[0]);
          double Gx2=(double)init_accel[0]*cos(init_orient[1])-(double)init_accel[2]*sin(init_orient[1]);
          init_orient[3]=atan2(Gy2,Gx2);
*/
