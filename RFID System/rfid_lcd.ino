#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>

#define SS_PIN 10
#define RST_PIN 9

#define LED_R 6
#define LED_G 7
#define BUZZER 8

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal lcd(A0, A1, A2, A3, A4, A5);

// AUTORYZOWANA KARTA (ZMIEN NA SWOJA)
byte authorizedUID[4] = {0xD0, 0x4D, 0xF3, 0x32};

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.begin(16, 2);
  lcd.print("System RFID");
  lcd.setCursor(0, 1);
  lcd.print("Przyloz karte");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("UID:");

  bool authorized = true;

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) lcd.print("0");
    lcd.print(rfid.uid.uidByte[i], HEX);

    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      authorized = false;
    }
  }

  if (authorized) {
    lcd.setCursor(0, 1);
    lcd.print("AUTORYZOWANA");

    digitalWrite(LED_G, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(300);
    digitalWrite(BUZZER, LOW);
    delay(200);
    digitalWrite(LED_G, LOW);
  } else {
    lcd.setCursor(0, 1);
    lcd.print("BRAK DOSTEPU");

    digitalWrite(LED_R, HIGH);
    digitalWrite(BUZZER, HIGH);
    delay(800);
    digitalWrite(BUZZER, LOW);
    digitalWrite(LED_R, LOW);
  }

  delay(1500);
  lcd.clear();
  lcd.print("Przyloz karte");

  rfid.PICC_HaltA();
}
