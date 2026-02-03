#include "DHT.h"

/* ---------- MQ-3 (ALKOL) ---------- */
#define MQ3_PIN A0
int ALKOL_THRESHOLD = 500;

/* ---------- DHT22 (ISI + NEM) ---------- */
#define DHTPIN 5
#define DHTTYPE DHT22
float TEMP_THRESHOLD = 30.0;
float HUM_THRESHOLD  = 60.0;
DHT dht(DHTPIN, DHTTYPE);

/* ---------- NABIZ SENSÖRÜ ---------- */
#define NABIZ_PIN A2
bool nabizAlarm = false;

/* ---------- ALARM ÇIKIŞLARI ---------- */
const int ledPins[] = {2, 3, 7, 8};
const int buzzerPin = 4;

/* ---------- ALARM DURUMLARI ---------- */
bool kameraAlarm = false;
bool alkolAlarm  = false;
bool isiAlarm    = false;
bool nemAlarm    = false;

/* ---------- PROCESSING'E GÖNDERİLECEK VERİLER ---------- */
float lastTemp   = 0.0;
float lastHum    = 0.0;
int   lastNabiz  = 0;
int   lastAlkol  = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  for (int i = 0; i < 4; i++) pinMode(ledPins[i], OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  alarmKapat();
  Serial.println("Sistem hazir");
}

void loop() {
  alkolKontrol();
  isiNemKontrol();
  nabizKontrol();
  kameraKomutKontrol();

  // Alarm sadece çok ciddi durumlarda devreye girsin
  if (alkolAlarm || kameraAlarm || nabizAlarm) {
    alarmCalistir();
  } else {
    alarmKapat();
  }

  // -------- PROCESSING'E VERİ GÖNDER --------
  Serial.print("S:");
  Serial.print(lastTemp);
  Serial.print("|H:");
  Serial.print(lastHum);
  Serial.print("|N:");
  Serial.print(lastNabiz);
  Serial.print("|A:");
  Serial.println(lastAlkol);
  //--------------------------------------------

  delay(300);
}

/* ---------- NABIZ KONTROLÜ (YENİ & UYUMLU) ---------- */
void nabizKontrol() {
  int val = analogRead(NABIZ_PIN);

  static int smooth = 0;
  smooth = (smooth * 9 + val) / 10;

  lastNabiz = smooth;

  // Çok yüksek nabız → alarm
  nabizAlarm = (smooth > 600);

  Serial.print("Nabiz sensörü: ");
  Serial.println(smooth);
}

/* ---------- ALKOL ---------- */
void alkolKontrol() {
  int val = analogRead(MQ3_PIN);
  alkolAlarm = (val > ALKOL_THRESHOLD);
  lastAlkol = val;

  Serial.print("MQ3: ");
  Serial.println(val);
}

/* ---------- ISI & NEM ---------- */
void isiNemKontrol() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) return;

  isiAlarm = (t > TEMP_THRESHOLD);
  nemAlarm = (h > HUM_THRESHOLD);

  lastTemp = t;
  lastHum  = h;

  Serial.print("Sicaklik: ");
  Serial.print(t);
  Serial.print("  Nem: ");
  Serial.println(h);
}

/* ---------- KAMERA KOMUTU ---------- */
void kameraKomutKontrol() {
  if (Serial.available()) {
    String komut = Serial.readStringUntil('\n');
    komut.trim();

    if (komut == "HIGH")  kameraAlarm = true;
    if (komut == "LOW")   kameraAlarm = false;
    if (komut == "1")     kameraAlarm = true;
    if (komut == "0")     kameraAlarm = false;
  }
}

/* ---------- ALARM FONKSİYONLARI ---------- */
void alarmCalistir() {
  for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], HIGH);
  digitalWrite(buzzerPin, HIGH);
}

void alarmKapat() {
  for (int i = 0; i < 4; i++) digitalWrite(ledPins[i], LOW);
  digitalWrite(buzzerPin, LOW);
}
