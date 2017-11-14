#define _HALL1    15	// M2 (IO1:digital)
#define _HALL2    17	// M4 (IO1:digital)
#define _HALL3    19	// M6 (IO1:digital)

int val,tmp;

void setup() {
  pinMode(_HALL1, INPUT);
  pinMode(_HALL2, INPUT);
  pinMode(_HALL3, INPUT);

  Serial.begin(115200);
}

void loop() {
  val = 8;

  if (digitalRead(_HALL1) == LOW) val += 4;
  if (digitalRead(_HALL2) == LOW) val += 2;
  if (digitalRead(_HALL3) == LOW) val += 1;

  Serial.print(val, BIN);
  Serial.print(" ");

  Serial.println();
  delay(100);
}

