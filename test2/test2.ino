#include <ezraraffael-project-1_inferencing.h>

#define MIC_PIN 4

static float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
unsigned long sampling_period_us;

void setup() {

    Serial.begin(115200);
    while (!Serial);

    pinMode(MIC_PIN, INPUT);

    // SAMA seperti saat recording dataset
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    sampling_period_us =
        round(1000000.0 / EI_CLASSIFIER_FREQUENCY);

    Serial.println("======================================");
    Serial.println(" Edge Impulse + MAX9814 ");
    Serial.println("======================================");

    Serial.print("Frame size : ");
    Serial.println(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);

    Serial.print("Frequency  : ");
    Serial.println(EI_CLASSIFIER_FREQUENCY);

    Serial.print("Sampling period us : ");
    Serial.println(sampling_period_us);
}

void loop() {

    // =====================================
    // RECORD AUDIO
    // =====================================
    for (size_t i = 0;
         i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
         i++) {

        unsigned long startMicros = micros();

        features[i] = (float)analogRead(MIC_PIN);

        while ((micros() - startMicros)
               < sampling_period_us) {
        }
    }

    // =====================================
    // DEBUG RAW ADC
    // =====================================
    float rawMin = 999999;
    float rawMax = -999999;

    for (size_t i = 0;
         i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
         i++) {

        if (features[i] < rawMin)
            rawMin = features[i];

        if (features[i] > rawMax)
            rawMax = features[i];
    }

    Serial.print("RAW MIN=");
    Serial.print(rawMin);

    Serial.print(" RAW MAX=");
    Serial.println(rawMax);

    // =====================================
    // REMOVE DC OFFSET
    // =====================================
    float mean = 0.0f;

    for (size_t i = 0;
         i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
         i++) {

        mean += features[i];
    }

    mean /= EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;

    for (size_t i = 0;
         i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
         i++) {

        features[i] -= mean;
    }

    // =====================================
    // NORMALISASI
    // =====================================
    float max_abs = 0.0f;

    for (size_t i = 0;
         i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
         i++) {

        float v = fabs(features[i]);

        if (v > max_abs)
            max_abs = v;
    }

    if (max_abs > 0.0f) {

        for (size_t i = 0;
             i < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;
             i++) {

            features[i] /= max_abs;
        }
    }

    Serial.print("MAX ABS=");
    Serial.println(max_abs);

    // =====================================
    // SIGNAL
    // =====================================
    signal_t signal;

    int err = numpy::signal_from_buffer(
        features,
        EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE,
        &signal
    );

    if (err != 0) {

        Serial.print("Signal Error: ");
        Serial.println(err);
        return;
    }

    // =====================================
    // CLASSIFIER
    // =====================================
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR res =
        run_classifier(
            &signal,
            &result,
            false
        );

    if (res != EI_IMPULSE_OK) {

        Serial.print("Classifier Error: ");
        Serial.println(res);
        return;
    }

    // =====================================
    // PRINT HASIL
    // =====================================
    Serial.println();
    Serial.println("===== HASIL =====");

    float bestScore = 0.0f;
    String bestLabel = "";

    for (size_t ix = 0;
         ix < EI_CLASSIFIER_LABEL_COUNT;
         ix++) {

        String label =
            result.classification[ix].label;

        float score =
            result.classification[ix].value;

        Serial.print(label);
        Serial.print(" : ");
        Serial.println(score, 4);

        if (score > bestScore) {

            bestScore = score;
            bestLabel = label;
        }
    }

    Serial.println("--------------------");

    Serial.print("Prediksi: ");
    Serial.print(bestLabel);

    Serial.print(" (");
    Serial.print(bestScore * 100.0f);
    Serial.println("%)");

    Serial.println("--------------------");

    delay(1000);
}