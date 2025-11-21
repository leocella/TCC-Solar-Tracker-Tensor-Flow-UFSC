import serial
import csv
import time

# --- CONFIGURAÇÃO ---
COM_PORT = 'COM4'  # <--- CONFIRA SUA PORTA (No Linux/Mac é /dev/ttyUSB0)
BAUD_RATE = 115200
FILENAME = 'solar_data.csv'

try:
    ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
    print(f"Conectado na {COM_PORT}. Aguardando dados...")
except Exception as e:
    print(f"Erro ao abrir porta serial: {e}")
    exit()

# Abrir arquivo CSV
with open(FILENAME, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Cabeçalho (Labels)
    writer.writerow(['ldr_dc', 'ldr_db', 'ldr_ec', 'ldr_eb', 'az', 'el'])
    
    print("GRAVAÇÃO INICIADA! (Pressione Ctrl+C para parar)")
    
    try:
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                
                # Filtra apenas linhas que começam com "DATA,"
                if line.startswith("DATA,"):
                    # Remove o prefixo "DATA," e separa os valores
                    data_str = line.replace("DATA,", "")
                    values = data_str.split(',')
                    
                    if len(values) == 6:
                        writer.writerow(values)
                        print(f"Gravado: {values}")
                    else:
                        print(f"Ignorado (Formato ruim): {line}")
                        
    except KeyboardInterrupt:
        print("\nGravação finalizada pelo usuário.")
        ser.close()
        print(f"Arquivo salvo: {FILENAME}")