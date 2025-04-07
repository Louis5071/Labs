from flask import Flask, request
import json
import logging

log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)  # Only log errors, not every request

app = Flask(__name__)

@app.route("/data", methods=["POST"])
def receive_data():
    data = request.get_json()
    if data:
        print(f"Received Data -> Temperature: {data['temperature']}°C, Humidity: {data['humidity']}%")
        return {"status": "success", "message": "Data received"}, 200
    return {"status": "error", "message": "Invalid data"}, 400

if __name__ == "__main__":    
    app.run(host="0.0.0.0", port=5000)
