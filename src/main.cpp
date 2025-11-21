#include <Arduino.h>
#include <WiFi.h> 
#include <ESP32Servo.h>
#include "config.h"

// Objetos Globais
Servo servoHor;
Servo servoVer;

// Variáveis de Estado
int posHor = 90;
int posVer = 90;
bool modoAutomatico = true; 

// --- Protótipos das Tasks ---
void taskControl(void *pvParameters); 

void setup() {
    Serial.begin(115200);
    
    // Inicializa Servos
    servoHor.setPeriodHertz(50);
    servoHor.attach(PIN_SERVO_HOR, 500, 2400);
    servoVer.setPeriodHertz(50);
    servoVer.attach(PIN_SERVO_VER, 500, 2400);
    
    // Posição inicial
    servoHor.write(posHor);
    servoVer.write(posVer);

    // Core 1: Controle (Apenas essa task importa agora)
    xTaskCreatePinnedToCore(taskControl, "Control", 4096, NULL, 3, NULL, 1);
    
    //  TASK DE WIFI/MQTT DESLIGADA PARA NÃO SUJAR O CSV
    // xTaskCreatePinnedToCore(taskWifiMqtt, "Comms", 4096, NULL, 1, NULL, 0);

    Serial.println("SISTEMA PRONTO PARA GRAVACAO DE DADOS - GEN EMBARCADOS");
    delay(1000); // Tempo para ler a msg
}

void loop() {
    vTaskDelete(NULL);
}

// =========================================
// TASK 1: CONTROLE + GRAVAÇÃO SERIAL
// =========================================
void taskControl(void *pvParameters) {
    Serial.println("TASK INICIADA!"); // Debug inicial

    for (;;) {
        // 1. Ler Sensores
        int ldc = analogRead(PIN_LDR_DC);
        int ldb = analogRead(PIN_LDR_DB);
        int lec = analogRead(PIN_LDR_EC);
        int leb = analogRead(PIN_LDR_EB);

        // 2. Imprimir AGORA (Sem esperar lógica)
        // Isso vai confirmar se o ESP32 está vivo e lendo
        Serial.printf("DATA,%d,%d,%d,%d,%d,%d\n", ldc, ldb, lec, leb, posHor, posVer);

        // --- Lógica Simples de Teste (Balança o servo pra provar que mexe) ---
        // Se os LDRs estiverem mortos, vamos mexer o servo na força bruta a cada 2s
        // Descomente as linhas abaixo APENAS se quiser testar o motor ignorando LDR
        /*
        static bool toggle = false;
        if (millis() % 2000 < 50) {
            toggle = !toggle;
            servoHor.write(toggle ? 90 : 120);
            Serial.println("Movendo Servo Teste!");
        }
        */

        // --- Lógica Original (LDR) ---
        // Se estiver tudo preto (leitura < 100), diminua a tolerância
        int tol = TOLERANCIA_LDR;
        if (ldc < 200) tol = 10; // Aumenta sensibilidade no escuro

        int valSup = (ldc + lec) / 2;
        int valInf = (ldb + leb) / 2;
        int difV = valSup - valInf;
        
        if (abs(difV) > tol) {
            if (valSup > valInf) posVer++; else posVer--;
            if (posVer > SERVO_V_MAX) posVer = SERVO_V_MAX;
            if (posVer < SERVO_V_MIN) posVer = SERVO_V_MIN;
            servoVer.write(posVer);
        }

        int valDir = (ldc + ldb) / 2;
        int valEsq = (lec + leb) / 2;
        int difH = valDir - valEsq;
        
        if (abs(difH) > tol) {
            if (valDir > valEsq) posHor--; else posHor++; 
            if (posHor > SERVO_H_MAX) posHor = SERVO_H_MAX;
            if (posHor < SERVO_H_MIN) posHor = SERVO_H_MIN;
            servoHor.write(posHor);
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
