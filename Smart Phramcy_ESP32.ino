#include <DS1302.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GenericDevice.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <ESP32Servo.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>


//-----------------------------------------------------OLED PINS -------------------------------------------------------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define EEPROM_SIZE 10

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//--------------------------------------------------WIFI DIFINITION-----------------------------------------------------------

const char* ssid = "Maison";
const char* password = "1234567890";
WebServer server(80);

//------------------------------------------------------IR PINS---------------------------------------------------------------

const int IR1 = 14;
const int IR2 = 12;

//-----------------------------------------------------LED's PINS-------------------------------------------------------------

const int led1 = 23;
const int led2 = 32;
const int led3 = 33;
const int led4 = 25;

//-----------------------------------------------------BUZZER PIN-------------------------------------------------------------

const int buzzerr = 4;

//--------------------------------------------------VOICE MODULE PINS---------------------------------------------------------

const int play1 = 13;
const int play2 = 15;

//------------------------------------------------------SERVO PINS------------------------------------------------------------

int servoPin1 = 26;
int servoPin2 = 27;
Servo myservo1;
Servo myservo2;

//------------------------------------------------------VARIABLES-------------------------------------------------------------

int hour = 0;
int minute = 0;
int second = 0;
int x = 1, y = 1, i = 1, j = 1;
int testminute;
int R = 0;
int addr = 0;
int val = 0;

//----------------------------------------------------EEPROM VARIABLES--------------------------------------------------------

String inputString = "";
String inputString1 = "";
String inputString2 = "";
String inputString3 = "";
bool stringComplete = false;
byte hourS, minuteS, pillS, drawS = 0;

//-------------------------------------------------------CLOCK PINS-----------------------------------------------------------

const int kCePin = 5;     // Chip Enable
const int kIoPin = 18;    // Input/Output
const int kSclkPin = 19;  // Serial Clock

DS1302 rtc(kCePin, kIoPin, kSclkPin);

String dayAsString(const Time::Day day) {
  switch (day) {
    case Time::kSunday: return "Sunday";
    case Time::kMonday: return "Monday";
    case Time::kTuesday: return "Tuesday";
    case Time::kWednesday: return "Wednesday";
    case Time::kThursday: return "Thursday";
    case Time::kFriday: return "Friday";
    case Time::kSaturday: return "Saturday";
  }
  return "(unknown day)";
}

//------------------------------------------------------OLED DISPLAY----------------------------------------------------------

void printTime() {

  Time t = rtc.time();

  // Name the day of the week.

  const String day = dayAsString(t.day);

  // Format the time and date and insert into the temporary buffer.

  char buf[50];
  snprintf(buf, sizeof(buf), "%s %04d-%02d-%02d %02d:%02d:%02d",
           day.c_str(),
           t.yr, t.mon, t.date,
           t.hr, t.min, t.sec);
  hour = t.hr;
  minute = t.min;
  second = t.sec;

  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(second);
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setCursor(45, 0);
  display.println("TIME");
  display.display();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 14);
  display.println("  :  :");
  display.setCursor(10, 14);
  display.println(hour);
  display.setCursor(45, 14);
  display.println(minute);
  display.setCursor(90, 14);
  display.println(second);
  display.display();
}

void welcome(){
  
  // Show "Welcome!!" for 5 seconds
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Welcome!!");
  display.display();
  delay(5000); // Wait for 5 seconds

  display.clearDisplay(); // Clear "Welcome!!"
}

//-------------------------------------------------------VOID SETUP-----------------------------------------------------------

