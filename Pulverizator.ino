#include "DHT.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define DHTPIN 2                // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11           // Temperature sensor type

LiquidCrystal_I2C lcd(0x3F, 16, 2);

const int startPin =  3;        // Digital pin connected to the START button
const int stopPin =  4;         // Digital pin connected to the STOP button
const int pumpPin = 5;          // Digital pin connected to the PUMP
const int ledPinGreen = 8;      // Digital pin connected to the GREEN LED pin
const int ledPinRed = 9;        // Digital pin connected to the RED LED pin
const int waterLevelPin = A2;   // Digital pin connected to the WATER LEVEL sensor

// variables will change:
int waterLevelValue = 0;        // Variable for reading the WATER LEVEL values
int startButtonState = 0;       // Variable for reading the START button status
int stopButtonState = 0;        // Variable for reading the STOP button status


int startCounter = 0;           // Auxiliar variable for timer without use of delay
int stopCrash = 0;           // Auxiliar variable to prevent crash of arduino
unsigned long previousTime = 0; // Variables used for counting the time difference between
unsigned long currentTime = 0;  // the start and end of MANUAL PUMP ON


DHT dht(DHTPIN, DHTTYPE);

void setup() {

  Serial.begin(9600);

  Serial.println(F("DHT11 test!"));
  dht.begin();

  // PUMP pin as output:
  pinMode(pumpPin, OUTPUT);
  // LED pins as output:
  pinMode(ledPinGreen , OUTPUT);
  pinMode(ledPinRed , OUTPUT);
  // START button pin as input:
  pinMode(startPin, INPUT);
  // STOP button pin as input:
  pinMode(stopPin, INPUT);

  lcd.begin();
	lcd.backlight();
}

void loop() {
  
  char buffer0[16]; //First LCD row
  char buffer1[16]; //Second LCD row

  // Read temperature as Celsius degrees
  float t = dht.readTemperature();

  // Check if any temperature reads have failed
  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  
  // Water level values over 350 are considered enough for the PUMP to run
  waterLevelValue = analogRead(waterLevelPin);
  Serial.println(waterLevelValue);

  if (waterLevelValue > 400){

    stopCrash = 0;

    // If temperature is equal to or exceeds 30 degrees start pump
    if (int(t) >= 30){
      digitalWrite(pumpPin, HIGH); 
      digitalWrite(ledPinGreen, HIGH);

      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.setCursor(6,0);
      lcd.print(t,1);
      lcd.setCursor(10,0);
      lcd.print("C");
      lcd.setCursor(0,1);
      lcd.print("Pump: ON        ");
    }else if (int(t) <= 30 && startButtonState == LOW){
      //If temperature is below 30 degrees stop pump
      digitalWrite(pumpPin, LOW);
      digitalWrite(ledPinGreen, LOW); 

      lcd.setCursor(0,0);
      lcd.print("Temp: ");
      lcd.setCursor(6,0);
      lcd.print(t,1);
      lcd.setCursor(10,0);
      lcd.print("C");
      lcd.setCursor(0,1);
      lcd.print("Pump: OFF       ");
    }

    //Checks state of buttons (pressed or not)
    if (startCounter == 0) {
      startButtonState = digitalRead(startPin);
    }
    stopButtonState = digitalRead(stopPin);
    
    // Gets time on every iteration
    currentTime = millis();
    
    Serial.println(currentTime);

    // Check if the start pushbutton is pressed
    // If it is then START buttonState is HIGH
    if (startButtonState == HIGH && int(t) < 30) {
      //PUMP ON manually:

      if (startCounter == 0){
        previousTime = currentTime;
      }
      startCounter = 1;

      //Power on PUMP and GREEN LED
      digitalWrite(pumpPin, HIGH);
      digitalWrite(ledPinGreen, HIGH);

      // If 5 seconds have passed by turn off PUMP and LED
      double x = currentTime - previousTime;
      if (x >= 5000) {
        startCounter = 0;
        digitalWrite(pumpPin, LOW);
        digitalWrite(ledPinGreen, LOW);
      }
      int y = (5 - int(x)/1000);
      if (y == 0){
        sprintf (buffer1, "Pump: OFF       ");
      }
      else{
        sprintf (buffer1, "Pump: ON  for %d  ", (5 - int(x)/1000) );
      }
      lcd.setCursor(0,1);
      lcd.print(buffer1);
      Serial.println(buffer1);
      
    }
    // Check if the stop pushbutton is pressed. If it is then STOP buttonState is HIGH
    if (stopButtonState == HIGH && (digitalRead(pumpPin) == HIGH)) {
      // Emergency brake PUMP OFF:
      digitalWrite(pumpPin, LOW);
      digitalWrite(ledPinGreen, LOW);
      digitalWrite(ledPinRed, HIGH);

      lcd.setCursor(0,1);
      for (int i = 5; i > 0; i--){   
        sprintf (buffer1, "Pump: OFF for %d  ", i);
        lcd.print(buffer1);
        Serial.println(buffer1);
        delay(1000);
        lcd.setCursor(0,1);
      }
      
      startCounter = 0;

      lcd.print("Pump: OFF       ");
      digitalWrite(ledPinRed, LOW);
    }
  }else if (waterLevelValue <= 400 && stopCrash == 0){

    // If there is no water in tank the sistem does not work
    Serial.println("no water");
    lcd.setCursor(0,0);
    lcd.print("ERROR!          ");
    lcd.setCursor(0,1);
    lcd.print("Water tank empty");
    //  Powers of PUMP 
    digitalWrite(pumpPin, LOW);
    digitalWrite(ledPinGreen, LOW);
    Serial.println("no water1");
    stopCrash = 1;
  }
}