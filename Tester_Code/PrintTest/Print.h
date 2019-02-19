static double factor = 4.88759;
static int32_t precision = 0;

void Print(int32_t in){
	Serial.print(in * factor, precision);
	Serial.print("\t");
}

void Println(int32_t in){
	Serial.println(in * factor, precision);
}