void setup() {

  //-----------------------------------------------------WIFI SETUP-----------------------------------------------------------

  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to Wi-Fi");
  Serial.println(WiFi.localIP());

  //----------------------------------------------------START SERVER----------------------------------------------------------

  server.begin();

  server.on("/send-data", handleData);  //sending data

  server.on("/set-time", handleSetTime);  //set time

  //---------------------------------------------http://<ESP32_IP_ADDRESS>/clear----------------=-----------------------------

  server.on("/clear", HTTP_GET, []() {  //clear memory

    clearEEPROM();
    server.send(200, "text/plain", "EEPROM Cleared");

  });

  Serial.println("Server started");

  //----------------------------------------------SERIAL,OLED,CLOCK-----------------------------------------------------------

  Serial.begin(9600);
  EEPROM.begin(EEPROM_SIZE);

  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  welcome();

  rtc.writeProtect(false);  //Time t(2025, 3, 21, 1, 25, 30, Time::kFriday);
  rtc.halt(false);          //rtc.time(t);

  //-----------------------------------------------------SERVO----------------------------------------------------------------

  myservo1.setPeriodHertz(50);
  myservo1.attach(servoPin1);
  myservo2.setPeriodHertz(50);
  myservo2.attach(servoPin2);

  myservo1.write(0);
  myservo2.write(180);

  //-------------------------------------------------IR & EEPROM--------------------------------------------------------------

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);

  inputString.reserve(200);

  //--------------------------------------------------LED's & BUZZER----------------------------------------------------------

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(buzzerr, OUTPUT);

  for (int p = 0; p < 5; p++) {

    digitalWrite(led1, HIGH);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    digitalWrite(led4, HIGH);
    digitalWrite(buzzerr, HIGH);

    delay(300);

    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(buzzerr, LOW);

    delay(300);
  }

  //---------------------------------------------------VOICE MODULE-----------------------------------------------------------

  pinMode(play1, OUTPUT);
  pinMode(play2, OUTPUT);

  digitalWrite(play1, LOW);
  digitalWrite(play2, LOW);
}

//-----------------------------------------------------VOID LOOP--------------------------------------------------------------

void loop() {

  //-------------------------------------------------PROCESS INCOMING DATA----------------------------------------------------

  server.handleClient();

  Serial.println(WiFi.localIP());

  //-----------------------------------------------------CLOCK----------------------------------------------------------------

  printTime();
  delay(1000);

  //----------------------------------------------EEPROM READ VALUE DRAWER 1--------------------------------------------------

  if (x == 1) {
    hourS = EEPROM.read(0);
    minuteS = EEPROM.read(1);
    drawS = EEPROM.read(2);
    pillS = EEPROM.read(3);

    //-------------------------------------------------COMPARE THE TIME-------------------------------------------------------

    if ((hourS == hour) && (minuteS == minute)) {
      testminute = minuteS;

      //-----------------------------------------------TURN ON BUZZER---------------------------------------------------------

      digitalWrite(buzzerr, HIGH);
      delay(200);  //Active buzzer
      digitalWrite(buzzerr, LOW);

      //------------------------------------------------SERVO DRAWER 1--------------------------------------------------------

      if (drawS == 1) {
        if (i == 1) {
          myservo1.write(90);
          delay(1000);  //moving sevo one time
          myservo1.write(0);
          i = 2;
        }

        //-------------------------------------------LED & VOICE MODULE FOR PILLS---------------------------------------------

        if (pillS == 1) {
          digitalWrite(led1, HIGH);
          digitalWrite(play1, HIGH);  //turn on led and voice replay
          delay(1000);
          digitalWrite(play1, LOW);
        }
        if (pillS == 2) {
          digitalWrite(led1, HIGH);
          digitalWrite(led2, HIGH);
          digitalWrite(play2, HIGH);  //turn on led and voice replay
          delay(1000);
          digitalWrite(play2, LOW);
        }
      }
    }
  }

  //---------------------------------------------EEPROM READ VALUE DRAWER 2---------------------------------------------------

  if (y == 1) {
    hourS = EEPROM.read(4);
    minuteS = EEPROM.read(5);
    drawS = EEPROM.read(6);
    pillS = EEPROM.read(7);

    //-------------------------------------------------COMPARE THE TIME-------------------------------------------------------

    if ((hourS == hour) && (minuteS == minute)) {
      testminute = minuteS;

      //------------------------------------------------TURN ON BUZZER--------------------------------------------------------

      digitalWrite(buzzerr, HIGH);
      delay(200);
      digitalWrite(buzzerr, LOW);

      //-------------------------------------------------SERVO DRAWER 2-------------------------------------------------------

      if (drawS == 2) {
        if (j == 1) {
          myservo2.write(90);
          delay(1000);
          myservo2.write(180);
          j = 2;
        }

        //-----------------------------------------LED & VOICE MODULE FOR PILLS-----------------------------------------------

        if (pillS == 1) {
          digitalWrite(led3, HIGH);
          digitalWrite(play1, HIGH);
          delay(1000);
          digitalWrite(play1, LOW);
        }
        if (pillS == 2) {
          digitalWrite(led3, HIGH);
          digitalWrite(led4, HIGH);
          digitalWrite(play2, HIGH);
          delay(1000);
          digitalWrite(play2, LOW);
        }
      }
    }
  }

  //--------------------------------------------------RSIGN VULUES------------------------------------------------------------

  if (minute != testminute) {
    x = 1;
    y = 1;
    i = 1;
    j = 1;
    testminute = 0;
  }

  //------------------------------------------------------IR------------------------------------------------------------------

  if (digitalRead(IR1) == HIGH) {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
    digitalWrite(buzzerr, LOW);
    digitalWrite(play1, LOW);
    x = 2;
    i = 1;  //change the value to stop iteration for draw 1
  }

  if (digitalRead(IR2) == HIGH) {
    digitalWrite(led3, LOW);
    digitalWrite(led4, LOW);
    digitalWrite(buzzerr, LOW);
    digitalWrite(play2, LOW);
    y = 2;
    j = 1;  //change the value to stop iteration for draw 2
  }
}

