#pragma once

// --- Configuração WiFi / MQTT ---
#define WIFI_SSID       "API_001"
#define WIFI_PASS       "SOLARtracker"
#define MQTT_SERVER     "10.0.0.156"
#define MQTT_PORT       1883
#define MQTT_USER       "solar"
#define MQTT_PASS       "tracker"
#define MQTT_TOPIC_TELEM "campus/tracker/telemetry"
#define MQTT_TOPIC_CMD   "campus/tracker/cmd"

// --- Pinos LDR (ADC) ---
// ATENÇÃO: No seu código original, GPIO 5 era usado para LDREC e LDR genérico.
// Separei aqui para evitar conflito. Verifique seu hardware!
#define PIN_LDR_GLOBAL  5  // LDR genérico de luminosidade
#define PIN_LDR_DC      4  // Direita Cima
#define PIN_LDR_DB      7  // Direita Baixo
#define PIN_LDR_EC      5  // Esquerda Cima (CONFLITO POTENCIAL com LDR_GLOBAL)
#define PIN_LDR_EB      6  // Esquerda Baixo

#define PIN_VOLT_PAINEL 10

// --- Pinos Servos ---
#define PIN_SERVO_HOR   13
#define PIN_SERVO_VER   14

// --- Constantes de Controle ---
#define SERVO_H_MIN     0
#define SERVO_H_MAX     180
#define SERVO_V_MIN     50
#define SERVO_V_MAX     150
#define TOLERANCIA_LDR  60
#define CHECK_INTERVAL  100 // ms