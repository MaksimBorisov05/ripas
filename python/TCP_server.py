#THE SERVER RECEIVING THE PROTOBUF SCANBATCH ONLY. THE ENTRY IS IN CSV FORMAT. OUTPUT TO THE TERMINAL

#===============================================LIBRARIES===============================================
import scan_pb2

import socket
from datetime import datetime
import ntplib
import sys

import csv
#=======================================================================================================


HOST = "0.0.0.0"
PORT = 9000


#=======================================INITIALIZATION VARIABLES========================================
file_name = 'ble_log_scanBacthProto_csv.csv'

offset = []
ntp_request_counter = 0
ntp_requests = 10           # Количество запросов на ntp сервер для вычисления avg_offset
raw_counter = 1
message_counter = 1
#------------------------------Временные переменные------------------------------
max_data_size = 0
max_avg_bleEvent_size = 0
#=======================================================================================================


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

            # Excel
            '''
            if not os.path.exists("ble_log.xlsx"):
                wb = Workbook()
                ws = wb.active
                ws.append(["MAC", "RSSI", "Monotonic", "UTC (ESP)",
                           "UTC Now (SYS)", "UTC Now (NTP)",
                           "Time sync", "UTC NTP - UTC ESP",])
                ws.append(["UTC (ESP)", "UTC Now (NTP)", "Time sync", "UTC NTP - UTC ESP", ])
                wb.save("ble_log.xlsx")'''



# Прослушивание порта, получение данных из потока, парсинг ScanBatch сообщений, вывод в терминал, запись в csv файл
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
                            batch = scan_pb2.ScanBatch()
                            batch.ParseFromString(payload)

                            print("=========================================================================================================")
                            print("Batch seq:\t", batch.batch_seq)
                            print("Events:\t\t", len(batch.ble))
                            print("Размер сообщения (bin):\t", len(data))
                            print("Размер payload (bin):\t", length)

                            avg_bleEvent_size = length / len(batch.ble)
                            print("Average ble event size:\t\t\t\t\t", avg_bleEvent_size)
                            print("---------------")

                            if len(data)>max_data_size:
                                max_data_size = len(data)
                            print("Max data in session:\t\t\t\t\t", max_data_size)

                            if avg_bleEvent_size > max_avg_bleEvent_size:
                                max_avg_bleEvent_size = avg_bleEvent_size
                            print("Max average ble event size in session:\t", max_avg_bleEvent_size)
                            print("---------------")

                            for ev in batch.ble:
                                mac = ev.mac.hex(":")
                                rssi = ev.rssi
                                adv_data = ev.adv_data.hex()
                                monotonic = ev.monotonic_ms
                                utc_esp_ms = ev.utc_ms
                                time_sync = ev.time_sync

                                dt_utc_esp = datetime.fromtimestamp(utc_esp_ms / 1000) # Вычислить дату на основании времени с ESP

                                sys_dt = datetime.now()                     # Получение текущей системной даты и времени
                                sys_time_s = sys_dt.timestamp()             # Перевод даты в секунды
                                ntp_time_s = sys_time_s - avg_offset_s      # Вычисление NTP времени на основе системного и смещения
                                ntp_dt = datetime.fromtimestamp(ntp_time_s) # Перевод секунд в дату и время

                                ntp_time_ms = int(ntp_time_s * 1000)
                                ntp_time_us = int(ntp_time_s * 1000000)

                                delta = (ntp_time_us - utc_esp_ms * 1000) / 1000 # Разница (отклонение) между UTC NTP и UTC ESP

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
                                print("Raw counter:", raw_counter)
                                print("Message counter:", message_counter)
                                print("--------")

                                # Запись в csv файл
                                with open(file_name, mode='a', newline='') as csv_file:
                                    writer = csv.writer(csv_file)
                                    writer.writerow(
                                        [mac, rssi, monotonic, dt_utc_esp, datetime.now(), ntp_dt, time_sync, delta])
                                raw_counter += 1
                            message_counter += 1

                        except Exception as e:
                            print("Decode error:", e)
            except ConnectionAbortedError:
                print("Прервано соединение. Попытка переподключения")
            finally:
                conn.close()
                print("Соединение закрыто")
except KeyboardInterrupt:
    print("Обработка KeyboardInterrupt во время работы сервера. Закрытие сокета и завершение работы...")
    s.close()
    sys.exit()