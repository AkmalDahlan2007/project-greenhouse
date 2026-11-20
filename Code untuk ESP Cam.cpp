//KODE UNTUK ESP CAM
#include "WiFi.h"
#include "HTTPClient.h"
#include "esp_camera.h"

// Pin kamera untuk modul AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

const char* ssid = "NAMA_WIFI_ANDA";          // <--- GANTI
const char* password = "PASSWORD_WIFI_ANDA"; // <--- GANTI

// Ganti dengan IP laptop/server lokal (satu jaringan Wi-Fi) dan port endpoint Python
const char* serverUrl = "http://192.168.1.50:5000/predict"; // <--- GANTI

const unsigned long intervalMs = 10000; // Interval ambil foto (10 detik)
unsigned long lastCaptureTime = 0;

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Resolusi QVGA (320x240) atau VGA (640x480) cukup ringan dan cepat dikirim
  if (psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 15;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Kamera gagal diinisialisasi! Error: 0x%x\n", err);
    return false;
  }
  return true;
}

void sendPhotoToServer() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Gagal mengambil frame kamera");
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "image/jpeg");

    int httpResponseCode = http.POST(fb->buf, fb->len);
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("HTTP %d | Respon model: %s\n", httpResponseCode, response.c_str());
    } else {
      Serial.printf("Gagal kirim HTTP POST: %s\n", http.errorToString(httpResponseCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Wi-Fi terputus");
  }

  // Kembalikan buffer frame agar memori tidak bocor
  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!initCamera()) {
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi terhubung! IP ESP32-CAM: " + WiFi.localIP().toString());
}

void loop() {
  if (millis() - lastCaptureTime >= intervalMs) {
    lastCaptureTime = millis();
    sendPhotoToServer();
  }
}