int emg = A1;
long total = 0;
int emgAvg = 0;
long avgTotal = 0;
int loopRuns = 0;
int value = 0;
int emgArray[25] = {0};
int readIndex = 0;

void emgCal(){
  avgTotal = 0;
  for(int i = 0; i < 25; i++){
    avgTotal += emgArray[i];
  }

  emgAvg = (avgTotal/25);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(loopRuns == 40){
    emgCal();
  }
  
  value = analogRead(emg);
  emgArray[readIndex] = value;

  
  loopRuns++;
  readIndex++;
  if(readIndex >= 25){
    readIndex = 0;
  }

  delay(25);
}
