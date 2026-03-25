#API for website

from flask import Flask, request, jsonify
from flask_cors import CORS
import time

app = Flask(__name__)
CORS(app)

bus_data = {}

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
    return jsonify({
        "buses": list(bus_data.values())
    })

# Single bus
@app.route('/bus/<bus_id>', methods=['GET'])
def get_bus(bus_id):
    if bus_id in bus_data:
        return jsonify(bus_data[bus_id])
    return jsonify({"error": "Bus not found"}), 404


if __name__ == '__main__':
    app.run()