//--------------------------------------------------HANDLE DATA---------------------------------------------------------------
//server.hasArg("hour") && server.hasArg("minute")&&server.hasArg("pills")
//-------------------------http://<ESP32_IP_ADDRESS>/send-data?hour=14&minute=30&drawer=1&pills=2-----------------------------

void handleData() {

  //-----------------------------------CHECK IF THE HTTP REQUEST CONTAIN ALL PARAMETERS---------------------------------------

  if (server.method() == HTTP_POST) {

    //-----------------------------------READ VALUE FROM URL AND PARS THEM----------------------------------------------------

    int hourS = server.arg("hour").toInt();
    int minuteS = server.arg("minute").toInt();
    int drawS = server.arg("drawer").toInt();
    int pillS = server.arg("pills").toInt();

    //----------------------------------------------EEPROM SAVE FOR DRAWER 1--------------------------------------------------

    if (drawS == 1) {
      EEPROM.write(0, hourS);
      EEPROM.write(1, minuteS);
      EEPROM.write(2, drawS);
      EEPROM.write(3, pillS);
      EEPROM.commit();

      ledindicator();

      // Print the received values
      
      Serial.print("\nReceived values: ");
      Serial.print("Hour: ");
      Serial.print(hourS);
      Serial.print(", Minute: ");
      Serial.print(minuteS);
      Serial.print(", Drawer: ");
      Serial.print(drawS);
      Serial.print(", Pills: ");
      Serial.println(pillS);
    }

    //----------------------------------------------EEPROM SAVE FOR DRAWER 2--------------------------------------------------

    else if (drawS == 2) {
      EEPROM.write(4, hourS);
      EEPROM.write(5, minuteS);
      EEPROM.write(6, drawS);
      EEPROM.write(7, pillS);
      EEPROM.commit();

      ledindicator();  //blink LED

      // Print the received values

      Serial.print("\nReceived values: ");
      Serial.print("Hour: ");
      Serial.print(hourS);
      Serial.print(", Minute: ");
      Serial.print(minuteS);
      Serial.print(", Drawer: ");
      Serial.print(drawS);
      Serial.print(", Pills: ");
      Serial.println(pillS);

    } else {
      server.send(400, "text/plain", "Invalid drawer number");
      return;
    }

    //-----------------------------------------------------SEND SUCCESS-------------------------------------------------------

    String response = "Data saved -> Hour: " + String(hourS) + ", Minute: " + String(minuteS) + ", Drawer: "
                      + String(drawS) + ", Pills: " + String(pillS);

    server.send(200, "text/plain", response);

  } else {
    server.send(400, "text/plain", "Missing one or more parameters: hour, minute, drawer, pills");  //missing error
  }
}

