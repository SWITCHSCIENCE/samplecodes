#include "espr32conta.h"

//------------------------------------------
// Main logics
//------------------------------------------

void setup()
{
  Serial.begin(115200);

  // DIO PIN
  pinMode(DIO1, OUTPUT);
  pinMode(DIO2, OUTPUT);
  digitalWrite(DIO1, LOW);
  digitalWrite(DIO2, HIGH);
}

void loop()
{
  //Serial.println("DIO1=LOW, DIO2=HIGH");
  digitalWrite(DIO1, LOW);
  digitalWrite(DIO2, HIGH);
  delay(1);
  //Serial.println("DIO1=HIGH, DIO2=LOW");
  digitalWrite(DIO1, HIGH);
  digitalWrite(DIO2, LOW);
  delay(1);
}
