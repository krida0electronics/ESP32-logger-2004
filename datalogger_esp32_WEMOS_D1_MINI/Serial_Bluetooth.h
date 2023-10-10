
//////////////////////////////////////////////////// 
//                                                // 
// Integrated bluetooth serial print output code  //
//                                                // 
////////////////////////////////////////////////////

switch (format) { case 0: if (date>9) {SerialBT.print("| "); SerialBT.print(date, DEC); SerialBT.print(" ");  }
                           else {SerialBT.print("| "); SerialBT.print("0"); SerialBT.print(date, DEC); SerialBT.print(" ");}

                          switch (month) { case 1:  SerialBT.print("JAN "); break;
                                           case 2:  SerialBT.print("FEB "); break;    
                                           case 3:  SerialBT.print("MAR "); break;
                                           case 4:  SerialBT.print("APR "); break; 
                                           case 5:  SerialBT.print("MAY "); break;     
                                           case 6:  SerialBT.print("JUN "); break;   
                                           case 7:  SerialBT.print("JUL "); break;    
                                           case 8:  SerialBT.print("AUG "); break; 
                                           case 9:  SerialBT.print("SEP "); break;  
                                           case 10: SerialBT.print("OCT "); break;  
                                           case 11: SerialBT.print("NOV "); break; 
                                           case 12: SerialBT.print("DEC "); break; }// end of SWITCH_CASE operator
                          
                          SerialBT.print(year, DEC); SerialBT.print(" | ");
                        
                          if (hour>9) {SerialBT.print(hour, DEC); SerialBT.print(":");}
                           else {SerialBT.print("0"); SerialBT.print(hour, DEC); SerialBT.print(":");}
                            if (minute>9) {SerialBT.print(minute, DEC); SerialBT.print(":");}
                             else {SerialBT.print("0"); SerialBT.print(minute, DEC); SerialBT.print(":");}
                              if (second>9) {SerialBT.print(second, DEC);}
                               else {SerialBT.print("0"); SerialBT.print(second, DEC);} 
                          
                          SerialBT.print(" |");    

                          for(int i=0;i<numberOfDevices; i++) { SerialBT.print(" ");
                                                                if (sign_mas[i] == '-') SerialBT.print("-");
                                                                SerialBT.print(temp_mas[i],1);
                                                                SerialBT.print(" |");           }

                          if (numberOfDevices == 0) { SerialBT.print(" NOT FOUND CONNECTED SENSORS"); }                                      
                                                                                              
                          SerialBT.println(); 
                          break;

                  case 1: if (date>9) {SerialBT.print(date, DEC);}
                           else {SerialBT.print("0"); SerialBT.print(date, DEC);}

                          switch (month) { case 1:  SerialBT.print("-JAN-"); break;
                                           case 2:  SerialBT.print("-FEB-"); break;    
                                           case 3:  SerialBT.print("-MAR-"); break;
                                           case 4:  SerialBT.print("-APR-"); break; 
                                           case 5:  SerialBT.print("-MAY-"); break;     
                                           case 6:  SerialBT.print("-JUN-"); break;   
                                           case 7:  SerialBT.print("-JUL-"); break;    
                                           case 8:  SerialBT.print("-AUG-"); break; 
                                           case 9:  SerialBT.print("-SEP-"); break;  
                                           case 10: SerialBT.print("-OCT-"); break;  
                                           case 11: SerialBT.print("-NOV-"); break; 
                                           case 12: SerialBT.print("-DEC-"); break; }// end of SWITCH_CASE operator
                                                      
                          SerialBT.print(year, DEC); SerialBT.print(",");

                          if (hour>9) {SerialBT.print(hour, DEC); SerialBT.print(":");}
                           else {SerialBT.print("0"); SerialBT.print(hour, DEC); SerialBT.print(":");}
                            if (minute>9) {SerialBT.print(minute, DEC); SerialBT.print(":");}
                             else {SerialBT.print("0"); SerialBT.print(minute, DEC); SerialBT.print(":");}
                              if (second>9) {SerialBT.print(second, DEC);}
                               else {SerialBT.print("0"); SerialBT.print(second, DEC);} 
                          SerialBT.print(",");

                          for(int i=0;i<numberOfDevices; i++) { if (sign_mas[i] == '-') SerialBT.print("-");
                                                                                        SerialBT.print(temp_mas[i],1);
                                                                                        SerialBT.print(",");             }

                          if (numberOfDevices == 0) { SerialBT.print(" NOT FOUND CONNECTED SENSORS"); }   
                                                                                         
                          SerialBT.println(); 
                          break;                      
                } // end of switch-case operator
               
