#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL6aJgRImCw"
#define BLYNK_TEMPLATE_NAME "siram"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>

char auth[] = "z9WIxH_YkQ5t7_vX-zhDdZBmt7gDluD6";
char ssid[] = "Kos Damai LT 3";
char pass[] = "Yontatex725";

#define DHTPIN 21
#define DHTTYPE DHT22
#define TRIG_PIN 14
#define ECHO_PIN 27
#define RELAY_POMPA 5
#define RELAY_UV 18
#define LDR_AO 34
#define LDR_DO 33
#define PH_PIN 26
#define PH_POWER 32
#define LED_PIN 19
#define BUZZER_PIN 23
#define BUTTON_PIN 25

#define V_PIN_TEMP V0
#define V_PIN_HUMIDITY V1
#define V_PIN_PH V2
#define V_PIN_LEVEL_AIR_PERSEN V3
#define V_PIN_LDR_ANALOG V4
#define V_PIN_TOMBOL_POMPA_MANUAL_BLYNK V5

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

unsigned long lastWaterTime = 0;
unsigned long waterInterval = 4UL * 60 * 60 * 1000;
unsigned long pumpDuration = 10000;

float lastSentTemperature = -999.9;
float lastSentHumidity = -999.9;
float lastSentPh = -999.9;
float lastSentTinggiPersen = -999.9;
int lastSentLdrAnalog = -9999;

#define RELAY_ON LOW
#define RELAY_OFF HIGH
#define LED_ON HIGH
#define LED_OFF LOW
#define BUZZER_ON HIGH
#define BUZZER_OFF LOW

float bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    Serial.println("Peringatan: pulseIn timeout pada sensor jarak!");
    return 50.0;
  }
  float distance = duration * 0.0343 / 2.0;
  return distance;
}

float bacaPH() {
  digitalWrite(PH_POWER, HIGH);
  delay(20);
  int analogValue = analogRead(PH_PIN);
  digitalWrite(PH_POWER, LOW);
  float voltage = analogValue * (3.3 / 4095.0);
  float phValue = 7.0 + ((2.5 - voltage) * 3.5);
  return phValue;
}

void sendSensorDataToBlynk() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float ph = bacaPH();
  float jarak_sensor = bacaJarak();
  float tinggiPersen = constrain(100.0 - ((jarak_sensor - 5.0) * 100.0 / (15.0 - 5.0)), 0.0, 100.0);
  int ldrAnalog = analogRead(LDR_AO);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error baca DHT sensor!");
  } else {
    if (abs(temperature - lastSentTemperature) > 0.1 || lastSentTemperature == -999.9) {
      Blynk.virtualWrite(V_PIN_TEMP, temperature);
      lastSentTemperature = temperature;
    }
    if (abs(humidity - lastSentHumidity) > 0.1 || lastSentHumidity == -999.9) {
      Blynk.virtualWrite(V_PIN_HUMIDITY, humidity);
      lastSentHumidity = humidity;
    }
  }
  if (abs(ph - lastSentPh) > 0.05 || lastSentPh == -999.9) {
    Blynk.virtualWrite(V_PIN_PH, ph);
    lastSentPh = ph;
  }
  if (abs(tinggiPersen - lastSentTinggiPersen) > 0.5 || lastSentTinggiPersen == -999.9) {
    Blynk.virtualWrite(V_PIN_LEVEL_AIR_PERSEN, tinggiPersen);
    lastSentTinggiPersen = tinggiPersen;
  }
  if (abs(ldrAnalog - lastSentLdrAnalog) > 10 || lastSentLdrAnalog == -9999) {
    Blynk.virtualWrite(V_PIN_LDR_ANALOG, ldrAnalog);
    lastSentLdrAnalog = ldrAnalog;
  }
  
  Serial.println("📡 Pembacaan Sensor:");
  if (!isnan(temperature)) {
    Serial.print("🌡️ Suhu: "); Serial.print(temperature, 2); Serial.println(" °C");
  } else {
    Serial.println("🌡️ Suhu: Error");
  }
  if (!isnan(humidity)) {
    Serial.print("💧 Kelembapan: "); Serial.print(humidity, 2); Serial.println(" %");
  } else {
    Serial.println("💧 Kelembapan: Error");
  }
  Serial.print("🧪 pH Air: "); Serial.println(ph, 2);
  Serial.print("📏 Jarak Air: "); Serial.print(jarak_sensor, 2); Serial.println(" cm");
  Serial.print("🌊 Level Air: "); Serial.print(tinggiPersen, 2); Serial.println(" %");
  Serial.print("☀️ Cahaya (Analog): "); Serial.println(ldrAnalog);

  bool uvStatusNyalaLokal = (digitalRead(RELAY_UV) == RELAY_ON);
  Serial.print("💡 UV Lampu: "); Serial.println(uvStatusNyalaLokal ? "NYALA 🔆" : "MATI 🌙");
  
  bool alarmAirRendahAktifLokal = (tinggiPersen < 20);
  Serial.print("🚨 Alarm Air Rendah: ");
  Serial.println(alarmAirRendahAktifLokal ? "AKTIF 🔴" : "AMAN 🟢");
  
  Serial.println("───────────────────────────────");
}

