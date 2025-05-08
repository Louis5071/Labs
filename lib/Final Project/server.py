from flask import Flask, render_template, request, jsonify
import time
import boto3

app = Flask(__name__)

# AWS SNS configuration
# Ensure your environment has valid AWS credentials (via environment variables or an IAM role).
sns_client = boto3.client('sns', region_name='us-east-2')
SNS_TOPIC_ARN = 'arn:aws:sns:us-east-2:376214823577:Bathroom_Occupancy_Management'


# Global state variables to track occupancy and timing
occupancy = "vacant"          # Current occupancy status ("vacant" or "occupied")
door_close_time = None        # Timestamp when the door sensor first detected closed
last_motion_time = None       # Timestamp when motion was last detected while in occupied state
prev_occupancy = "vacant"     # To detect state changes
queue_requests = []           # List of user identifiers who pressed the "get in line" button

# Time thresholds in seconds
MOTION_DETECTION_WINDOW_START = 2.5  # 2.5 seconds after door closes
MOTION_DETECTION_WINDOW_END = 30     # 30 seconds after door closes
MOTION_TIMEOUT = 15 * 60             # 15 minutes (900 seconds) without motion to revert to vacant

@app.route('/')
def index():
    return render_template('index.html')


@app.route('/update', methods=['POST'])
def update():
    global occupancy, door_close_time, last_motion_time, prev_occupancy, queue_requests
    data = request.get_json()
    if not data:
        return jsonify({"error": "Invalid data"}), 400

    # Interpret sensor readings:
    # Assume: doorState = True when closed, False when open.
    door_state = data.get("doorState")
    motion = data.get("motion")
    button_request = data.get("buttonRequest", False)

    current_time = time.time()

    # If the TTGO button was pressed, register the request.
    # In a complete application, you would use a unique user/device ID.
    if button_request:
        if "user" not in queue_requests:
            queue_requests.append("user")


    # Logic when the door is open: immediately set state to vacant and reset timers.
    if not door_state:
        occupancy = "vacant"
        door_close_time = None
        last_motion_time = None

    else:  # door_state is True, meaning the door is closed.
        # When the door first closes, record the timestamp.
        if door_close_time is None:
            door_close_time = current_time

        # If we are in the waiting window after door closure:
        elapsed_since_door_close = current_time - door_close_time
        # Only consider motion if within the allowed window and not already occupied.
        if elapsed_since_door_close >= MOTION_DETECTION_WINDOW_START and elapsed_since_door_close <= MOTION_DETECTION_WINDOW_END:
            if motion and occupancy != "occupied":
                occupancy = "occupied"
                last_motion_time = current_time
        # After the waiting window, if no occupancy has been set, reset the door_close_time.
        if elapsed_since_door_close > MOTION_DETECTION_WINDOW_END and occupancy != "occupied":
            door_close_time = None

        # If the bathroom is currently occupied, update last_motion_time when motion is detected.
        if occupancy == "occupied":
            if motion:
                last_motion_time = current_time

            # If no motion is detected for more than 15 minutes, revert to vacant.
            if last_motion_time and (current_time - last_motion_time) > MOTION_TIMEOUT:
                occupancy = "vacant"
                door_close_time = None
                last_motion_time = None

    # Optionally log or process button_request if that logic becomes relevant.

    if prev_occupancy == "occupied" and occupancy == "vacant":
        # If someone pressed the button or queued via mobile, fire off a notification:
        if button_request or queue_requests:
            message = "🚽 Bathroom is now available!"
            try:
                # send to an SNS Topic
                sns_client.publish(
                    TopicArn=SNS_TOPIC_ARN,
                    Message=message,
                    Subject="Bathroom Available"
                )
                print("✅ Notification sent.")
            except Exception as e:
                print("❌ SNS publish failed:", e)

            # Clear the queue if you want one-time alerts
            queue_requests.clear()

    # Update for next cycle
    prev_occupancy = occupancy

    print(f"Door State: {door_state}, Motion: {motion}, Occupancy: {occupancy}, Button Pressed: {button_request}")
    return jsonify({"occupancy": occupancy})

    @app.route('/status', methods=['GET'])
def status():
    """
    Returns the current occupancy state of the bathroom.
    """
    return jsonify({"occupancy": occupancy})

@app.route('/queue', methods=['POST'])
def queue():
    """
    This endpoint allows a mobile device to request adding a user into
    the notification queue (e.g., to be notified when the bathroom becomes free).
    (Not modified in this update; functionality remains optional.)
    """
    data = request.get_json()
    if not data or "user" not in data:
        return jsonify({"error": "Invalid request"}), 400
    user = data["user"]
    if user not in queue_requests:
        queue_requests.append(user)    

    return jsonify({"message": f"{user} has been added to the queue"})

if __name__ == '__main__':
    # Run the server on all available IP addresses on port 5000
    app.run(host='0.0.0.0', port=5000)

