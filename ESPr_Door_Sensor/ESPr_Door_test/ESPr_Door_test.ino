int LED = 4;
int reed_sw = 5;

void setup() {
     pinMode(LED,OUTPUT);
     delay(10);
     if(digitalRead(reed_sw) == 0){
        digitalWrite(LED,HIGH);
        delay(100);
        digitalWrite(LED,LOW); 
     }
     else{
        for(int i = 0; i < 3; i++){
          digitalWrite(LED,HIGH);
          delay(100);
          digitalWrite(LED,LOW);
          delay(100);
        }
     }
    ESP.deepSleep(0,WAKE_RF_DEFAULT);
    delay(1000);
}

void loop() {
  

}
