#include <EEPROMex.h>
#include <EEPROMVar.h>

#include "ModbusRtu.h"
#define ID 1
Modbus slave(ID,mySerial);

#define IN1 A0
#define IN2 A1
#define IN3 A2
#define IN4 A3
#define IN5 A4
#define IN6 A5
#define IN7 8
#define IN8 9

#define OUT1 2
#define OUT2 3
#define OUT3 4
#define OUT4 5
#define OUT5 6
#define OUT6 7
#define OUT7 10
#define OUT8 11

#define RUN digitalRead(IN2)
#define STOP digitalRead(IN1)
#define Lazer_sensor digitalRead(IN5)

#define ON_MOTOR digitalWrite(OUT1,HIGH);
#define OFF_MOTOR digitalWrite(OUT1,LOW);
#define On_Push_Error digitalWrite(OUT5,HIGH);
#define Off_Push_Error digitalWrite(OUT5,LOW);
#define On_Red digitalWrite(OUT6,HIGH);
#define Off_Red digitalWrite(OUT6,LOW);
#define On_Yellow digitalWrite(OUT7,HIGH);
#define Off_Yellow digitalWrite(OUT7,LOW);
#define On_Green digitalWrite(OUT8,HIGH);
#define Off_Green digitalWrite(OUT8,LOW);

#define Machine_Waitting 0
#define Machine_Manual 1
#define Machine_Working 2 
#define Machine_Stopping 3
void io_setup(void);
void hmi_poll(void);
void get_data(void);
void save_data(void);
void get_data_eeprom(void);

//modbus rtu----------------------------------------------------
  int8_t state = 0;       ////
  unsigned long tempus;       ////
  uint16_t au16data[50];      ////
  unsigned long count=millis();
  int temp1;
  uint16_t value_setting = 0;
  uint16_t value_display = 0;
  uint16_t value_offset = 0;
  uint16_t Count_OK = 0;
  uint16_t Count_NG = 0;
  uint16_t value = 0;
  uint16_t error = 0;
  uint16_t dem =0 ;
  uint16_t dem1 =0 ;
  uint16_t dem2 =0 ;
  uint16_t dem3 =0 ;
  uint16_t Machine_Status;
  boolean OK = false;
  boolean NG = false;
  boolean Push = false;
  boolean flag = false;
  boolean flag1 = false;
  boolean save = false;
  boolean Error = false;
  boolean Error1 = false;
  
void setup() {
  io_setup();
  Serial.begin(9600);
  slave.begin(19200);      ////
  tempus = millis() + 100;
}
void loop() {
  state = slave.poll( au16data,50);  
  hmi_poll();
  switch( Machine_Status){
    case Machine_Waitting :
                           if(bitRead(au16data[0],0) == 1){
                               bitWrite(au16data[0],0,0);
                               bitWrite(au16data[0],1,0);
                               save = true;
                            } 
                           if(save){
                              EEPROM.writeInt(0,au16data[4]);
                              EEPROM.writeInt(5,au16data[5]);
                              save = false;
                            }
                          if(bitRead(au16data[0],1) == 1) Machine_Status = Machine_Manual;
                          else {
                            if(!RUN) Machine_Status = Machine_Working;
                            else if(STOP) Machine_Status = Machine_Stopping;
                          }
                          break;
    case Machine_Manual :
                          if(bitRead(au16data[0],2) == 1){On_Push_Error;}//Serial.println(bitRead(au16data[0],2));}
                          else {Off_Push_Error;}
                          if(bitRead(au16data[0],3) == 1){On_Yellow;}//Serial.println(bitRead(au16data[0],3));}
                          else {Off_Yellow;}
                          Machine_Status = Machine_Waitting;
                          break;
    case Machine_Working :
                            get_data_eeprom();
                            ON_MOTOR;
                            On_Green;
                            Off_Red;
                            get_data();
                            if(STOP) Machine_Status = Machine_Stopping;
                            break;
    case Machine_Stopping : 
                           OFF_MOTOR;
                           Off_Green;
                           On_Red;
                           Machine_Status = Machine_Waitting;
                           break;
  }
}

void get_data_eeprom(){
 // value get from eeprom 
  au16data[4] = EEPROM.readInt(0);
  au16data[5] = EEPROM.readInt(5);
}
void get_data(){
  value_display = au16data[1]*65535 + au16data[2];
  if(value_display < 65000)au16data[3] = value_display;
  else au16data[3] = 0; 
  au16data[8] = Count_OK + Count_NG;
  au16data[6] = Count_OK;
  au16data[7] = Count_NG;
    if(Push && Lazer_sensor==1) {flag = true;}
    if(flag && Lazer_sensor==0) {
      On_Push_Error;
      if(bitRead(au16data[0],4) == 1){Off_Yellow;}
      else {On_Yellow;}
      flag1 = true;
      Push = false;
      }
    if(flag1) dem2++;
    if(dem2 >=70) {Off_Push_Error;Off_Yellow; flag1 = false;dem2=0;flag = false;}
    Serial.println(dem2);
  if(value_display > 40000 || value_display < (au16data[5]+15)/4)
  {
//    if( value_display <= 20){
//       value_offset = value_display;
//       au16data[3] = 0;  
//    }
    if(Error){
        Count_NG++;
        dem3 =0;
        dem1 =0;
        OK = false;
        Push = true;
        Error = false;
        Error1 = false;
        }
    if(OK){
            Count_OK++;
            dem3 =0;
            dem1 =0;
            OK = false;
            Error = false;
            Error1 = false;
            }
   else if(Error1){
            Count_NG++;
            dem3 =0;
            dem1 =0;
            OK = false;
            Push = true;
            Error = false;
            Error1 = false;
            } 
  }
  else
  { 
    //Serial.print("value_offset :");
    //Serial.println(value_offset);
    if(value_display > au16data[4]){
      Error = true;
      OK = false;
      dem1 = 0;
      dem3 = 0;
      //Serial.println(value_display);
      //Serial.println("NG");
      }  
    if(value_display >= au16data[5] && value_display <= au16data[4])
      {
        Error1 = false;
        dem1++;
        //Serial.print("dem1 :");
        //Serial.println(dem1);
        //Serial.println(value_display);
        if(dem1 >= 5 && dem1 < 200){
        OK = true;
        dem3 = 0;
        //Serial.println("OK");
        }
      }
    if(value_display > (au16data[5] - 20) && value_display < au16data[5]){
      dem3++;
      //Serial.print("dem3 :");
      //Serial.println(dem3);
      //Serial.println(value_display);
      if(dem3 > 30){
        Error1 = true;
        OK = false;
        dem1 = 0;
        //Serial.println("NG1");
      }
    }
  }
}
void hmi_poll()
{
  au16data[47] = slave.getInCnt();
  au16data[48] = slave.getOutCnt();
  au16data[49] = slave.getErrCnt();
}
void io_setup(){
  pinMode(IN1,INPUT_PULLUP);pinMode(IN2,INPUT_PULLUP);pinMode(IN3,INPUT_PULLUP);pinMode(IN4,INPUT_PULLUP);pinMode(IN5,INPUT_PULLUP);pinMode(IN6,INPUT_PULLUP);pinMode(IN7,INPUT_PULLUP);pinMode(IN8,INPUT_PULLUP);
  pinMode(OUT1,OUTPUT);pinMode(OUT2,OUTPUT);pinMode(OUT3,OUTPUT);pinMode(OUT4,OUTPUT);pinMode(OUT5,OUTPUT);pinMode(OUT6,OUTPUT);pinMode(OUT7,OUTPUT);pinMode(OUT8,OUTPUT);
}
