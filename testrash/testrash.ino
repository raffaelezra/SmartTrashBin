#include <ezraraffael-project-1_inferencing.h>

// Tentukan pin analog yang kamu pakai (Gunakan ADC1, contoh: GPIO 4)
#define MIC_PIN 4 

// KOREKSI: Menggunakan tipe data 'float' agar sesuai dengan library Edge Impulse
static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
unsigned long sampling_period_us;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Atur pin mikrofon sebagai input analog
    pinMode(MIC_PIN, INPUT);

    // Hitung periode sampling berdasarkan frekuensi model (16000Hz / 16kHz)
    sampling_period_us = round(1000000 * (1.0 / EI_CLASSIFIER_FREQUENCY));
    
    Serial.println("===============================================");
    Serial.println("=== Edge Impulse Testing Terbuka (MAX9814) ===");
    Serial.println("===============================================");
    Serial.println("Silakan ucapkan: Kertas / Logam / Plastik...");
}

void loop() {
    // 1. Ambil data audio analog dari MAX9814
    for (size_t i = 0; i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; i++) {
        unsigned long microseconds = micros();
        
        // Baca sinyal analog (ESP32-S3 default ADC nya 12-bit, nilai tengah ~2048)
        features[i] = (float)analogRead(MIC_PIN) - 2048.0; 

        // Tunggu sampai waktu sampling berikutnya pas (sesuai frekuensi model)
        while (micros() < (microseconds + sampling_period_us)) {
            // Tunggu sebentar/do nothing
        }
    }

    // 2. Bungkus data ke struktur Edge Impulse
    signal_t signal;
    int err = numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
    if (err != 0) {
        Serial.print("Gagal membuat sinyal dari buffer: ");
        Serial.println(err);
        return;
    }

    // 3. Jalankan Classifier (Model AI mendeteksi suara)
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR r = run_classifier(&signal, &result, false);
    if (r != EI_IMPULSE_OK) {
        Serial.print("Gagal menjalankan classifier: ");
        Serial.println(r);
        return;
    }

    // 4. Cek hasil prediksi dan Print jika akurasi di atas 80% (0.80)
    bool keyword_terdeteksi = false;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        String label = result.classification[ix].label;
        float score = result.classification[ix].value;

        // Abaikan kelas NOISE dan UNKNOWN agar Serial Monitor fokus ke keyword
        if (label != "NOISE" && label != "UNKNOWN") {
            if (score > 0.80) { // Threshold akurasi 80%
                Serial.println("");
                Serial.print("[TERDETEKSI] ---> ");
                Serial.print(label);
                Serial.print(" (Akurasi: ");
                Serial.print(score * 100);
                Serial.println("%)");
                keyword_terdeteksi = true;
            }
        }
    }

    // Indikator titik kalau model sedang stand-by mendengarkan
    if (!keyword_terdeteksi) {
        Serial.print("."); 
    }

    delay(100); // Delay kecil biar pembacaan tidak terlalu beruntun
}