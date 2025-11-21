# ☀️ ESP32 Solar Tracker com TinyML (TCC)

Projeto de TCC focado no desenvolvimento de um rastreador solar de duplo eixo utilizando ESP32-S3. O sistema implementa controle híbrido: inicialmente baseado em lógica LDR e futuramente migrando para inferência de Redes Neurais (TensorFlow Lite Micro) embarcada.

## 🛠 Hardware

- **MCU**: ESP32-S3 (4D Systems Gen4 R8N16 / DevKit)
- **Atuadores**: 2x Servos MG995 (Azimute e Elevação)
- **Sensores**: 
  - 4x LDRs (GL5528) dispostos em quadrantes
  - Sensor de Tensão (Divisor resistivo)
- **Conectividade**: WiFi 2.4GHz (MQTT)

## ⚙️ Arquitetura de Software

O projeto utiliza **PlatformIO** com framework Arduino sobre **FreeRTOS**.

### Estrutura de Tarefas (RTOS)
| Task | Prioridade | Core | Descrição |
| :--- | :---: | :---: | :--- |
| `taskControl` | 3 (Alta) | 1 | Leitura de ADCs, lógica de comparação LDR e PID/ML dos servos. |
| `taskSensors` | 2 (Média)| 1 | Leitura lenta de tensão do painel e temperatura. |
| `taskWifiMqtt`| 1 (Baixa)| 0 | Manutenção da conexão e publicação de JSON via MQTT. |

### Fluxo de Dados (Fase Atual)
1. **Sensoriamento**: LDRs leem intensidade luminosa.
2. **Processamento**: Algoritmo compara (Cima+Baixo) e (Esq+Dir).
3. **Atuação**: Servos movem-se em passos discretos até o erro < `TOLERANCIA`.
4. **Telemetria**: Dados de sensores e ângulos são enviados via MQTT para coleta de Dataset.

## 🚀 Como Rodar

1. **Instalar Dependências**:
   Certifique-se de ter o VS Code + PlatformIO instalados.
2. **Configurar Credenciais**:
   Edite o arquivo `include/config.h` com seu SSID, Senha e Broker MQTT.
3. **Build & Upload**:
   ```bash
   pio run -t upload