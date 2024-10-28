#include <Arduino.h>
#include "HX711.h"

// Variabel untuk menghitung laju tetesan
unsigned long lastDropTime = 0;
unsigned long totalDropTime = 0;
unsigned long totalDropTimeDisplay = 0;
unsigned long prevMillis = 0;
unsigned long interval = 3000;

float dripRate = 0;
float dropCountDisplay = 0;
int dripPin = 13; // Sesuaikan dengan pin yang Anda gunakan untuk sensor tetesan
int dropCount = 0;

// Variabel HX711
const int LOADCELL_DOUT_PIN = 27; // Pin DT
const int LOADCELL_SCK_PIN = 26;  // Pin SCK
HX711 scale;

void setup() {
  // Setup untuk sensor tetesan
  pinMode(dripPin, INPUT);

  // Setup untuk HX711
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop() {
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

  // Kode HX711
  if (scale.is_ready()) {
    long reading = scale.read();
    Serial.print("Pembacaan HX711: ");
    Serial.println(reading);
  } else {
    Serial.println("HX711 tidak ditemukan.");
  }

  // Cetak data laju tetesan
  Serial.print("Total waktu tiap tetesan: ");
  Serial.print((float)totalDropTimeDisplay / 1000);
  Serial.print(", Jumlah tetesan: ");
  Serial.print(dropCountDisplay);
  Serial.print(", Laju tetesan: ");
  Serial.println(dripRate);

  delay(1000); // Delay untuk membuat output serial lebih mudah dibaca
}
