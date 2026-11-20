// KODE UNTUK SENSOR DHT11, SOIL MOISTURE, DAN I2C DARI ESP32

#include <Wire.h>
#include <BH1750.h>
#include <DHT.h>

// --- Konfigurasi Pin ---
#define DHTPIN 4          // Pin data DHT11 di GPIO 4
#define DHTTYPE DHT11     // Tipe sensor DHT11
#define SOIL_PIN 34       // Pin analog Soil Moisture di GPIO 34

// --- Kalibrasi Soil Moisture ---
// Sesuaikan nilai ini dengan hasil pembacaan mentah (raw) sensor Anda:
const int DRY_VALUE = 3500; // Nilai ADC saat tanah benar-benar kering (di udara)
const int WET_VALUE = 1200; // Nilai ADC saat sensor dicelupkan ke air

// --- Inisialisasi Objek ---
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

void setup() {
  Serial.begin(115200);
  Serial.println("\nMemulai Sistem Monitoring Greenhouse...");

  // Inisialisasi DHT11
  dht.begin();

  // Inisialisasi I2C untuk BH1750 (SDA = 21, SCL = 22)
  Wire.begin(21, 22);
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Sensor BH1750 berhasil diinisialisasi.");
  } else {
    Serial.println("Gagal mendeteksi BH1750! Periksa kabel SDA/SCL.");
  }

  // Konfigurasi pin Soil Moisture
  pinMode(SOIL_PIN, INPUT);
  delay(2000);
}

void loop() {
  // 1. Baca Suhu & Kelembapan Udara (DHT11)
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Dalam Celcius

  // 2. Baca Intensitas Cahaya (BH1750)
  float lux = lightMeter.readLightLevel();

  // 3. Baca Kelembapan Tanah (Soil Moisture)
  int rawSoil = analogRead(SOIL_PIN);
  // Ubah nilai mentah menjadi persentase (0% - 100%)
  int soilPercent = map(rawSoil, DRY_VALUE, WET_VALUE, 0, 100);
  soilPercent = constrain(soilPercent, 0, 100); // Batasi agar tetap di rentang 0-100%

  // --- Tampilkan Hasil di Serial Monitor ---
  Serial.println("========================================");
  
  // Cek apakah DHT11 terbaca dengan benar
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error: Gagal membaca data dari sensor DHT11!");
  } else {
    Serial.print("Suhu Udara       : ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Kelembapan Udara : ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // Cek pembacaan BH1750
  if (lux < 0) {
    Serial.println("Error: Gagal membaca sensor BH1750!");
  } else {
    Serial.print("Intensitas Cahaya: ");
    Serial.print(lux);
    Serial.println(" lx");
  }

  // Tampilkan data Soil Moisture
  Serial.print("Kelembapan Tanah : ");
  Serial.print(soilPercent);
  Serial.print(" % (Raw ADC: ");
  Serial.print(rawSoil);
  Serial.println(")");

  Serial.println("========================================\n");

  // Jeda 3 detik sebelum pembacaan berikutnya
  delay(3000);
}



