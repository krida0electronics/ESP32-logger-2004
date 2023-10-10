
// WEMOS D1 MINI ESP32

#include <LiquidCrystal.h>
#include "OneButton.h"
#include <EEPROM.h>
#include <DS3231.h>
#include <Wire.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <OneWire.h>
#include <DallasTemperature.h>     // 1-wire pin on GPIO 10
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

File myFile;

DS3231 myRTC;

#define ONE_WIRE_BUS 10
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numberOfDevices;
DeviceAddress tempDeviceAddress;

OneButton BUTTON_ENTER  (14, true);
OneButton BUTTON_SELECT (34, true);
OneButton BUTTON_UP     (33, true);
OneButton BUTTON_DOWN   (35, true);

hw_timer_t *My_timer = NULL;

//LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
  LiquidCrystal lcd(27, 25, 32, 26, 4, 16);

unsigned char cursor_position;
unsigned char menu;
unsigned char measuring;
unsigned char format;
unsigned char time_select;
unsigned char date_select;
unsigned char clock_read_flag;
unsigned char set_new_time;
unsigned char set_new_date;
unsigned char logging_flag;
unsigned char flag;
unsigned char log_time_flag;
unsigned char backlight_level;
unsigned char backlight_mode;
unsigned int interval;
unsigned int interval_tick;
unsigned char sd_card_status;
unsigned char file_open_flag;

float temp_mas[8];
unsigned int temp_x10_mas[8];
unsigned char sign_mas[8];

unsigned char temp0_values[4];
unsigned char temp1_values[4];
unsigned char temp2_values[4];
unsigned char temp3_values[4];
unsigned char temp4_values[4];
unsigned char temp5_values[4];
unsigned char temp6_values[4];
unsigned char temp7_values[4];


// двухмерный массив для температуры, 8 строк 4 столбцов
unsigned char temp_values_mas[8][4] = { {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 },
                                        {0, 0, 0, 0 }  };


byte year;
byte month;
byte date;
byte dOW;
byte hour;
byte minute;
byte second;

byte gradus[8] = { 0b00110, 0b01001, 0b01001, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000 };
byte point[8]  = { 0b01000, 0b01100, 0b01110, 0b01111, 0b01110, 0b01100, 0b01000, 0b00000 };      

////////////////////////////////////////////////////////////////////

// Timer interrupt
   void IRAM_ATTR onTimer(){ flag = 1; } // this timer for buttons

////////////////////////////////////////////////////////////////////

void IRAM_ATTR Ext_INT1_ISR() { clock_read_flag = 1;
                                 log_time_flag = 1;
                                   interval_tick++;         } // end of SQW_interrupt function

////////////////////////////////////////////////////////////////////


void setup() { 
               EEPROM.begin(32); delay(50);

               Wire.begin(); delay(10);

               interval = EEPROM.read(0);
               format   = EEPROM.read(4);
               measuring   = EEPROM.read(8);
               backlight_level = EEPROM.read(12);
               backlight_mode = EEPROM.read(16);

               Serial.begin(115200); Serial.println();
               SerialBT.begin("DATALOGGER");

               Serial.println("> Device successfully started");
               Serial.println("> Firmware version: 1.01"); 
               Serial.println("> Firmware size: 1135545 bytes");
               Serial.println("> Manufacturing date: 31 AUG 2023");
               Serial.println("> Bluetooth activated. Name: DATALOGGER");
               Serial.print("> Chip temperature: "); Serial.print(myRTC.getTemperature(),1); Serial.println(" C degrees");   

               sensors.begin(); // ds18b20 sensors

               numberOfDevices = sensors.getDeviceCount();

               sensors.requestTemperatures(); 

               Serial.print("> Found DS18B20 sensor(s): "); Serial.println(numberOfDevices, DEC);

               for(int i=0;i<numberOfDevices; i++) {
                                                    if(sensors.getAddress(tempDeviceAddress, i)) {
                                                                                                  Serial.print("> DS18B20 sensor #");
                                                                                                  Serial.print(i+1, DEC);
                                                                                                  Serial.print(" ROM code: "); printAddress(tempDeviceAddress);
                                                                                                  Serial.println();
                                                                                                 }
                                                   } // end of FOR operator
                                                 

               /////////////////////////////////////
               
               #include "SD_card_code.h"

               /////////////////////////////////////
               
             
               pinMode(2, OUTPUT);                        // LCD backlight pwm pin
               //analogWrite(2, backlight_level*2.55);    // set backlight level

               analogWrite(2, backlight_level*2.55);    // set backlight level

               pinMode(17, INPUT);

               My_timer = timerBegin(0, 80, true);
               timerAttachInterrupt(My_timer, &onTimer, true);
               timerAlarmWrite(My_timer, 10000, true);           // time in us, origin 8000
               timerAlarmEnable(My_timer);

               bool outputSQW = true;
               bool batteryUseAllowed = false;
               byte outputFrequency = 0;         // select 1 Hz
               bool mode12 = false;              // use 24-hour clock mode

               myRTC.setClockMode(mode12);
               myRTC.enableOscillator(outputSQW, batteryUseAllowed, outputFrequency); 

               //myRTC.setHour(14); myRTC.setMinute(45); myRTC.setSecond(0); myRTC.setYear(23); myRTC.setMonth(1); myRTC.setDate(7);
               
               //if (EEPROM.read(20) == 0) { myRTC.setHour(14); myRTC.setMinute(52); myRTC.setSecond(0); myRTC.setYear(23); myRTC.setMonth(1); myRTC.setDate(15); 
               //                            EEPROM.write(20, 1); EEPROM.commit(); 
               //                            
               //                           
               //                          }
               
               BUTTON_ENTER.attachClick(click_enter);
               BUTTON_SELECT.attachClick(click_select);
               BUTTON_UP.attachClick(click_up);
               BUTTON_DOWN.attachClick(click_down);
               BUTTON_SELECT.attachLongPressStart(long_select);
  
               lcd.begin(20, 4); delay(200);
               lcd.createChar(0, gradus);
               lcd.createChar(1, point);
               lcd.clear(); delay(5);
               lcd.setCursor(2, 0); lcd.print("8CH TEMPERATURE");
               lcd.setCursor(1, 1); lcd.print("STANDALONE SD CARD");
               lcd.setCursor(5, 2); lcd.print("DATALOGGER");
               lcd.setCursor(1, 3); lcd.print("FOR DS18B20 PROBES");
               delay(3000);
               lcd.clear(); delay(5);

               if (sd_card_status == 1) { myFile = SD.open("/data.txt", FILE_APPEND);
                                          if (myFile){ Serial.println("> File data.txt exist and ready for logging"); 
                                                       Serial.print("> File data.txt size: "); Serial.print(myFile.size()); Serial.println(" bytes");
                                                       lcd.setCursor(0, 1); lcd.print("SD Card detected  OK");
                                                       lcd.setCursor(0, 2); lcd.print("File data.txt     OK");
                                                       delay(3000);
                                                       lcd.clear(); delay(5);
                                                      } 
                                                      
                                                else { Serial.println("> Error opening file data.txt or it doesn't exist"); 
                                                       lcd.setCursor(0, 1); lcd.print("SD Card detected  OK");
                                                       lcd.setCursor(0, 2); lcd.print("File data.txt  ERROR");
                                                       delay(3000);
                                                       lcd.clear(); delay(5);
                                                     }
                                          myFile.close();       
                                        }

                                        else { Serial.println("> SD card not detect or it broken"); 
                                               lcd.setCursor(0, 1); lcd.print("SD Card not detected");
                                               delay(3000);
                                               lcd.clear(); delay(5);
                                             }

               // Here check function for DS18B20 sensors                              

               
               lcd.setCursor(2, 0); lcd.print("START LOGGING");
               lcd.setCursor(2, 2); lcd.print("DEVICE SETUP");
               lcd.setCursor(2, 3); lcd.print("BACKLIGHT SETUP");
               lcd.setCursor(0, cursor_position); lcd.write(0x3E);

               attachInterrupt(17, Ext_INT1_ISR, FALLING);

              } // end of SETUP void

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////                                                                                          


