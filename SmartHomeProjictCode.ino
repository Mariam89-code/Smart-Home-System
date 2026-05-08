#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// تعريف الشاشة
LiquidCrystal_I2C lcd(0x27, 16, 2);

// --- تعريف المنافذ ---
const int trigPin = 2;
const int echoN = 4, echoE = 5, echoS = 6, echoW = 7;

const int radarBuzzer = 8;
const int homeBuzzer = 13;

const int ledWhite = 3;  
const int ledRed = 11;   
const int homeLed1 = 9;  
const int homeLed2 = 10; 

const int flamePin = 12;
const int rainPin = A0;
const int gasPin = A1;
const int ldrPin = A2; 

// --- متغيرات الوقت والإنذار ---
unsigned long dangerStartTime = 0;
unsigned long rainStartTime = 0;

bool dangerActive = false;
bool rainActive = false;

int lastState = -1; 

void setup() {
  Serial.begin(9600);
  
  // تهيئة الشاشة
  lcd.init();
  lcd.backlight();
  
  // إعداد اتجاهات المنافذ
  pinMode(trigPin, OUTPUT);
  pinMode(echoN, INPUT); pinMode(echoE, INPUT); 
  pinMode(echoS, INPUT); pinMode(echoW, INPUT);
  
  pinMode(radarBuzzer, OUTPUT); pinMode(homeBuzzer, OUTPUT);
  pinMode(ledWhite, OUTPUT); pinMode(ledRed, OUTPUT);
  pinMode(homeLed1, OUTPUT); pinMode(homeLed2, OUTPUT);
  
  pinMode(flamePin, INPUT);
  pinMode(ldrPin, INPUT); 
}

// دالة قراءة مسافة الحساسات وتحديد المدى الأقصى بـ 20 سانتي
int readDistance(int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(5);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 25000); 
  if (duration == 0) return 40;
  int dist = duration * 0.034 / 2;
  
  // تعديل مريم: إذا كانت المسافة المكتشفة أكبر من 20 سم، نعتبرها خارج نطاق الرصد (نعيد القيمة 40 كمسافة آمنة)
  if (dist > 20 || dist <= 0) {
    return 40; 
  }
  return dist; // يعيد المسافة الحقيقية فقط لو كانت 20 سم أو أقل
}

void loop() {
  // قراءة حساسات المسافة الأربعة
  int dN = readDistance(echoN); delay(20);
  int dE = readDistance(echoE); delay(20);
  int dS = readDistance(echoS); delay(20);
  int dW = readDistance(echoW); delay(20);

  // إرسال المسافات لبرنامج الرادار
  Serial.print(dN); Serial.print(","); Serial.print(dE); Serial.print(",");
  Serial.print(dS); Serial.print(","); Serial.println(dW);

  // قراءة باقي الحساسات
  int gasValue = analogRead(gasPin);
  int rainValue = analogRead(rainPin);
  bool isFire = (digitalRead(flamePin) == LOW); 
  
  // الاقتحام يحدث فقط لو اقترب جسم أقل من 20 سم
  bool isIntrusion = (dN < 20 || dE < 20 || dS < 20 || dW < 20);
  
  bool isDark = (digitalRead(ldrPin) == HIGH); 
  bool isRaining = (rainValue < 800); 

  // تحديد نوع الخطر الحقيقي
  bool isDanger = (isIntrusion || isFire || gasValue > 500);

  // --- 1. منطق إنذار الخطر (20 ثانية) ---
  if (isDanger) {
    if (!dangerActive) {
      dangerStartTime = millis();
      dangerActive = true;
    }
    if (millis() - dangerStartTime < 20000) {
      tone(homeBuzzer, 1500);
      tone(radarBuzzer, 1000);
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledWhite, LOW);
    } else {
      noTone(homeBuzzer);
      noTone(radarBuzzer);
      digitalWrite(ledRed, LOW);
      digitalWrite(ledWhite, HIGH);
    }
  } else {
    dangerActive = false;
    if (!isRaining) {
      noTone(homeBuzzer);
      noTone(radarBuzzer);
    }
  }

  // --- 2. منطق إنذار المطر (10 ثوانٍ) ---
  if (isRaining && !isDanger) { 
    if (!rainActive) {
      rainStartTime = millis();
      rainActive = true;
    }
    if (millis() - rainStartTime < 10000) {
      tone(homeBuzzer, 800); 
    } else {
      noTone(homeBuzzer);
    }
  } else {
    rainActive = false;
  }

  // --- 3. منطق الإضاءة والأنوار الذكية ---
  if (isDark) {
    analogWrite(homeLed1, 150); 
    analogWrite(homeLed2, 150);
  } else {
    digitalWrite(homeLed1, LOW); 
    digitalWrite(homeLed2, LOW);
  }

  if (!isDanger) {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledWhite, HIGH); 
  }

  // --- 4. تحديث شاشة الـ LCD ---
  int currentState = 0; 
  
  if (isFire) currentState = 1; 
  else if (gasValue > 500) currentState = 2; 
  else if (isIntrusion) currentState = 3; 
  else if (isRaining) currentState = 4;

  if (currentState != lastState) {
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (currentState == 1) {
      lcd.print("!! FIRE !!");
    } 
    else if (currentState == 2) {
      lcd.print("!! GAS LEAK !!");
    } 
    else if (currentState == 3) {
      lcd.print("INTRUDER!");
    } 
    else if (currentState == 4) {
      lcd.print("RAINING");
    } 
    else {
      lcd.print("HOME IS SAFE");
      lcd.setCursor(0, 1);
      lcd.print("SYSTEM READY");
    }
    lastState = currentState;
  }
  
  delay(10); 
}