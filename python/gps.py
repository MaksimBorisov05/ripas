import serial
import pynmea2
import time
import sys
from datetime import datetime

# Параметры — подставь свой COM и скорость (например 'COM5', 4800)
PORT = 'COM6'      # или r'\\.\COM10' если COM>9
BAUD = 9600
TIMEOUT = 5        # сек

def nmea_to_decimal(coord, direction):
    # coord: строка ddmm.mmmm или dddmm.mmmm
    if not coord or coord == '':
        return None
    try:
        f = float(coord)
    except:
        return None
    degrees = int(f // 100)
    minutes = f - degrees * 100
    dec = degrees + minutes / 60.0
    if direction in ('S', 'W'):
        dec = -dec
    return dec

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=TIMEOUT)
        print(f"Opened {PORT} @ {BAUD} baud")
    except Exception as e:
        print("Не могу открыть порт:", e)
        sys.exit(1)

    try:
        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if not line:
                continue
            # Опционально показываем «сырые» NMEA
            # print("RAW:", line)

            if line.startswith('$'):
                try:
                    msg = pynmea2.parse(line)
                except pynmea2.ParseError:
                    continue

                # GGA содержит fix, координаты, кол-во спутников
                if isinstance(msg, pynmea2.types.talker.GGA):
                    lat = nmea_to_decimal(msg.lat, msg.lat_dir)
                    lon = nmea_to_decimal(msg.lon, msg.lon_dir)
                    fix = msg.gps_qual  # 0 = no fix, 1 = GPS fix, 2 = DGPS
                    sats = msg.num_sats
                    alt = msg.altitude
                    ts = datetime.utcnow().isoformat()
                    print(f"[{ts}] GGA lat={lat:.6f} lon={lon:.6f} fix={fix} sats={sats} alt={alt}")

                # RMC содержит статус, скорость и время, и тоже координаты
                elif isinstance(msg, pynmea2.types.talker.RMC):
                    lat = nmea_to_decimal(msg.lat, msg.lat_dir)
                    lon = nmea_to_decimal(msg.lon, msg.lon_dir)
                    status = msg.status  # 'A' = data valid
                    speed = msg.spd_over_grnd
                    course = msg.true_course
                    ts = datetime.utcnow().isoformat()

                    if lat is not None and lon is not None:
                        print(f"[{ts}] RMC lat={lat:.6f} lon={lon:.6f} status={status} speed={speed} course={course}")
                    else:
                        print(f"[{ts}] RMC no fix yet (status={status})")

            # небольшая пауза, чтобы не спамить
            time.sleep(0.01)

    except KeyboardInterrupt:
        print("Stopping...")
    finally:
        ser.close()

if __name__ == '__main__':
    main()
