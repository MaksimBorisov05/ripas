from flask import Flask, request, jsonify
from datetime import datetime

app = Flask(__name__)


@app.route('/api/ble', methods=['POST'])
def handle_ble_data():
    try:
        data = request.get_json()
        device_count = data.get('devices_count', 0)
        device_name = data.get('device', 'unknown')

        current_time = datetime.now().strftime('%H:%M:%S')
        print(f"\n[{current_time}] 📡 From: {device_name}")
        print(f"📊 BLE Devices found: {device_count}")

        return jsonify({
            "status": "success",
            "message": f"Received from {device_name}",
            "devices_count": device_count,
            "server_time": current_time
        }), 200

    except Exception as e:
        print(f"❌ Error: {e}")
        return jsonify({"status": "error"}), 500


if __name__ == '__main__':
    print("🚀 BLE Server started on http://192.168.1.1:5000")
    app.run(host='192.168.1.1', port=5000, debug=False)