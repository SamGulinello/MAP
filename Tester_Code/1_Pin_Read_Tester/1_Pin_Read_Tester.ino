/* All purpose tester for quick analog or digital read and plotter */

int pin = A1;
int value;

void setup() {
  // put your setup code here, to run once:
  pinMode(pin, INPUT);
  Serial.begin(9600);
}

void loop() {
  emgValue = analogRead(pin);
  Serial.println(value);
  delay(25);

}
