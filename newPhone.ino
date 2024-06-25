#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal.h>  
#include <SoftwareSerial.h>

SoftwareSerial sim(10, 11);  // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  // RS, E, D4, D5, D6, D7

const byte ROWS = 4;  // Four rows
const byte COLS = 4;  // Four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};  
byte colPins[COLS] = {5, 4, 3, 2}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int _timeout;
String _buffer;
String enteredNumber = "";
bool enteringNumber = false;

void setup() {
  Serial.begin(9600);
  sim.begin(9600);
  
  // LCD initialization
  lcd.begin(16, 2); 
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A/B/C:");
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    handleKeyPress(key);
  }
  if (sim.available() > 0) {
    char c = sim.read();
    Serial.write(c);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Incoming SMS:");
    lcd.setCursor(0, 1);
    lcd.print(c);
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Press A/B/C:");
  }
}

void handleKeyPress(char key) {
  if (enteringNumber) {
    if (key == '#') {
      enteringNumber = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Number Set:");
      lcd.setCursor(0, 1);
      lcd.print(enteredNumber);
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Press A/B/C:");
    } else if (key == '*') {
      if (enteredNumber.length() > 0) {
        enteredNumber.remove(enteredNumber.length() - 1);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Enter Number:");
        lcd.setCursor(0, 1);
        lcd.print(enteredNumber);
      }
    } else {
      enteredNumber += key;
      lcd.setCursor(0, 1);
      lcd.print(enteredNumber);
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Key Pressed:");
    lcd.setCursor(0, 1);
    lcd.print(key);

    switch (key) {
      case 'A':
        SendMessage();
        break;
      case 'B':
        RecieveMessage();
        break;
      case 'C':
        if (enteredNumber.length() > 0) {
          callNumber();
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Enter Number:");
          enteredNumber = "";
          enteringNumber = true;
        }
        break;
    }
  }
}

void SendMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending SMS...");
  
  sim.println("AT+CMGF=1");    
  delay(200);
  sim.println("AT+CMGS=\"" + enteredNumber + "\"\r"); 
  delay(200);
  String SMS = "Hello, how are you?";
  sim.println(SMS);
  delay(100);
  sim.println((char)26);
  delay(200);
  _buffer = _readSerial();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SMS Sent");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A/B/C:");
}

void RecieveMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reading SMS...");

  Serial.println("SIM800L Read an SMS");
  sim.println("AT+CMGF=1");
  delay(200);
  sim.println("AT+CNMI=1,2,0,0,0"); 
  delay(200);
}

void callNumber() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calling...");
  
  sim.print("ATD");
  sim.print(enteredNumber);
  sim.println(";");
  delay(100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calling ");
  lcd.print(enteredNumber);
  delay(2000); 
  _buffer = _readSerial();

  // End the call
  sim.println("ATH");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Call ended");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press A/B/C:");
}

String _readSerial() {
  _timeout = 0;
  while (!sim.available() && _timeout < 12000) {
    delay(13);
    _timeout++
  }
  if (sim.available()) {
    return sim.readString();
  }
  return "";
}
