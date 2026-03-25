#API for website

from flask import Flask, request, jsonify
from flask_cors import CORS
import time
import requests

app = Flask(__name__)
CORS(app)

bus_data = {}

GOOGLE_SHEET_URL = "https://script.google.com/macros/s/AKfycbzSzTxr8dzgx79bpR8iMTTs0UuDAXfot4JG2ZUiRlHyIIejVr7dsb7mQDEWDoaV9UpmnA/exec"


@app.route('/')
def home():
    return "Bus Tracking API Running"

# ESP32 sends data
@app.route('/update', methods=['POST'])
def update_location():
    data = request.json

    bus_id = data.get("bus_id")
    lat = data.get("lat")
    lon = data.get("lon")
    speed = data.get("speed", 0)

    bus_data[bus_id] = {
        "bus_id": bus_id,
        "lat": lat,
        "lon": lon,
        "speed": speed,
        "timestamp": int(time.time())
    }

    return jsonify({"status": "success"})

# Website fetches all buses
@app.route('/buses', methods=['GET'])
def get_buses():

    # If live data exists → return it
    if bus_data:
        return jsonify({
            "buses": list(bus_data.values())
        })

    # 🔥 Fallback to Google Sheets
    try:
        response = requests.get(GOOGLE_SHEET_URL)
        data = response.json()

        return jsonify({
            "buses": [
                {
                    "bus_id": "BUS_01",
                    "lat": data["lat"],
                    "lon": data["lon"],
                    "status": "FROM_SHEETS"
                }
            ]
        })

    except:
        # Final fallback (default location)
        return jsonify({
            "buses": [
                {
                    "bus_id": "BUS_01",
                    "lat": 12.9716,
                    "lon": 79.1595,
                    "status": "NO_DATA"
                }
            ]
        })

# Single bus
@app.route('/bus/<bus_id>', methods=['GET'])
def get_bus(bus_id):
    if bus_id in bus_data:
        return jsonify(bus_data[bus_id])
    return jsonify({"error": "Bus not found"}), 404


if __name__ == '__main__':
    app.run()
