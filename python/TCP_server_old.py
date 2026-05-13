#THE SERVER RECEIVING THE PACKETS AND PROTOBUF. THE ENTRY IS IN CSV FORMAT. OUTPUT TO THE TERMINAL

#===============================================LIBRARIES===============================================
import scan_pb2

import socket
import struct
from datetime import datetime
import ntplib
import sys

import csv
#=======================================================================================================


HOST = "0.0.0.0"
PORT = 9000


#=======================================INITIALIZATION VARIABLES========================================
use_protobuf = True         # Использование protobuf
if use_protobuf:
    file_name = 'ble_log_proto_csv.csv'
else:
    file_name = 'ble_log_packet_csv.csv'

offset = []
ntp_request_counter = 0
ntp_requests = 10           # Количество запросов на ntp сервер для вычисления avg_offset
record_counter = 1
#=======================================================================================================

def format_mac(mac):
    return ":".join(f"{b:02X}" for b in mac)

#===============================================START APP===============================================

# Подключение к NTP серверу для получения актуального времени и вычисление среднего
# смещения offset между системным временем (считается за монотонное) и временем с NTP сервера
while ntp_request_counter < ntp_requests:
    try:
        c = ntplib.NTPClient()
        response = c.request('pool.ntp.org', version=3, timeout=1)
        ntp_time_s = response.tx_time  # Время NTP сервера в секундах
    except Exception:
        print('\nNTP Server ERROR')
    except KeyboardInterrupt:
        print("Обработка KeyboardInterrupt при подключении к pool.ntp.org. Завершение работы...")
        sys.exit()
    else:
        # Время системное
        sys_dt = datetime.now()
        sys_time_s = sys_dt.timestamp()
        sys_time_ms = int(sys_time_s * 1000)

        ntp_dt = datetime.fromtimestamp(ntp_time_s)
        ntp_time_ms = int(ntp_time_s * 1000)

        # Смещение между системным и реальным utc
        offset_s = sys_time_s - ntp_time_s
        offset.append(offset_s)

        ntp_request_counter += 1

        print(ntp_request_counter, " Offset sys-ntp = ", offset_s, "s")

        if ntp_request_counter == ntp_requests:
            avg_offset_s = sum(offset) / len(offset)

            print("\nsys dt", sys_dt)
            print("sys sec", sys_time_s)
            print("sys ms", sys_time_ms)

            print("ntp dt", ntp_dt)
            print("ntp sec", ntp_time_s)
            print("ntp ms", ntp_time_ms)

            print("Average offset = ", avg_offset_s, "s")

            # CSV
            fieldnames = ['MAC', 'RSSI', 'Monotonic', 'UTC (ESP)', 'UTC Now (SYS)', 'UTC Now (NTP)', 'Time sync', 'UTC NTP - UTC ESP']
            with open(file_name, mode='w', newline='') as csv_file:
                # fieldnames — обязательный параметр
                writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
                # Записываем первую строку с заголовками
                writer.writeheader()


# Прослушивание порта, получение данных из потока, парсинг packet/protobuf(ble_event) сообщений, вывод в терминал, запись в csv файл
try:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        s.listen()

        while True:
            print("Waiting...")
            conn, addr = s.accept()
            print("Connected:", addr)

            buffer = b""

            try:
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break

                    buffer += data

                    while len(buffer) >= 2:
                        if use_protobuf:
                            # Protobuf parsing
                            # Читаем длину payload_len (little endian!)
                            length = int.from_bytes(buffer[0:2], "little")

                            # Проверяем что пакет полностью получен
                            if len(buffer) < 2 + length:
                                break

                            # Извлекаем payload
                            payload = buffer[2:2 + length]

                            # Обрезаем buffer (отсекаем извлеченный payload)
                            buffer = buffer[2 + length:]

                            # Decode protobuf
                            try:
                                event = scan_pb2.BleEvent()
                                event.ParseFromString(payload)

                                mac = event.mac.hex(":")
                                rssi = event.rssi
                                adv_data = event.adv_data.hex()
                                monotonic = event.monotonic_ms
                                utc_esp_ms = event.utc_ms
                                time_sync = event.time_sync

                            except Exception as e:
                                print("Decode error:", e)

                        else:
                            # Packet parsing
                            length = struct.unpack("<H", buffer[:2])[0]

                            if len(buffer) < 2 + length:
                                break

                            packet = buffer[2:2+length]
                            buffer = buffer[2+length:]

                            offset = 0

                            mac_ = packet[offset:offset+6]
                            mac = format_mac(mac_)
                            offset += 6

                            rssi = struct.unpack("b", packet[offset:offset+1])[0]
                            offset += 1

                            adv_len = packet[offset]
                            offset += 1

                            adv_data_ = packet[offset:offset+adv_len]
                            adv_data = adv_data_.hex()
                            offset += adv_len

                            monotonic = struct.unpack("<Q", packet[offset:offset+8])[0]
                            offset += 8

                            utc_esp_ms = struct.unpack("<Q", packet[offset:offset+8])[0]
                            offset += 8

                            time_sync = packet[offset]



                        dt_utc_esp = datetime.fromtimestamp(utc_esp_ms / 1000)  # Вычислить дату на основании времени с ESP

                        sys_dt = datetime.now()                         # Получение текущей системной даты и времени
                        sys_time_s = sys_dt.timestamp()                 # Перевод даты в секунды
                        ntp_time_s = sys_time_s - avg_offset_s          # Вычисление NTP времени на основе системного и смещения
                        ntp_dt = datetime.fromtimestamp(ntp_time_s)     # Перевод секунд в дату и время

                        ntp_time_ms = int(ntp_time_s * 1000)
                        ntp_time_us = int(ntp_time_s * 1000000)

                        delta = (ntp_time_us - utc_esp_ms*1000)/1000   # Разница (отклонение) между UTC NTP и UTC ESP

                        print("Размер payload:", length)
                        print("MAC:", mac)
                        print("RSSI:", rssi)
                        print("ADV:", adv_data)
                        print("Monotonic:", monotonic, "ms")
                        print("UTC (ESP):    ", dt_utc_esp, "ms")
                        print("UTC Now (SYS):", datetime.now(), "ms")
                        print("UTC Now (NTP):", ntp_dt, "ms")
                        print("ntp_ms: ", ntp_time_ms, "ms")
                        print("utc_esp:", utc_esp_ms, "ms")
                        print("UTC NTP - UTC ESP = ", delta, "ms")
                        print("Time sync:", time_sync)
                        print("Record counter:", record_counter)
                        print("--------")

                        # Запись в csv файл
                        with open(file_name, mode='a', newline='') as csv_file:
                            writer = csv.writer(csv_file)
                            writer.writerow([mac, rssi, monotonic, dt_utc_esp, datetime.now(), ntp_dt, time_sync, delta])
                        record_counter += 1

            except ConnectionAbortedError:
                print("Прервано соединение. Попытка переподключения")
            finally:
                conn.close()
                print("Соединение закрыто")
except KeyboardInterrupt:
    print("Обработка KeyboardInterrupt во время работы сервера. Закрытие сокета и завершение работы...")
    s.close()
    sys.exit()