//------------------------------------------------HANDLE SET TIME-------------------------------------------------------------

//----------------------------------http://<ESP32_IP_ADDRESS>/set-time?hour=14&minute=45--------------------------------------

void handleSetTime() {

  if (server.hasArg("hour") && server.hasArg("minute")) {

    int setHour = server.arg("hour").toInt();
    int setMinute = server.arg("minute").toInt();

    if (setHour >= 0 && setHour < 24 && setMinute >= 0 && setMinute < 60) {
      Time t(2025, 4, 20, setHour, setMinute, 0, Time::kSunday);
      rtc.time(t);

      ledindicator();

      server.send(200, "text/plain", "Time set successfully: " + String(setHour) + ":" + String(setMinute));
      Serial.print("Time set to: ");
      Serial.print(setHour);
      Serial.print(":");
      Serial.println(setMinute);

    } else {
      server.send(400, "text/plain", "Invalid time format.");
    }

  } else {
    server.send(400, "text/plain", "Missing 'hour' or 'minute' parameter.");
  }
}

//------------------------------------------------------CLEAR MEMORY----------------------------------------------------------

void clearEEPROM() {

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);
  }

  EEPROM.commit();
  ledindicator();

  Serial.println("EEPROM memory cleared!");
}

//-----------------------------------------DISPLAY VALUES IN THE SERIAL INPUT-------------------------------------------------

void readEEPROMSerial() {

  Serial.println("------ EEPROM DATA ------");

  //---------------------------------------------------------DRAWER 1---------------------------------------------------------

  byte hour1 = EEPROM.read(0);
  byte minute1 = EEPROM.read(1);
  byte drawer1 = EEPROM.read(2);
  byte pills1 = EEPROM.read(3);

  Serial.print("Drawer 1 -> Hour: ");
  Serial.print(hour1);
  Serial.print(", Minute: ");
  Serial.print(minute1);
  Serial.print(", Drawer: ");
  Serial.print(drawer1);
  Serial.print(", Pills: ");
  Serial.println(pills1);

  //---------------------------------------------------------DRAWER 1---------------------------------------------------------

  byte hour2 = EEPROM.read(4);
  byte minute2 = EEPROM.read(5);
  byte drawer2 = EEPROM.read(6);
  byte pills2 = EEPROM.read(7);

  Serial.print("Drawer 2 -> Hour: ");
  Serial.print(hour2);
  Serial.print(", Minute: ");
  Serial.print(minute2);
  Serial.print(", Drawer: ");
  Serial.print(drawer2);
  Serial.print(", Pills: ");
  Serial.println(pills2);

  Serial.println("--------------------------");
}

//-----------------------------------------------SERIAL & LEDindicator--------------------------------------------------------

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 'r') {
      readEEPROMSerial();  // Read EEPROM when 'r' is entered
    }
  }
}

void ledindicator() {
  for (R = 0; R < 5; R++) {
    digitalWrite(buzzerr, HIGH);
    delay(100);  //buzzer setup
    digitalWrite(buzzerr, LOW);
    delay(100);
  }
}

//----------------------------------------------------------END---------------------------------------------------------------
