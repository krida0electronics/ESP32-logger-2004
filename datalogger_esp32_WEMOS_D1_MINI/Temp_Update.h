
/////////////////////////////////////////////////////////////////////////////////////
//
// This function read temperature values from ds18b20 sensors and update them on LCD
//
/////////////////////////////////////////////////////////////////////////////////////

sensors.requestTemperatures();
 
// читаем значения и пишем в массив
for(int i=0;i<numberOfDevices; i++) { if(sensors.getAddress(tempDeviceAddress, i)) {temp_mas[i] = sensors.getTempC(tempDeviceAddress);} 
                                    
                                      if (measuring == 1) { temp_mas[i] = (temp_mas[i]*1.8)+32;} }    // convert to Fahrenheit   

// проверяем знак и пишем его в массив
for(int i=0;i<numberOfDevices; i++) { if (temp_mas[i] < 0) {sign_mas[i] = 0x2D;} else {sign_mas[i] = 0x2B;} } 

// возводим значение в модуль  
for(int i=0;i<numberOfDevices; i++) { if (temp_mas[i] < 0) {temp_mas[i] = temp_mas[i] * -1;}  } 

// умножаем значение температуры на x10
for(int i=0;i<numberOfDevices; i++) { temp_x10_mas[i] = temp_mas[i] * 10; } 

// перепишем массив для float
for(int i=0;i<numberOfDevices; i++) { temp_mas[i] = temp_x10_mas[i]/10.0; } 

// двухмерный массив, 8 строк 4 столбцов - temp_values_mas[8][4] = { };
// раскладывает температуру на числа и пишем их в массив. i значение температуры, j разложенные числа
for (int i=0;i<numberOfDevices; i++) {
                                      temp_values_mas[i][0] =  temp_x10_mas[i] / 1000;
                                      temp_values_mas[i][1] =  (temp_x10_mas[i] /100) % 10;
                                      temp_values_mas[i][2] =  (temp_x10_mas[i] / 10) % 10;
                                      temp_values_mas[i][3] =  temp_x10_mas[i] % 10;
                                     }
                

