#define BLYNK_TEMPLATE_ID "TMPL381gWNRi2"
#define BLYNK_TEMPLATE_NAME "IOT BASED power monitoring and theft detection " ssAZzNJbxXiKtsM-1rIBvfQLffDEaDk7
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;

/* BLYNK DETAILS */
char auth[] = "ssAZzNJbxXiKtsM-1rIBvfQLffDEaDk7";
char ssid[] = "realme 12x 5G ";
char pass[] = "0987654321";

/* PINS */
#define PULSE_PIN D5
#define RELAY_PIN D6
#define BUZZER D7

/* VARIABLES */
volatile unsigned long pulseCount = 0;
float energy = 0.0;
float power = 0;
float current = 0;
float voltage = 230.0;

/* INTERRUPT */
ICACHE_RAM_ATTR void pulseISR() {
  pulseCount++;
}

/* READ CURRENT (AVERAGED) */
float readCurrent() {
  long sum = 0;
  for(int i = 0; i < 50; i++) {
    sum += analogRead(A0);
    delayMicroseconds(200);
  }

  float adc = sum / 50.0;
  float v = adc * (1.0 / 1023.0);   // ESP8266 ADC = 1V
  float current = (v - 0.5) / 0.066; // Adjust offset after calibration
  return abs(current);
}

/* MAIN CALCULATION */
void calculate() {

  // Safely copy pulse count
  noInterrupts();
  unsigned long pulses = pulseCount;
  pulseCount = 0;
  interrupts();

  current = readCurrent();
  power = voltage * current;

  // 1000 pulses = 1 kWh
  energy += pulses / 1000.0;

  lcd.setCursor(0,0);
  lcd.print("P:");
  lcd.print(power,1);
  lcd.print("W   ");

  lcd.setCursor(0,1);
  lcd.print("E:");
  lcd.print(energy,3);
  lcd.print("kWh ");

  Blynk.virtualWrite(V0, voltage);
  Blynk.virtualWrite(V1, current);
  Blynk.virtualWrite(V2, power);
  Blynk.virtualWrite(V3, energy);

  if(power > 1000) {
    digitalWrite(BUZZER, HIGH);
    Blynk.virtualWrite(V5, 1);
  } else {
    digitalWrite(BUZZER, LOW);
    Blynk.virtualWrite(V5, 0);
  }
}

/* RELAY CONTROL */
BLYNK_WRITE(V4) {
  digitalWrite(RELAY_PIN, param.asInt());
}

void setup() {
  Serial.begin(9600);

  pinMode(PULSE_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(PULSE_PIN), pulseISR, FALLING);

  lcd.init();
  lcd.backlight();

  Blynk.begin(auth, ssid, pass);
  timer.setInterval(2000L, calculate);
}

void loop() {
  Blynk.run();
  timer.run();
}