void loop() { 
              if (flag == 1) { BUTTON_ENTER.tick();
                                BUTTON_SELECT.tick();
                                BUTTON_UP.tick();
                                BUTTON_DOWN.tick();
                                flag = 0;             } //end of flag void
               

              if (set_new_time == 1) { myRTC.setHour(hour); myRTC.setMinute(minute); myRTC.setSecond(second);            
                                       //EIMSK=(1<<INT1) | (0<<INT0);    // Stop external interrupt     
                                       attachInterrupt(17, Ext_INT1_ISR, FALLING);
                                       set_new_time = 0;                                                        }

              if (set_new_date == 1) { myRTC.setDate(date); myRTC.setMonth(month); myRTC.setYear(year);        
                                       //EIMSK=(1<<INT1) | (0<<INT0);    // Stop external interrupt         
                                       attachInterrupt(17, Ext_INT1_ISR, FALLING);
                                       set_new_date = 0;                                                        }

              if (clock_read_flag == 1)
               { 
                second = myRTC.getSecond();
                minute = myRTC.getMinute();

                bool h12Flag;
                bool pmFlag;
                hour = myRTC.getHour(h12Flag, pmFlag);

                date = myRTC.getDate();
    
                bool CenturyBit;
                month = myRTC.getMonth(CenturyBit);
    
                year = myRTC.getYear();

                if (menu == 2)
                 { 
                  if (hour>9) { lcd.setCursor(12, 0); lcd.print(hour, DEC); } else { lcd.setCursor(12, 0); lcd.print(" "); lcd.print(hour, DEC); }
                  if (minute>9) { lcd.setCursor(15, 0); lcd.print(minute, DEC); } else { lcd.setCursor(15, 0); lcd.print("0"); lcd.print(minute, DEC); }
                  if (second>9) { lcd.setCursor(18, 0); lcd.print(second, DEC); } else { lcd.setCursor(18, 0); lcd.print("0"); lcd.print(second, DEC); }

                  if (date>9) { lcd.setCursor(12, 1); lcd.print(date, DEC); } else { lcd.setCursor(12, 1); lcd.print(" "); lcd.print(date, DEC); }
                  if (month>9) { lcd.setCursor(15, 1); lcd.print(month, DEC); } else { lcd.setCursor(15, 1); lcd.print("0"); lcd.print(month, DEC); }
                  if (year>9) { lcd.setCursor(18, 1); lcd.print(year, DEC); } else { lcd.setCursor(18, 1); lcd.print("0"); lcd.print(year, DEC); }     
                 }

                if (menu == 9)
                 { 
                  if (hour>9)   { lcd.setCursor(12, 0); lcd.print(hour, DEC);   } else { lcd.setCursor(12, 0); lcd.print(" "); lcd.print(hour, DEC); }
                  if (minute>9) { lcd.setCursor(15, 0); lcd.print(minute, DEC); } else { lcd.setCursor(15, 0); lcd.print("0"); lcd.print(minute, DEC); }
                  if (second>9) { lcd.setCursor(18, 0); lcd.print(second, DEC); } else { lcd.setCursor(18, 0); lcd.print("0"); lcd.print(second, DEC); }
                 }

                clock_read_flag = 0;
               }                       // end here    

              if (logging_flag == 1 && log_time_flag == 1 && interval_tick >= interval) 
                                     {

                                      #include "Temp_Update.h"

                                      #include "Serial_Bluetooth.h"
                                      
                                      // Serial print function 
                                      switch (format) { case 0: if (date>9) {Serial.print("| "); Serial.print(date, DEC); Serial.print(" ");  }
                                                                 else {Serial.print("| "); Serial.print("0"); Serial.print(date, DEC); Serial.print(" ");}

                                                                switch (month) { case 1:  Serial.print("JAN "); break;
                                                                                 case 2:  Serial.print("FEB "); break;    
                                                                                 case 3:  Serial.print("MAR "); break;
                                                                                 case 4:  Serial.print("APR "); break; 
                                                                                 case 5:  Serial.print("MAY "); break;     
                                                                                 case 6:  Serial.print("JUN "); break;   
                                                                                 case 7:  Serial.print("JUL "); break;    
                                                                                 case 8:  Serial.print("AUG "); break; 
                                                                                 case 9:  Serial.print("SEP "); break;  
                                                                                 case 10: Serial.print("OCT "); break;  
                                                                                 case 11: Serial.print("NOV "); break; 
                                                                                 case 12: Serial.print("DEC "); break; }// end of SWITCH_CASE operator
                          
                                                                Serial.print(year, DEC); Serial.print(" | ");
                        
                                                                 if (hour>9) {Serial.print(hour, DEC); Serial.print(":");}
                                                                 else {Serial.print("0"); Serial.print(hour, DEC); Serial.print(":");}
                                                                  if (minute>9) {Serial.print(minute, DEC); Serial.print(":");}
                                                                   else {Serial.print("0"); Serial.print(minute, DEC); Serial.print(":");}
                                                                    if (second>9) {Serial.print(second, DEC);}
                                                                     else {Serial.print("0"); Serial.print(second, DEC);} 

                                                                Serial.print(" |");    

                                                                for(int i=0;i<numberOfDevices; i++) { Serial.print(" ");
                                                                                                      if (sign_mas[i] == '-') Serial.print("-");
                                                                                                      Serial.print(temp_mas[i],1);
                                                                                                      Serial.print(" |");           }

                                                                if (numberOfDevices == 0) { Serial.print(" NOT FOUND CONNECTED SENSORS"); }                                         
                                                                                              
                                                                Serial.println(); 
                                                                break;

                                                        case 1: if (date>9) {Serial.print(date, DEC);}
                                                                 else {Serial.print("0"); Serial.print(date, DEC);}

                                                                switch (month) { case 1:  Serial.print("-JAN-"); break;
                                                                                 case 2:  Serial.print("-FEB-"); break;    
                                                                                 case 3:  Serial.print("-MAR-"); break;
                                                                                 case 4:  Serial.print("-APR-"); break; 
                                                                                 case 5:  Serial.print("-MAY-"); break;     
                                                                                 case 6:  Serial.print("-JUN-"); break;   
                                                                                 case 7:  Serial.print("-JUL-"); break;    
                                                                                 case 8:  Serial.print("-AUG-"); break; 
                                                                                 case 9:  Serial.print("-SEP-"); break;  
                                                                                 case 10: Serial.print("-OCT-"); break;  
                                                                                 case 11: Serial.print("-NOV-"); break; 
                                                                                 case 12: Serial.print("-DEC-"); break; }// end of SWITCH_CASE operator
                                                       
                                                                Serial.print(year, DEC); Serial.print(",");

                                                                if (hour>9) {Serial.print(hour, DEC); Serial.print(":");}
                                                                 else {Serial.print("0"); Serial.print(hour, DEC); Serial.print(":");}
                                                                  if (minute>9) {Serial.print(minute, DEC); Serial.print(":");}
                                                                   else {Serial.print("0"); Serial.print(minute, DEC); Serial.print(":");}
                                                                    if (second>9) {Serial.print(second, DEC);}
                                                                     else {Serial.print("0"); Serial.print(second, DEC);} 

                                                                Serial.print(",");

                                                                for(int i=0;i<numberOfDevices; i++) { if (sign_mas[i] == '-') Serial.print("-");
                                                                                                      Serial.print(temp_mas[i],1);
                                                                                                      Serial.print(",");             }

                                                                if (numberOfDevices == 0) { Serial.print(" NOT FOUND CONNECTED SENSORS"); }                                         
                                                                Serial.println(); 
                                                                break;                      
                                                                
                                                      } // end of switch-case operator

                                       log_time_flag = 0;
                                       interval_tick = 0;

                                       // SD card record function  
                                       if (sd_card_status == 1) { myFile = SD.open("/data.txt", FILE_APPEND);
                                       
                                                                  switch (format) { case 0: if (date>9) {myFile.print("| "); myFile.print(date, DEC); myFile.print(" ");  }
                                                                                             else {myFile.print("| "); myFile.print("0"); myFile.print(date, DEC); myFile.print(" ");}

                                                                                            switch (month) { case 1:  myFile.print("JAN "); break;
                                                                                                             case 2:  myFile.print("FEB "); break;    
                                                                                                             case 3:  myFile.print("MAR "); break;
                                                                                                             case 4:  myFile.print("APR "); break; 
                                                                                                             case 5:  myFile.print("MAY "); break;     
                                                                                                             case 6:  myFile.print("JUN "); break;   
                                                                                                             case 7:  myFile.print("JUL "); break;    
                                                                                                             case 8:  myFile.print("AUG "); break; 
                                                                                                             case 9:  myFile.print("SEP "); break;  
                                                                                                             case 10: myFile.print("OCT "); break;  
                                                                                                             case 11: myFile.print("NOV "); break; 
                                                                                                             case 12: myFile.print("DEC "); break; }// end of SWITCH_CASE operator
                           
                                                                                            myFile.print(year, DEC); myFile.print(" | ");
                        
                                                                                            if (hour>9) {myFile.print(hour, DEC); myFile.print(":");}
                                                                                             else {myFile.print("0"); myFile.print(hour, DEC); myFile.print(":");}
                                                                                              if (minute>9) {myFile.print(minute, DEC); myFile.print(":");}
                                                                                               else {myFile.print("0"); myFile.print(minute, DEC); myFile.print(":");}
                                                                                                if (second>9) {myFile.print(second, DEC);}
                                                                                                 else {myFile.print("0"); myFile.print(second, DEC);} 

                                                                                            myFile.print(" |");    

                                                                                            for(int i=0;i<numberOfDevices; i++) {  myFile.print(" ");
                                                                                                                                   if (sign_mas[i] == '-') myFile.print("-");
                                                                                                                                   myFile.print(temp_mas[i],1);
                                                                                                                                   myFile.print(" |");           }

                                                                                            if (numberOfDevices == 0) { myFile.print(" NOT FOUND CONNECTED SENSORS"); }                                          
                                                                                              
                                                                                            myFile.println();         
                                                                                            break;

                                                                                            case 1: if (date>9) {myFile.print(date, DEC);}
                                                                                                     else {myFile.print("0"); myFile.print(date, DEC);}

                                                                                                    switch (month) { case 1:  myFile.print("-JAN-"); break;
                                                                                                                     case 2:  myFile.print("-FEB-"); break;    
                                                                                                                     case 3:  myFile.print("-MAR-"); break;
                                                                                                                     case 4:  myFile.print("-APR-"); break; 
                                                                                                                     case 5:  myFile.print("-MAY-"); break;     
                                                                                                                     case 6:  myFile.print("-JUN-"); break;   
                                                                                                                     case 7:  myFile.print("-JUL-"); break;    
                                                                                                                     case 8:  myFile.print("-AUG-"); break; 
                                                                                                                     case 9:  myFile.print("-SEP-"); break;  
                                                                                                                     case 10: myFile.print("-OCT-"); break;  
                                                                                                                     case 11: myFile.print("-NOV-"); break; 
                                                                                                                     case 12: myFile.print("-DEC-"); break; }// end of SWITCH_CASE operator
                                                        
                                                                                                                     myFile.print(year, DEC); myFile.print(",");

                                                                                                                     if (hour>9) {myFile.print(hour, DEC); myFile.print(":");}
                                                                                                                      else {myFile.print("0"); myFile.print(hour, DEC); myFile.print(":");}
                                                                                                                       if (minute>9) {myFile.print(minute, DEC); myFile.print(":");}
                                                                                                                        else {myFile.print("0"); myFile.print(minute, DEC); myFile.print(":");}
                                                                                                                         if (second>9) {myFile.print(second, DEC);}
                                                                                                                          else {myFile.print("0"); myFile.print(second, DEC);} 

                                                                                                                     myFile.print(",");       

                                                                                                                     for(int i=0;i<numberOfDevices; i++) { if (sign_mas[i] == '-') myFile.print("-");
                                                                                                                                                           myFile.print(temp_mas[i],1);
                                                                                                                                                           myFile.print(",");             }

                                                                                                                     if (numberOfDevices == 0) { myFile.print(" NOT FOUND CONNECTED SENSORS"); }                                       

                                                                                                                     myFile.println(" ");               
                                                       
                                                                                                                     break;                      
                                                                
                                                                             } // end of switch-case operator          
                                                               
                                                                myFile.close(); } // end of SD card record
                                     } // end of IF operator            
                 

            } // end of LOOP void

