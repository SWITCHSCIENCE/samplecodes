const int pinBzz = 18;

void setup(void)
{
}

void loop(void)
{
  tone(pinBzz, 2252, 50);
  delay(1000);

  tone(pinBzz, 2000, 50);
  delay(1000);
}