BLYNK_WRITE(V_PIN_TOMBOL_POMPA_MANUAL_BLYNK) {
  int pinValue = param.asInt();
  if (pinValue == 1) {
    Serial.println("🔘 Tombol Pompa Manual Blynk (V5) DITEKAN.");
    
    float jarak_cek_blynk = bacaJarak();
    float tinggiPersen_cek_blynk = constrain(100.0 - ((jarak_cek_blynk - 5.0) * 100.0 / (15.0 - 5.0)), 0.0, 100.0);
    
    if (tinggiPersen_cek_blynk > 20) {
      Serial.println("Pompa AKTIF 💧 via Blynk.");
      digitalWrite(RELAY_POMPA, RELAY_ON);
      timer.setTimeout(pumpDuration, []() {
          digitalWrite(RELAY_POMPA, RELAY_OFF);
          Serial.println("✅ Penyiraman manual via Blynk (V5) SELESAI.");
          Blynk.virtualWrite(V_PIN_TOMBOL_POMPA_MANUAL_BLYNK, 0);
      });
    } else {
      Serial.println("⚠️ Air tidak cukup. Pompa TIDAK dinyalakan via Blynk (V5).");
      Blynk.virtualWrite(V_PIN_TOMBOL_POMPA_MANUAL_BLYNK, 0);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n🚀 Sistem Aeroponik Otomatis dengan Blynk Dimulai (VPin disesuaikan)...");

  dht.begin();

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RELAY_POMPA, OUTPUT);
  pinMode(RELAY_UV, OUTPUT);
  pinMode(LDR_DO, INPUT);
  pinMode(PH_POWER, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_POMPA, RELAY_OFF);
  digitalWrite(RELAY_UV, RELAY_OFF);
  digitalWrite(PH_POWER, LOW);
  digitalWrite(LED_PIN, LED_OFF);
  digitalWrite(BUZZER_PIN, BUZZER_OFF);

  Serial.print("Menghubungkan ke WiFi: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  int wifiConnectAttempt = 0;
  while (WiFi.status() != WL_CONNECTED && wifiConnectAttempt < 20) {
    delay(500);
    Serial.print(".");
    wifiConnectAttempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi terhubung!");
    Serial.print("   Alamat IP: ");
    Serial.println(WiFi.localIP());
    Blynk.config(auth);
    Blynk.connect(3000);
    if (Blynk.connected()) {
        Serial.println("✅ Terhubung ke Server Blynk.");
    } else {
        Serial.println("⚠️ Gagal terhubung ke Server Blynk.");
    }
  } else {
    Serial.println("\n❌ Gagal terhubung ke WiFi setelah beberapa percobaan.");
  }
  
  timer.setInterval(5000L, sendSensorDataToBlynk);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (!Blynk.connected()) {
      Serial.println("Koneksi Blynk terputus, mencoba menghubungkan ulang ke Server Blynk...");
      Blynk.connect(3000);
      if(Blynk.connected()){
          Serial.println("✅ Re-koneksi ke Server Blynk berhasil.");
      } else {
          Serial.println("⚠️ Masih gagal re-koneksi ke Server Blynk.");
      }
    }
    Blynk.run();
  } else {
    Serial.println("Koneksi WiFi terputus. Mencoba menghubungkan ulang WiFi...");
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    int wifiReconnectAttempt = 0;
    while (WiFi.status() != WL_CONNECTED && wifiReconnectAttempt < 6) {
        delay(500);
        Serial.print("*");
        wifiReconnectAttempt++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi berhasil terhubung kembali!");
    } else {
        Serial.println("\n❌ Masih gagal terhubung kembali ke WiFi. Akan dicoba lagi nanti.");
        delay(5000);
    }
  }
  timer.run();

  float jarak_lokal = bacaJarak();
  float tinggiPersen_lokal = constrain(100.0 - ((jarak_lokal - 5.0) * 100.0 / (15.0 - 5.0)), 0.0, 100.0);

  if (tinggiPersen_lokal < 20) {
    digitalWrite(LED_PIN, LED_OFF);
    digitalWrite(BUZZER_PIN, BUZZER_ON);
  } else {
    digitalWrite(LED_PIN, LED_ON);
    digitalWrite(BUZZER_PIN, BUZZER_OFF);
  }

  static unsigned long lastButtonPressTime = 0;
  static bool buttonStateLatched = false;
  const unsigned long debounceDelay = 50;

  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!buttonStateLatched && (millis() - lastButtonPressTime > debounceDelay)) {
      lastButtonPressTime = millis();
      buttonStateLatched = true;
      Serial.println("🔘 Tombol Fisik ditekan: Pompa Manual AKTIF 💧");
      if (tinggiPersen_lokal > 20) {
          digitalWrite(RELAY_POMPA, RELAY_ON);
          unsigned long pumpStartTime = millis();
          while(millis() - pumpStartTime < pumpDuration) {
            if (WiFi.status() == WL_CONNECTED) Blynk.run();
            timer.run();
            delay(10);
          }
          digitalWrite(RELAY_POMPA, RELAY_OFF);
          Serial.println("✅ Penyiraman manual fisik selesai.");
      } else {
          Serial.println("⚠️ Air tidak cukup. Pompa TIDAK dinyalakan (fisik).");
      }
    }
  } else {
      if (buttonStateLatched && (millis() - lastButtonPressTime > debounceDelay)) {
        buttonStateLatched = false;
      }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastWaterTime >= waterInterval) {
    float jarak_otomatis = bacaJarak();
    float tinggiPersen_otomatis = constrain(100.0 - ((jarak_otomatis - 5.0) * 100.0 / (15.0 - 5.0)), 0.0, 100.0);
    if (tinggiPersen_otomatis > 20) {
      Serial.println("💧 Pompa MENYALA (penyiraman otomatis)...");
      digitalWrite(RELAY_POMPA, RELAY_ON);
      unsigned long pumpStartTimeAuto = millis();
      while(millis() - pumpStartTimeAuto < pumpDuration) {
        if (WiFi.status() == WL_CONNECTED) Blynk.run();
        timer.run();
        delay(10);
      }
      digitalWrite(RELAY_POMPA, RELAY_OFF);
      Serial.println("✅ Penyiraman otomatis SELESAI.");
    } else {
      Serial.println("⚠️ Air tidak cukup. Pompa otomatis TIDAK dinyalakan.");
    }
    lastWaterTime = currentMillis;
  }

  if (digitalRead(LDR_DO) == LOW) {
    digitalWrite(RELAY_UV, RELAY_OFF);
  } else {
    digitalWrite(RELAY_UV, RELAY_ON);
  }
}