////////////////////////////////////////////////////////////////////////////////////////////////////      

      


void click_enter()  {
                     if (menu == 0 && cursor_position == 0) {
                                                             menu = 10;
                                                             interval_tick = interval;
                                                             logging_flag = 1;
                                                             
                                                             if (backlight_mode == 0)  analogWrite(2, 3);
                                                             
                                                             if (sd_card_status == 1) {myFile = SD.open("/data.txt", FILE_APPEND);
                                                                                       myFile.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -"); 
                                                                                       myFile.println("New log session started");
                                                                                       myFile.println();    
                                                                                       myFile.close();          } // end of IF operator 
                                                             lcd.clear(); delay(5);


                                                            
                                                             lcd.setCursor(7, 0); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 1) goto label_2;
                                                             lcd.setCursor(7, 1); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 2) goto label_2; 
                                                             lcd.setCursor(7, 2); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 3) goto label_2;
                                                             lcd.setCursor(7, 3); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 4) goto label_2;
                                                             lcd.setCursor(18, 0); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 5) goto label_2;
                                                             lcd.setCursor(18, 1); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 6) goto label_2;
                                                             lcd.setCursor(18, 2); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 7) goto label_2;
                                                             lcd.setCursor(18, 3); lcd.write(byte(0)); if (measuring == 0) {lcd.print("C");} else {lcd.print("F");} if (numberOfDevices == 8) goto label_2;
                                                             
                                                             label_2: __asm__("nop");
                                                             
                                                            }

                     if (menu == 0 && cursor_position == 1) { 
                                                             menu = 1;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET TIME & DATE"); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET LOG FORMAT");
                                                             lcd.setCursor(0, 2); lcd.print("  SET INTERVAL"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK "); 
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }     

                     if (menu == 0 && cursor_position == 2) {
                                                             menu = 11;
                                                             lcd.clear(); delay(5); 
                                                             lcd.setCursor(0, 0); lcd.print("  SET LEVEL:"); 
                                                             if (backlight_level < 100) {lcd.setCursor(13, 0); lcd.print(" "); lcd.print(backlight_level, DEC); lcd.print("%");}
                                                              else {lcd.setCursor(13, 0); lcd.print(backlight_level, DEC); lcd.print("%");}
                                                              
                                                             lcd.setCursor(0, 1); lcd.print("  SET MODE : ON  OFF"); 
                                                             if (backlight_mode == 0) {lcd.setCursor(12,1); lcd.print(" "); lcd.setCursor(16,1); lcd.write(byte(1));}
                                                              else {lcd.setCursor(12,1); lcd.write(byte(1)); lcd.setCursor(16,1); lcd.print(" ");}
                                                             
                                                             lcd.setCursor(0, 3); lcd.print("  BACK "); 
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             cursor_position = 0;
                                                             goto label;
                                                            }    

                     if (menu == 11 && cursor_position == 2) { 
                                                              menu = 0;
                                                              analogWrite(2, backlight_level*2.55); 
                                                              lcd.clear(); delay(5);
                                                              lcd.setCursor(0, 0); lcd.print("  START LOGGING"); 
                                                              lcd.setCursor(0, 2); lcd.print("  DEVICE SETUP");
                                                              lcd.setCursor(0, 3); lcd.print("  BACKLIGHT SETUP");
                                                              lcd.setCursor(0, 0); lcd.write(0x3E);
                                                              cursor_position = 0;
                                                              goto label;
                                                             }            

                     if (menu == 11 && cursor_position == 0) { 
                                                              menu = 12;
                                                              lcd.setCursor(0, 0); lcd.print(" "); 
                                                              lcd.setCursor(12, 0); lcd.write(byte(1));
                                                              cursor_position = 0;
                                                              goto label;
                                                             }         

                     if (menu == 11 && cursor_position == 1) { 
                                                              menu = 13;
                                                              lcd.setCursor(0, 1); lcd.print(" "); 
                                                              
                                                              if (backlight_mode == 0) {lcd.setCursor(12,1); lcd.print(" "); lcd.setCursor(16,1); lcd.write(byte(1));}
                                                              else {lcd.setCursor(12,1); lcd.write(byte(1)); lcd.setCursor(16,1); lcd.print(" ");}
                                                              
                                                              cursor_position = 1;
                                                              goto label;
                                                             }         

                     if (menu == 13 && cursor_position == 1) { 
                                                              EEPROM.write(16, backlight_mode); 
                                                              //EEPROM.end();
                                                              EEPROM.commit();
                                                              menu = 11;
                                                              lcd.setCursor(0, 1); lcd.write(0x3E);
                                                              cursor_position = 1;
                                                              goto label;
                                                             }                                                                                        

                     if (menu == 12 && cursor_position == 0) { 
                                                              EEPROM.write(12, backlight_level); 
                                                              //EEPROM.end();
                                                              EEPROM.commit();
                                                              menu = 11;
                                                              lcd.setCursor(12, 0); lcd.print(" "); 
                                                              lcd.setCursor(0, 0); lcd.write(byte(1));
                                                              lcd.setCursor(0, 0); lcd.write(0x3E);
                                                              goto label;
                                                             }                                                                                                                                                                     

                     if (menu == 1 && cursor_position == 3) { 
                                                             menu = 0;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  START LOGGING"); 
                                                             lcd.setCursor(0, 2); lcd.print("  DEVICE SETUP");
                                                             lcd.setCursor(0, 3); lcd.print("  BACKLIGHT SETUP");
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             cursor_position = 0;
                                                             goto label;
                                                            }    

                     if (menu == 1 && cursor_position == 0) { 
                             
                                                             menu = 2;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET TIME:   :  :  "); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET DATE:   /  /  ");
                                                             lcd.setCursor(0, 3); lcd.print("  BACK");

                                                             if (hour>9) { lcd.setCursor(12, 0); lcd.print(hour, DEC); }
                                                              else { lcd.setCursor(12, 0); lcd.print(" "); lcd.print(hour, DEC); }
                                                              
                                                               if (minute>9) { lcd.setCursor(15, 0); lcd.print(minute, DEC); }
                                                                else { lcd.setCursor(15, 0); lcd.print("0"); lcd.print(minute, DEC); }
                                                                 
                                                                 if (second>9) { lcd.setCursor(18, 0); lcd.print(second, DEC); }
                                                                  else { lcd.setCursor(18, 0); lcd.print("0"); lcd.print(second, DEC); }

                                                             if (date>9) { lcd.setCursor(12, 1); lcd.print(date, DEC); }
                                                              else { lcd.setCursor(12, 1); lcd.print(" "); lcd.print(date, DEC); }

                                                               if (month>9) { lcd.setCursor(15, 1); lcd.print(month, DEC); }
                                                                else { lcd.setCursor(15, 1); lcd.print("0"); lcd.print(month, DEC); }

                                                                 if (year>9) { lcd.setCursor(18, 1); lcd.print(year, DEC); }
                                                                  else { lcd.setCursor(18, 1); lcd.print("0"); lcd.print(year, DEC); }     

                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }                                           

                     if (menu == 2 && cursor_position == 2) { 
                                                             menu = 1;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET TIME & DATE"); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET LOG FORMAT");
                                                             lcd.setCursor(0, 2); lcd.print("  SET INTERVAL"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK "); 
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }   

                     if (menu == 1 && cursor_position == 1) { 
                                                             menu = 3;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET:  TXT   CSV"); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET:  "); lcd.write(byte(0));lcd.print("C"); lcd.setCursor(14, 1); lcd.write(byte(0));lcd.print("F");
                                                             lcd.setCursor(0, 3); lcd.print("  BACK");
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             if (format == 0)
                                                               {
                                                                lcd.setCursor(13, 0); lcd.print(" "); 
                                                                lcd.setCursor(7, 0); lcd.write(byte(1)); 
                                                               }

                                                              if (format == 1)
                                                               {
                                                                lcd.setCursor(7, 0); lcd.print(" "); 
                                                                lcd.setCursor(13, 0); lcd.write(byte(1));  
                                                               } 

                                                             if (measuring == 0)
                                                               {
                                                                lcd.setCursor(13, 1); lcd.print(" "); 
                                                                lcd.setCursor(7, 1); lcd.write(byte(1)); 
                                                               }

                                                              if (measuring == 1)
                                                               {
                                                                lcd.setCursor(7, 1); lcd.print(" "); 
                                                                lcd.setCursor(13, 1); lcd.write(byte(1));  
                                                               }   

                                                             
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            } 

                     if (menu == 3 && cursor_position == 2) { 
                                                             menu = 1;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET TIME & DATE"); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET LOG FORMAT");
                                                             lcd.setCursor(0, 2); lcd.print("  SET INTERVAL"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK "); 
                                                             lcd.setCursor(0, 1); lcd.write(0x3E);
                                                             cursor_position = 1;
                                                             goto label;
                                                             
                                                            }    

                     if (menu == 1 && cursor_position == 2) { 
                                                             menu = 4;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET INTR:      sec"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK");
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             lcd.setCursor(13, 0); lcd.print(interval, DEC);
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }   

                     if (menu == 4 && cursor_position == 1) { 
                                                             menu = 1;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET TIME & DATE"); 
                                                             lcd.setCursor(0, 1); lcd.print("  SET LOG FORMAT");
                                                             lcd.setCursor(0, 2); lcd.print("  SET INTERVAL"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK "); 
                                                             lcd.setCursor(0, 2); lcd.write(0x3E);
                                                             cursor_position = 2;
                                                             goto label;
                                                             
                                                            }     

                     if (menu == 4 && cursor_position == 0) {
                                                             menu = 5; 
                                                             lcd.setCursor(0, 0); lcd.print(" "); 
                                                             lcd.setCursor(11, 0); lcd.write(byte(1));
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }      

                     if (menu == 5 && cursor_position == 0) {
                                                             EEPROM.write(0, interval); 
                                                             //EEPROM.end();
                                                             EEPROM.commit();
                                                             menu = 4;
                                                             lcd.clear(); delay(5);
                                                             lcd.setCursor(0, 0); lcd.print("  SET INTR:      sec"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK");
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             lcd.setCursor(13, 0); lcd.print(interval, DEC);
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }     

                    if (menu == 3 && cursor_position == 0) {
                                                             menu = 6;
                                                             lcd.setCursor(0, 0); lcd.print(" "); 
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }       

                    if (menu == 6 && cursor_position == 0) {
                                                             menu = 3;
                                                             EEPROM.write(4, format); 
                                                             //EEPROM.end();
                                                             EEPROM.commit();
                                                             lcd.setCursor(0, 0); lcd.print("  SET:  TXT   CSV"); 
                                                             lcd.setCursor(0, 3); lcd.print("  BACK");
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                              if (format == 0)
                                                               {
                                                                lcd.setCursor(13, 0); lcd.print(" "); 
                                                                lcd.setCursor(7, 0); lcd.write(byte(1)); 
                                                               }

                                                              if (format == 1)
                                                               {
                                                                lcd.setCursor(7, 0); lcd.print(" "); 
                                                                lcd.setCursor(13, 0); lcd.write(byte(1));  
                                                               } 
                                                             cursor_position = 0;
                                                             goto label;
                                                             
                                                            }     

                     if (menu == 3 && cursor_position == 1) {
                                                             menu = 7;
                                                             lcd.setCursor(0, 1); lcd.print(" "); 
                                                             cursor_position = 1;
                                                             goto label;
                                                             
                                                            }       

                     if (menu == 7 && cursor_position == 1) {
                                                             menu = 3;
                                                             EEPROM.write(8, measuring); 
                                                             //EEPROM.end();
                                                             EEPROM.commit();
                                                             //lcd.setCursor(0, 0); lcd.print("  SET:  TXT   CSV"); 
                                                             //lcd.setCursor(0, 3); lcd.print("  BACK");
                                                             lcd.setCursor(0, 1); lcd.write(0x3E);
                                                              if (measuring == 0)
                                                               {
                                                                lcd.setCursor(13, 1); lcd.print(" "); 
                                                                lcd.setCursor(7, 1); lcd.write(byte(1)); 
                                                               }

                                                              if (measuring == 1)
                                                               {
                                                                lcd.setCursor(7, 1); lcd.print(" "); 
                                                                lcd.setCursor(13, 1); lcd.write(byte(1));  
                                                               } 
                                                             cursor_position = 1;
                                                             goto label;
                                                             
                                                            }     

                    if (menu == 2 && cursor_position == 0) {
                                                             //EIMSK=(0<<INT1) | (0<<INT0);    // Stop external interrupt
                                                             detachInterrupt(17);
                                                             menu = 8;
                                                             lcd.setCursor(0, 0); lcd.print(" "); 
                                                             cursor_position = 0;
                                                             lcd.setCursor(11, 0); lcd.write(byte(1));  
                                                             goto label;
                                                             
                                                            }                                                                                                                                 
                                                                                                                                                                                                                                                                                          
                     if (menu == 8 && cursor_position == 0) {
                                                             menu = 2;                                       // HERE SAVE AND WRITE "TIME" TO RTC CLOCK
                                                             lcd.setCursor(0, 0); lcd.write(0x3E);
                                                             lcd.setCursor(11, 0); lcd.print(" "); 
                                                             lcd.setCursor(14, 0); lcd.print(":");
                                                             lcd.setCursor(17, 0); lcd.print(":");
                                                             time_select = 0;
                                                             cursor_position = 0; 
                                                             set_new_time = 1;
                                                             goto label;
                                                             
                                                            }    
                                                              
                    if (menu == 2 && cursor_position == 1) {
                                                             //EIMSK=(0<<INT1) | (0<<INT0);                    // Stop external interrupt
                                                             detachInterrupt(17);
                                                             menu = 9;                                        
                                                             lcd.setCursor(0, 1); lcd.print(" "); 
                                                             cursor_position = 1;
                                                             lcd.setCursor(11, 1); lcd.write(byte(1));  
                                                             goto label;
                                                             
                                                            }  


                    if (menu == 9 && cursor_position == 1) {
                                                             menu = 2;                                       // HERE SAVE AND WRITE "DATE" TO RTC CLOCK
                                                             lcd.setCursor(0, 1); lcd.write(0x3E);
                                                             lcd.setCursor(11, 1); lcd.print(" "); 
                                                             lcd.setCursor(14, 1); lcd.print("/");
                                                             lcd.setCursor(17, 1); lcd.print("/");
                                                             date_select = 0;
                                                             cursor_position = 1; 
                                                             set_new_date = 1;
                                                             goto label;
                                                             
                                                            }                                                    
                                                                                                                                                                          
                    label: __asm__("nop");
                     
                    } // end of ENTER void

void click_select() {

                     if (menu == 0)
                      {
                        cursor_position++;

                        
                         if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.write(0x3E);
                                                   }
                       
                         if (cursor_position == 2) { 
                                                    lcd.setCursor(0, 3); lcd.write(0x3E); 
                                                    lcd.setCursor(0, 2); lcd.print(" ");
                                                   }

                         if (cursor_position == 3) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.write(0x3E); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");
                                                   }   
                      }

                      if (menu == 1)
                      {
                        cursor_position++;

                        
                        if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 1); lcd.write(0x3E);
                                                  }
                       
                        if (cursor_position == 2) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 2); lcd.write(0x3E);
                                                  }

                        if (cursor_position == 3 ) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 3); lcd.write(0x3E);
                                                  }                          

                        if (cursor_position == 4 ) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 0); lcd.write(0x3E);
                                                  }                                                    
                      } 

                       if (menu == 2)
                      {
                        cursor_position++;

                        
                        if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 1); lcd.write(0x3E);
                                                  }
                       
                        if (cursor_position == 2) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 3); lcd.write(0x3E);
                                                  }

                        if (cursor_position == 3 ) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 0); lcd.write(0x3E);
                                                  }                                                    
                      } 

                       if (menu == 3)
                      {
                        cursor_position++;

                        
                        if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 1); lcd.write(0x3E);
                                                  }
                       
                        if (cursor_position == 2) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 3); lcd.write(0x3E);
                                                  }

                        if (cursor_position == 3 ) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 0); lcd.write(0x3E);
                                                  }                                                    
                      } 

                       if (menu == 4)
                      {
                        cursor_position++;

                        
                        if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 4); lcd.write(0x3E);
                                                  }
                       

                        if (cursor_position == 2 ) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.print(" ");
                                                    lcd.setCursor(0, 1); lcd.print(" "); 
                                                    lcd.setCursor(0, 2); lcd.print(" "); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");  
                                                    lcd.setCursor(0, 0); lcd.write(0x3E);
                                                  }                                                    
                      } 

                      if (menu == 6)
                      {
                       format++;
                       if (format >= 2) {format = 0;}

                       
                       if (format == 0)
                        {
                         lcd.setCursor(13, 0); lcd.print(" "); 
                         lcd.setCursor(7, 0); lcd.write(byte(1)); 
                        }

                       if (format == 1)
                        {
                         lcd.setCursor(7, 0); lcd.print(" "); 
                         lcd.setCursor(13, 0); lcd.write(byte(1));  
                        } 

                        cursor_position = 0;   
                      }

                        if (menu == 7)
                      {
                       measuring++;
                       if (measuring >= 2) {measuring = 0;}

                       
                       if (measuring == 0)
                        {
                         lcd.setCursor(13, 1); lcd.print(" "); 
                         lcd.setCursor(7, 1); lcd.write(byte(1)); 
                        }

                       if (measuring == 1)
                        {
                         lcd.setCursor(7, 1); lcd.print(" "); 
                         lcd.setCursor(13, 1); lcd.write(byte(1));  
                        } 

                        cursor_position = 1;   
                      }

                      if (menu == 8)
                      {
                       time_select++;
                       if (time_select == 3) {time_select = 0;}

                       if (time_select == 0)
                       { 
                        lcd.setCursor(11, 0); lcd.write(byte(1)); 
                        lcd.setCursor(14, 0); lcd.print(":"); 
                        lcd.setCursor(17, 0); lcd.print(":"); 
                       }

                       if (time_select == 1)
                       { 
                        lcd.setCursor(11, 0); lcd.print(" "); 
                        lcd.setCursor(14, 0); lcd.write(byte(1)); 
                        lcd.setCursor(17, 0); lcd.print(":"); 
                       }

                       if (time_select == 2)
                       { 
                        lcd.setCursor(11, 0); lcd.print(" "); 
                        lcd.setCursor(14, 0); lcd.print(":");
                        lcd.setCursor(17, 0); lcd.write(byte(1));
                       }
                        
                      }

                       if (menu == 9)
                      {
                       date_select++;
                       if (date_select == 3) {date_select = 0;}

                       if (date_select == 0)
                       { 
                        lcd.setCursor(11, 1); lcd.write(byte(1)); 
                        lcd.setCursor(14, 1); lcd.print("/"); 
                        lcd.setCursor(17, 1); lcd.print("/"); 
                       }

                       if (date_select == 1)
                       { 
                        lcd.setCursor(11, 1); lcd.print(" "); 
                        lcd.setCursor(14, 1); lcd.write(byte(1)); 
                        lcd.setCursor(17, 1); lcd.print("/"); 
                       }

                       if (date_select == 2)
                       { 
                        lcd.setCursor(11, 1); lcd.print(" "); 
                        lcd.setCursor(14, 1); lcd.print("/");
                        lcd.setCursor(17, 1); lcd.write(byte(1));
                       }
                        
                      }

                      ///////////////     here deleted MENU == 10


                      

                       if (menu == 11)
                       {
                        cursor_position++;

                        
                         if (cursor_position == 1) { 
                                                    lcd.setCursor(0, 0); lcd.print(" "); 
                                                    lcd.setCursor(0, 1); lcd.write(0x3E);
                                                   }
                       
                         if (cursor_position == 2) { 
                                                    lcd.setCursor(0, 3); lcd.write(0x3E); 
                                                    lcd.setCursor(0, 1); lcd.print(" ");
                                                   }

                         if (cursor_position == 3) { 
                                                    cursor_position = 0;
                                                    lcd.setCursor(0, 0); lcd.write(0x3E); 
                                                    lcd.setCursor(0, 3); lcd.print(" ");
                                                   }   
                      }

                       if (menu == 13)
                       {
                        backlight_mode++;
                        if (backlight_mode == 2) {backlight_mode = 0;}

                       
                       if (backlight_mode == 0)
                        {
                         lcd.setCursor(16, 1); lcd.write(byte(1)); 
                         lcd.setCursor(12, 1); lcd.print(" "); 
                        }

                       if (backlight_mode == 1)
                        {
                         lcd.setCursor(12, 1); lcd.write(byte(1)); 
                         lcd.setCursor(16, 1); lcd.print(" "); 
                        } 

                        cursor_position = 1;   
                       }
                       
                 
                      } // end select void

                     