// выводим температуру на дисплей
if(numberOfDevices>0) 
{
 lcd.setCursor(0, 0); lcd.print("  "); 
 if(temp_mas[0]<10 && temp_mas[0]>=0)   {lcd.setCursor(2, 0); lcd.write(sign_mas[0]); lcd.print(temp_values_mas[0][2]); lcd.print("."); lcd.print(temp_values_mas[0][3]);}    
  if(temp_mas[0]>=10 && temp_mas[0]<100) {lcd.setCursor(1, 0); lcd.write(sign_mas[0]); lcd.print(temp_values_mas[0][1]); lcd.print(temp_values_mas[0][2]); lcd.print("."); lcd.print(temp_values_mas[0][3]);}
   if(temp_mas[0]>=100)                   {lcd.setCursor(0, 0); lcd.write(sign_mas[0]); lcd.print(temp_values_mas[0][0]); lcd.print(temp_values_mas[0][1]); lcd.print(temp_values_mas[0][2]); lcd.print("."); lcd.print(temp_values_mas[0][3]);} 
    if (numberOfDevices == 1) goto label_3;

 lcd.setCursor(0, 1); lcd.print("  "); 
 if(temp_mas[1]<10 && temp_mas[1]>=0)   {lcd.setCursor(2, 1); lcd.write(sign_mas[1]); lcd.print(temp_values_mas[1][2]); lcd.print("."); lcd.print(temp_values_mas[1][3]);}    
  if(temp_mas[1]>=10 && temp_mas[1]<100) {lcd.setCursor(1, 1); lcd.write(sign_mas[1]); lcd.print(temp_values_mas[1][1]); lcd.print(temp_values_mas[1][2]); lcd.print("."); lcd.print(temp_values_mas[1][3]);}
   if(temp_mas[1]>=100)                   {lcd.setCursor(0, 1); lcd.write(sign_mas[1]); lcd.print(temp_values_mas[1][0]); lcd.print(temp_values_mas[1][1]); lcd.print(temp_values_mas[1][2]); lcd.print("."); lcd.print(temp_values_mas[1][3]);} 
    if (numberOfDevices == 2) goto label_3;

 lcd.setCursor(0, 2); lcd.print("  "); 
 if(temp_mas[2]<10 && temp_mas[2]>=0)   {lcd.setCursor(2, 2); lcd.write(sign_mas[2]); lcd.print(temp_values_mas[2][2]); lcd.print("."); lcd.print(temp_values_mas[2][3]);}    
  if(temp_mas[2]>=10 && temp_mas[2]<100) {lcd.setCursor(1, 2); lcd.write(sign_mas[2]); lcd.print(temp_values_mas[2][1]); lcd.print(temp_values_mas[2][2]); lcd.print("."); lcd.print(temp_values_mas[2][3]);}
   if(temp_mas[2]>=100)                   {lcd.setCursor(0, 2); lcd.write(sign_mas[2]); lcd.print(temp_values_mas[2][0]); lcd.print(temp_values_mas[2][1]); lcd.print(temp_values_mas[2][2]); lcd.print("."); lcd.print(temp_values_mas[2][3]);} 
    if (numberOfDevices == 3) goto label_3;      

 lcd.setCursor(0, 3); lcd.print("  "); 
 if(temp_mas[3]<10 && temp_mas[3]>=0)   {lcd.setCursor(2, 3); lcd.write(sign_mas[3]); lcd.print(temp_values_mas[3][2]); lcd.print("."); lcd.print(temp_values_mas[3][3]);}    
  if(temp_mas[3]>=10 && temp_mas[3]<100) {lcd.setCursor(1, 3); lcd.write(sign_mas[3]); lcd.print(temp_values_mas[3][1]); lcd.print(temp_values_mas[3][2]); lcd.print("."); lcd.print(temp_values_mas[3][3]);}
   if(temp_mas[3]>=100)                   {lcd.setCursor(0, 3); lcd.write(sign_mas[3]); lcd.print(temp_values_mas[3][0]); lcd.print(temp_values_mas[3][1]); lcd.print(temp_values_mas[3][2]); lcd.print("."); lcd.print(temp_values_mas[3][3]);} 
    if (numberOfDevices == 4) goto label_3;   

 lcd.setCursor(11, 0); lcd.print("  "); 
 if(temp_mas[4]<10 && temp_mas[4]>=0)   {lcd.setCursor(13, 0); lcd.write(sign_mas[4]); lcd.print(temp_values_mas[4][2]); lcd.print("."); lcd.print(temp_values_mas[4][3]);}    
  if(temp_mas[4]>=10 && temp_mas[4]<100) {lcd.setCursor(12, 0); lcd.write(sign_mas[4]); lcd.print(temp_values_mas[4][1]); lcd.print(temp_values_mas[4][2]); lcd.print("."); lcd.print(temp_values_mas[4][3]);}
   if(temp_mas[4]>=100)                   {lcd.setCursor(11, 0); lcd.write(sign_mas[4]); lcd.print(temp_values_mas[4][0]); lcd.print(temp_values_mas[4][1]); lcd.print(temp_values_mas[4][2]); lcd.print("."); lcd.print(temp_values_mas[4][3]);} 
    if (numberOfDevices == 5) goto label_3;          

 lcd.setCursor(11, 1); lcd.print("  "); 
 if(temp_mas[5]<10 && temp_mas[5]>=0)   {lcd.setCursor(13, 1); lcd.write(sign_mas[5]); lcd.print(temp_values_mas[5][2]); lcd.print("."); lcd.print(temp_values_mas[5][3]);}    
  if(temp_mas[5]>=10 && temp_mas[5]<100) {lcd.setCursor(12, 1); lcd.write(sign_mas[5]); lcd.print(temp_values_mas[5][1]); lcd.print(temp_values_mas[5][2]); lcd.print("."); lcd.print(temp_values_mas[5][3]);}
   if(temp_mas[5]>=100)                   {lcd.setCursor(11, 1); lcd.write(sign_mas[5]); lcd.print(temp_values_mas[5][0]); lcd.print(temp_values_mas[5][1]); lcd.print(temp_values_mas[5][2]); lcd.print("."); lcd.print(temp_values_mas[5][3]);} 
    if (numberOfDevices == 6) goto label_3;             

 lcd.setCursor(11, 2); lcd.print("  "); 
 if(temp_mas[6]<10 && temp_mas[6]>=0)   {lcd.setCursor(13, 2); lcd.write(sign_mas[6]); lcd.print(temp_values_mas[6][2]); lcd.print("."); lcd.print(temp_values_mas[6][3]);}    
  if(temp_mas[6]>=10 && temp_mas[6]<100) {lcd.setCursor(12, 2); lcd.write(sign_mas[6]); lcd.print(temp_values_mas[6][1]); lcd.print(temp_values_mas[6][2]); lcd.print("."); lcd.print(temp_values_mas[6][3]);}
   if(temp_mas[6]>=100)                   {lcd.setCursor(11, 2); lcd.write(sign_mas[6]); lcd.print(temp_values_mas[6][0]); lcd.print(temp_values_mas[6][1]); lcd.print(temp_values_mas[6][2]); lcd.print("."); lcd.print(temp_values_mas[6][3]);} 
    if (numberOfDevices == 7) goto label_3;         

 lcd.setCursor(11, 3); lcd.print("  "); 
 if(temp_mas[7]<10 && temp_mas[7]>=0)   {lcd.setCursor(13, 3); lcd.write(sign_mas[7]); lcd.print(temp_values_mas[7][2]); lcd.print("."); lcd.print(temp_values_mas[7][3]);}    
  if(temp_mas[7]>=10 && temp_mas[7]<100) {lcd.setCursor(12, 3); lcd.write(sign_mas[7]); lcd.print(temp_values_mas[7][1]); lcd.print(temp_values_mas[7][2]); lcd.print("."); lcd.print(temp_values_mas[7][3]);}
   if(temp_mas[7]>=100)                   {lcd.setCursor(11, 3); lcd.write(sign_mas[7]); lcd.print(temp_values_mas[7][0]); lcd.print(temp_values_mas[7][1]); lcd.print(temp_values_mas[7][2]); lcd.print("."); lcd.print(temp_values_mas[7][3]);} 
    if (numberOfDevices == 8) goto label_3;       

 label_3: __asm__("nop");   
}
 else
 {
  lcd.clear(); delay(1);
  lcd.setCursor(1, 1); lcd.print("NOT FOUND");
  lcd.setCursor(1, 2); lcd.print("CONNECTED SENSORS");
 }
