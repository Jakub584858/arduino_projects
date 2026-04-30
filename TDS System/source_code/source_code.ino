// Libraries
#include <LiquidCrystal.h>
#include <dht.h>

// Constant variables and pins
#define TdsSensorPin A0
#define dhtPin 2
#define ledPin 3
#define VREF 5.0      // Reference voltage of ADC
#define SCOUNT 30     // Number of samples for median filter

int analogBuffer[SCOUNT];     // Buffer for raw TDS samples
int analogBufferTemp[SCOUNT]; // Temporary buffer for sorting
int analogBufferIndex = 0, copyIndex = 0, check;

float averageVoltage = 0, tdsValue = 0, Temperature, temperatureKelvin;

// Members of classes: lcd for LiquidCrystal (display) and DHT for dht class (sensor)
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
dht DHT;

// Setting serial port, lcd display and pins
void setup() { 
  lcd.begin(16, 2);
  pinMode(TdsSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(dhtPin, INPUT);
  Serial.begin(9600);
} 

void loop() { 
  // Reading temperature from DHT11 and calculating Kelvin scale
  static unsigned long analogSampleTimepoint = millis();
  check = DHT.read11(dhtPin);
  if(check == 0) Temperature = DHT.temperature;
  temperatureKelvin = Temperature + 273.15;

  // Sampling TDS sensor every 40ms and storing in circular buffer
  if (millis() - analogSampleTimepoint > 40U) {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT) analogBufferIndex = 0;
  }

  static unsigned long printTimepoint = millis();

  // Processing and displaying data every 800ms
  if (millis() - printTimepoint > 800U) {
    printTimepoint = millis();

    // Copying buffer for filtering (to keep original data intact)
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++) analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

    // Getting median voltage and converting to real voltage value
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;

    // Temperature compensation (standard reference 25*C)
    float compensationCoefficient = 1.0 + 0.02 * (Temperature - 25.0);
    float compensationVoltage = averageVoltage / compensationCoefficient;

    // Converting voltage to TDS value (ppm) using polynomial equation
    tdsValue = (133.42 * pow(compensationVoltage, 3) - 255.86 * pow(compensationVoltage, 2) + 857.39 * compensationVoltage) * 0.5;

    // High TDS alarm - LED indicator
    if(tdsValue >= 300) digitalWrite(ledPin, HIGH);
    else digitalWrite(ledPin, LOW);

    // Refreshing LCD with TDS results and temperature in Kelvin
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TDS: ");
    lcd.print(tdsValue, 0);
    lcd.print("ppm");

    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(temperatureKelvin, 0);
    lcd.print("K");
  }
}

// Median filter function: sorts the array and returns the middle value
int getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) bTab[i] = bArray[i];

  int i, j, bTemp;

  // Simple Bubble Sort algorithm
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }

  // Returning median based on odd/even length
  if ((iFilterLen & 1) > 0) bTemp = bTab[(iFilterLen - 1) / 2];
  else bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;

  return bTemp;
}