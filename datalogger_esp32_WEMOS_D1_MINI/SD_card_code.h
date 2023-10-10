
  if(!SD.begin()){ Serial.println("> SD Card mount failed"); sd_card_status = 0;}
   else {Serial.println("> SD Card successfully initialized"); sd_card_status = 1;}

  if (sd_card_status == 1) { uint8_t cardType = SD.cardType();

                             Serial.print("> SD Card type: ");
               
                             if(cardType == CARD_MMC){Serial.println("MMC");}
                              else if(cardType == CARD_SD){Serial.println("SDSC" );} 
                               else if(cardType == CARD_SDHC){Serial.println("SDHC");} 
                                else {Serial.println("UNKNOWN");}

                             uint64_t cardSize = SD.cardSize() / (1024 * 1024);
                             Serial.printf("> SD Card size: %lluMB\r\n", cardSize);          } // end of IF operator

  
                             
  