void click_up()     {

                        if (menu == 5)
                        {
                         interval++;
                         if (interval == 7201) {interval = 1;}
                         lcd.setCursor(13, 0); lcd.print(" ");
                         lcd.setCursor(14, 0); lcd.print(" ");
                         lcd.setCursor(15, 0); lcd.print(" ");
                         lcd.setCursor(16, 0); lcd.print(" ");
                         lcd.setCursor(13, 0); lcd.print(interval, DEC);
                     
                        } // end

                        /////////////////////////////////////////////////////////////

                         if (menu == 8 && time_select == 0)
                        {
                         hour++;
                         if (hour == 24) {hour = 0;}

                         if (hour>9) { lcd.setCursor(12, 0); lcd.print(hour, DEC); }
                          else { lcd.setCursor(12, 0); lcd.print(" "); lcd.print(hour, DEC); }
                     
                         } // end
                         

                         if (menu == 8 && time_select == 1)
                        {
                         minute++;
                         if (minute == 60) {minute = 0;}

                         if (minute>9) { lcd.setCursor(15, 0); lcd.print(minute, DEC); }
                          else { lcd.setCursor(15, 0); lcd.print("0"); lcd.print(minute, DEC); }
                     
                         } // end

                        if (menu == 8 && time_select == 2)
                        {
                         second++;
                         if (second == 60) {second = 0;}

                         if (second>9) { lcd.setCursor(18, 0); lcd.print(second, DEC); }
                          else { lcd.setCursor(18, 0); lcd.print("0"); lcd.print(second, DEC); }
                     
                         } // end

                         //////////////////////////////////////////////////////////////

                           if (menu == 9 && date_select == 0)
                        {
                         date++;
                         if (date == 32) {date = 1 ;}

                         if (date>9) { lcd.setCursor(12, 1); lcd.print(date, DEC); }
                          else { lcd.setCursor(12, 1); lcd.print(" "); lcd.print(date, DEC); }
                     
                         } // end

                         if (menu == 9 && date_select == 1)
                        {
                         month++;
                         if (month == 13) {month = 1;}

                         if (month>9) { lcd.setCursor(15, 1); lcd.print(month, DEC); }
                          else { lcd.setCursor(15, 1); lcd.print("0"); lcd.print(month, DEC); }
                     
                         } // end

                        if (menu == 9 && date_select == 2)
                        {
                         year++;
                         if (year == 100) {year = 0;}

                         if (year>9) { lcd.setCursor(18, 1); lcd.print(year, DEC); }
                          else { lcd.setCursor(18, 1); lcd.print("0"); lcd.print(year, DEC); }
                     
                         } // end

                         ////////////////////////////////////////////////

                        if (menu == 12)
                        {
                         backlight_level = backlight_level + 5;
                         
                         if (backlight_level == 105) {backlight_level = 100;}

                         if (backlight_level < 100) {lcd.setCursor(13, 0); lcd.print(" "); lcd.print(backlight_level, DEC); lcd.print("%");}
                          else {lcd.setCursor(13, 0); lcd.print(backlight_level, DEC); lcd.print("%");}
                          
                         analogWrite(2, (backlight_level * 2.55));    // set backlight level
                     
                        } // end

                                               
                         

                    }    // end of void click_up

