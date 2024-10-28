#include <Arduino.h>
#include <HX711.h>

// Variabel untuk menghitung laju tetesan
unsigned long lastDropTime = 0;
unsigned long totalDropTime = 0;
unsigned long totalDropTimeDisplay = 0;

float dripRate = 0;
float dropCountDisplay = 0;
int dripPin = 13; // Sesuaikan dengan pin yang Anda gunakan untuk sensor tetesan
int dropCount = 0;

// Variabel HX711
const int LOADCELL_DOUT_PIN = 27; // Pin DT
const int LOADCELL_SCK_PIN = 26;  // Pin SCK
HX711 scale;
float calibrationFactor = 2280.0; // Sesuaikan dengan faktor kalibrasi yang ditemukan
long offset = 0; // Offset tanpa beban

// Pin untuk buzzer
const int buzzerPin = 4; // Sesuaikan dengan pin yang Anda gunakan untuk buzzer

// Handle untuk tugas-tugas RTOS
TaskHandle_t TaskDripSensor;
TaskHandle_t TaskScaleSensor;

void setup() {
  // Setup untuk sensor tetesan
  pinMode(dripPin, INPUT);

  // Setup untuk HX711
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Membaca nilai tanpa beban untuk mendapatkan offset
  scale.tare();
  offset = scale.read();

  // Setup untuk buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW); // Pastikan buzzer mati saat mulai

  // Membuat tugas-tugas RTOS
  xTaskCreate(readDripSensor, "Task Drip Sensor", 1000, NULL, 1, &TaskDripSensor);
  xTaskCreate(readScaleSensor, "Task Scale Sensor", 1000, NULL, 1, &TaskScaleSensor);
}

void loop() {
  // Tidak ada kode di loop() karena semua tugas dijalankan oleh RTOS
}

void readDripSensor(void *pvParameters) {
  (void) pvParameters;
  unsigned long prevMillis = 0;
  unsigned long interval = 3000;

  for (;;) {
    unsigned long currMillis = millis();

    // Kode sensor tetesan
    if (digitalRead(dripPin) == 1 && currMillis - lastDropTime > 900) { 
      unsigned long dropTime = millis();
      unsigned long diff = dropTime - lastDropTime;

      totalDropTimeDisplay += diff;
      dropCountDisplay++;

      totalDropTime += diff;
      dropCount++;
      lastDropTime = dropTime;
    }

    if (currMillis - prevMillis >= interval) { 
      if (totalDropTime != 0) {
        dripRate = (float)dropCount / ((float)totalDropTime / 60000.0);  
      } else {
        dripRate = 0;
      }

      // Reset totalDropTime, dropCount, dan prevMillis
      totalDropTime = 0;
      dropCount = 0;
      prevMillis = currMillis;
    }

    // Cetak data laju tetesan
    Serial.print("Total waktu tiap tetesan: ");
    Serial.print((float)totalDropTimeDisplay / 1000);
    Serial.print(", Jumlah tetesan: ");
    Serial.print(dropCountDisplay);
    Serial.print(", Laju tetesan: ");
    Serial.println(dripRate);

    // Penundaan untuk memberi waktu tugas lain
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void readScaleSensor(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    // Kode HX711
    if (scale.is_ready()) {
      long reading = scale.read() - offset;
      float weight = reading / calibrationFactor;
      Serial.print("Berat: ");
      Serial.print(weight);
      Serial.println(" gram");

      // Trigger buzzer jika berat kurang dari 10 gram
      if (weight < 10.0) {
        digitalWrite(buzzerPin, HIGH); // Nyalakan buzzer
      } else {
        digitalWrite(buzzerPin, LOW); // Matikan buzzer
      }

    } else {
      Serial.println("HX711 tidak ditemukan.");
    }

    // Penundaan untuk memberi waktu tugas lain
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
