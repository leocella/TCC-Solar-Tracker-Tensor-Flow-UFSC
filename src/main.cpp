#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <TensorFlowLite_ESP32.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "config.h"
#include "model_data.h"

// --- Globais TFLite ---
tflite::ErrorReporter* error_reporter = nullptr;
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// Aumentado para 60KB para garantir (Float32 gasta mais RAM, mas você tem sobra)
const int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

// --- Globais Hardware ---
Servo servoHor;
Servo servoVer;

void setupML();
void taskControl(void *pvParameters);

void setup() {
    Serial.begin(115200);
    
    servoHor.setPeriodHertz(50);
    servoHor.attach(PIN_SERVO_HOR, 500, 2400);
    servoVer.setPeriodHertz(50);
    servoVer.attach(PIN_SERVO_VER, 500, 2400);

    setupML();

    // Aumentei a Stack da Task para 8KB
    xTaskCreatePinnedToCore(taskControl, "Control", 8192, NULL, 3, NULL, 1);

    Serial.println("=== SISTEMA SOLAR TRACKER (FLOAT32) INICIADO ===");
}

void loop() {
    vTaskDelete(NULL);
}

void setupML() {
    static tflite::MicroErrorReporter micro_error_reporter;
    error_reporter = &micro_error_reporter;

    model = tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        Serial.println("Erro Schema!");
        return;
    }

    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    if (interpreter->AllocateTensors() != kTfLiteOk) {
        Serial.println("Erro: Falha AllocateTensors!");
        return;
    }

    input = interpreter->input(0);
    output = interpreter->output(0);
    Serial.println("IA Carregada! (Modo Float32)");
}

// Variáveis para guardar a posição anterior (Filtro)
float anguloAzFiltrado = 90.0;
float anguloElFiltrado = 90.0;

void taskControl(void *pvParameters) {
    // Fator de Suavização (0.0 a 1.0)
    // 0.1 = Muito suave (lento)
    // 0.5 = Médio
    // 0.9 = Rápido (quase sem filtro)
    const float SUAVIZACAO = 0.15; 

    for (;;) {
        // 1. Ler Sensores
        int ldc = analogRead(PIN_LDR_DC);
        int ldb = analogRead(PIN_LDR_DB);
        int lec = analogRead(PIN_LDR_EC);
        int leb = analogRead(PIN_LDR_EB);

        if (input != nullptr) {
            // Normaliza Entrada
            input->data.f[0] = ldc / 4095.0f;
            input->data.f[1] = ldb / 4095.0f;
            input->data.f[2] = lec / 4095.0f;
            input->data.f[3] = leb / 4095.0f;

            // Inferência
            if (interpreter->Invoke() == kTfLiteOk) {
                
                // Pega o "alvo" bruto da IA
                float alvoAz = output->data.f[0] * 180.0;
                float alvoEl = output->data.f[1] * 180.0;

                // --- A MÁGICA DO FILTRO EXPONENCIAL ---
                // Novo = (Antigo * 0.85) + (Alvo * 0.15)
                anguloAzFiltrado = (anguloAzFiltrado * (1.0 - SUAVIZACAO)) + (alvoAz * SUAVIZACAO);
                anguloElFiltrado = (anguloElFiltrado * (1.0 - SUAVIZACAO)) + (alvoEl * SUAVIZACAO);

                // Converte para inteiro para o Servo
                int posAz = (int)anguloAzFiltrado;
                int posEl = (int)anguloElFiltrado;

                // Clamp de Segurança
                if (posAz > 180) posAz = 180; if (posAz < 0) posAz = 0;
                if (posEl > 160) posEl = 160; if (posEl < 20) posEl = 20;

                // Mover Servos (Agora vai ser macio!)
                servoHor.write(posAz);
                servoVer.write(posEl);
            }
        }
        // Loop rápido para o filtro funcionar bem
        vTaskDelay(pdMS_TO_TICKS(20)); 
    }
}