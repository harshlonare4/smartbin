#define BLYNK_TEMPLATE_ID "TMPL34kBqyG0i"
#define BLYNK_TEMPLATE_NAME "Smart Bin"
#define BLYNK_AUTH_TOKEN "KoSYQyq0fsf6cbB9JpKWebaL9_pys5yx"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <LiquidCrystal_I2C.h>

#include <Servo.h>
#include <LiquidCrystal_I2C.h>

// Enter your Auth token
char auth[] = BLYNK_AUTH_TOKEN;

//Enter your WIFI SSID and password
char ssid[] = "Project";
char pass[] = "123456789";
// char ssid[] = "ElectroEra";
// char pass[] = "mywifipassword";
//#define servoPin 8
#define Buzzer D0
#define servoPin1 D7
#define isGarbage D4
#define isWet D3
#define trigPin D6   //Define the Trigger pin of the Ultrasonic
#define echoPin1 D5  //Define the Echo pin of the Ultrasonic
#define echoPin2 D8
Servo S1;  // create servo object to control a servo
int ReadyPos = 90;
int DryPos = 0;
int WetPos = 180;
int angle = 90;
char incoming_char = 0;

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

int ultrasonic(int a) {
  int duration, distance;
  digitalWrite(trigPin, HIGH);  //Write High for 1ms
  delayMicroseconds(1000);      // 1  mili sec
  digitalWrite(trigPin, LOW);

  switch (a) {
    case 1:
      duration = pulseIn(echoPin1, HIGH);  //Time pulse in
      break;
    case 2:
      duration = pulseIn(echoPin2, HIGH);  //Time pulse in
      break;
  }

  distance = ((duration / 2) / 29.1);  //Convert time to Cm
  return distance;
}
bool B1Alert = true;
bool B2Alert = true;
void setup() {
  // put your setup code here, to run once:
 
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Connecting...");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
   pinMode(trigPin, OUTPUT);  //Setup Trigger and Echo
  pinMode(echoPin1, INPUT);
  pinMode(echoPin2, INPUT);
  pinMode(isGarbage, INPUT);
  pinMode(isWet, INPUT);
  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, LOW);
  S1.attach(servoPin1);
  S1.write(angle);


  
  Blynk.begin(auth, ssid, pass);
  lcd.print("Connected..");
  
  delay(1000);

  lcd.clear();
  lcd.print("DRY WET _______");

  //012345678912345
}
int binSize = 30;
void playBuzzer() {
  tone(Buzzer, 1000);
  delay(1000);
  noTone(Buzzer);
}

void loop() {
  Blynk.run();
  if (Serial.available()) {
    char ch = Serial.read();
    Serial.print(ch);
    if (ch == 'D' || ch == 'W')
      ServoControl(ch);
  }
  // put your main code here, to run repeatedly:
  int d1 = ultrasonic(1);
  delay(50);
  int d2 = ultrasonic(2);
  delay(50);
  Serial.println("D1 : " + String(d1) + " D2 : " + String(d2));
  d1 = 100 - (d1 * 100) / binSize;
  d2 = 100 - (d2 * 100) / binSize;
  if (d1 < 0)
    d1 = 0;
  if (d2 < 0)
    d2 = 0;
  if (d1 > 100)
    d1 = 100;
  if (d2 > 100)
    d2 = 100;
  Serial.println("B1=" + String(d1) + "% B2=" + String(d2) + "%");
  lcd.setCursor(0, 1);
  lcd.print("    ");
  lcd.setCursor(0, 1);
  lcd.print(d1);

  lcd.setCursor(4, 1);
  lcd.print("    ");
  lcd.setCursor(4, 1);
  lcd.print(d2);
  Blynk.virtualWrite(V2, d1);  //DRY
  Blynk.virtualWrite(V3, d2);

  if (d1 > 85) {
    if (B1Alert) {
      Blynk.logEvent("alert", "Dry Bin is about to filled");
      B1Alert = false;
    }
  } else {
    B1Alert = true;
  }

  if (d2 > 85) {
    if (B2Alert) {
      Blynk.logEvent("alert", "Wet Bin is about to filled");
      B2Alert = false;
    }
  } else {
    B2Alert = true;
  }


  delay(300);
  if (digitalRead(isGarbage) == LOW) {
    playBuzzer();
    Serial.print("Garbage - ");
    delay(2000);
    lcd.setCursor(9, 0);
    lcd.print("Garbage");
    if (digitalRead(isWet) == LOW) {
      Blynk.virtualWrite(V1, 0);
      lcd.setCursor(10, 1);
      lcd.print("WET");
      Serial.println("WET ");
      ServoControl('W');
    } else {
      lcd.setCursor(10, 1);
      lcd.print("DRY");
      Serial.println("Dry ");
      Blynk.virtualWrite(V1, 1);
      ServoControl('D');
    }

    lcd.setCursor(9, 0);
    lcd.print("_______");
    lcd.setCursor(10, 1);
    lcd.print("   ");
    digitalWrite(Buzzer, LOW);
  }
}

void ServoControl(char ch) {
  if (ch == 'D') {
    Serial.print("DRY>Tilting>");
    for (int i = ReadyPos; i <= WetPos; i = i + 10) {
      S1.write(i);
      delay(50);
    }
    delay(2000);
    Serial.print("BackToNormal");
    for (int i = WetPos; i >= ReadyPos; i = i - 10) {
      S1.write(i);
      delay(50);
    }
    Serial.print("Ready");
    delay(1000);
  } else if (ch == 'W') {
    Serial.print("WET>Tilting>");
    for (int i = ReadyPos; i >= DryPos; i = i - 10) {
      S1.write(i);
      delay(50);
    }

    delay(2000);
    Serial.print("BackToNormal");
    for (int i = DryPos; i <= ReadyPos; i = i + 10) {
      S1.write(i);
      delay(50);
    }
    Serial.print("Ready");
    delay(1000);
  }
}
