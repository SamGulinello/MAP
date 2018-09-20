int emg = A1;
int emgArray[25];
int readIndex;
int emgValue;
int rmsValue;
long total;

void setup() {
  // put your setup code here, to run once:
  pinMode(emg,INPUT);
  for(int thisReading = 0; thisReading < 25; thisReading++){
    emgArray[thisReading]=0;
  }
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  emgValue = analogRead(emg)-334;
  total = total - emgArray[readIndex];
  emgArray[readIndex] = sq(emgValue);
  total = total + emgArray[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= 25) {
    readIndex = 0;
  }
  rmsValue = (sqrt(total/25));
  Serial.print(emgValue);
  Serial.print("\t");
  Serial.println(rmsValue);
  delay(25);
}