void click_down()   {
                        if (menu == 5)
                        {
                         interval--;
                         if (interval == 0) {interval = 7200;}
                         lcd.setCursor(13, 0); lcd.print(" ");
                         lcd.setCursor(14, 0); lcd.print(" ");
                         lcd.setCursor(15, 0); lcd.print(" ");
                         lcd.setCursor(16, 0); lcd.print(" ");
                         lcd.setCursor(13, 0); lcd.print(interval, DEC);
                     
                        } // end

                        if (menu == 8 && time_select == 0)
                        {
                         hour--;
                         if (hour == 255) {hour = 23;}

                         if (hour>9) { lcd.setCursor(12, 0); lcd.print(hour, DEC); }
                          else { lcd.setCursor(12, 0); lcd.print(" "); lcd.print(hour, DEC); }
                     
                        } // end

                        if (menu == 8 && time_select == 1)
                        {
                         minute--;
                         if (minute == 255) {minute = 59;}

                         if (minute>9) { lcd.setCursor(15, 0); lcd.print(minute, DEC); }
                          else { lcd.setCursor(15, 0); lcd.print("0"); lcd.print(minute, DEC); }
                     
                         } // end 

                        if (menu == 8 && time_select == 2)
                        {
                         second--;
                         if (second == 255) {second = 59;}

                         if (second>9) { lcd.setCursor(18, 0); lcd.print(second, DEC); }
                          else { lcd.setCursor(18, 0); lcd.print("0"); lcd.print(second, DEC); }
                     
                         } // end 

                         //////////////////////////////////////////////////////////////////////////////////////

                        if (menu == 9 && date_select == 0)
                        {
                         date--;
                         if (date == 0) {date = 31 ;}

                         if (date>9) { lcd.setCursor(12, 1); lcd.print(date, DEC); }
                          else { lcd.setCursor(12, 1); lcd.print(" "); lcd.print(date, DEC); }
                     
                         } // end

                        if (menu == 9 && date_select == 1)
                        {
                         month--;
                         if (month == 0) {month = 12;}

                         if (month>9) { lcd.setCursor(15, 1); lcd.print(month, DEC); }
                          else { lcd.setCursor(15, 1); lcd.print("0"); lcd.print(month, DEC); }
                     
                         } // end

                        if (menu == 9 && date_select == 2)
                        {
                         year--;
                         if (year == 255) {year = 99;}

                         if (year>9) { lcd.setCursor(18, 1); lcd.print(year, DEC); }
                          else { lcd.setCursor(18, 1); lcd.print("0"); lcd.print(year, DEC); }
                     
                         } // end

                         ////////////////////////////////////////////////

                        if (menu == 12)
                        {
                         backlight_level = backlight_level - 5;
                         
                         if (backlight_level == 0) {backlight_level = 5;}
                
                         if (backlight_level < 100 && backlight_level >= 10) {lcd.setCursor(13, 0); lcd.print(" "); lcd.print(backlight_level, DEC); lcd.print("%");}
                         if (backlight_level <  10) {lcd.setCursor(14, 0); lcd.print(" "); lcd.print(backlight_level, DEC); lcd.print("%");}
                         if (backlight_level == 100) {lcd.setCursor(13, 0); lcd.print(backlight_level, DEC); lcd.print("%");}

                         analogWrite(2, (backlight_level * 2.55));    // set backlight level
                     
                        } // end

                      
                    } 

void long_select() {  if (menu == 10)
                       { 
                        menu = 0;
                        logging_flag = 0;
                        analogWrite(2, backlight_level*2.55);  
                        
                        
                        if (sd_card_status == 1) { myFile = SD.open("/data.txt", FILE_APPEND);
                                                   myFile.println(); 
                                                   myFile.println("Log session ended"); 
                                                   myFile.println("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
                                                   myFile.close(); }                                                                // end of IF operator
                        
                        
                        lcd.clear(); delay(5);
                        lcd.setCursor(0, 0); lcd.print("  START LOGGING"); 
                        lcd.setCursor(0, 2); lcd.print("  DEVICE SETUP");
                        lcd.setCursor(0, 3); lcd.print("  BACKLIGHT SETUP");
                        lcd.setCursor(0, 0); lcd.write(0x3E);
                        cursor_position = 0;
                       } // end
  
                    } // end of LONG SELECT function

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}                    
