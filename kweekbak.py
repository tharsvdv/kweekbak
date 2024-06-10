import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
import json

# InfluxDB configurations
INFLUXDB_ADDRESS = '192.168.0.129'
INFLUXDB_USER = 'thars'
INFLUXDB_PASSWORD = 'thars'
INFLUXDB_DATABASE = 'kweekbak'

# MQTT configurations
MQTT_BROKER = "tharspi.local"
MQTT_PORT = 1883
MQTT_TOPIC = "esp32/#"
MQTT_USER = "thars"
MQTT_PASSWORD = "thars"

# Initialize InfluxDB client
influx_client = InfluxDBClient(
    INFLUXDB_ADDRESS,
    8086,
    INFLUXDB_USER,
    INFLUXDB_PASSWORD,
    INFLUXDB_DATABASE
)

# Verify that we can connect to InfluxDB
try:
    influx_client.create_database(INFLUXDB_DATABASE)
    print(f"Connected to InfluxDB at {INFLUXDB_ADDRESS}")
except Exception as e:
    print(f"Error connecting to InfluxDB: {e}")

def on_connect(client, userdata, flags, rc):
    print(f"Connected to MQTT Broker with result code {rc}")
    if rc == 0:
        print("Connection successful")
    else:
        print("Connection failed")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode()
        print(f"Received `{payload}` from `{msg.topic}` topic")
        data = json.loads(payload)
        print(f"Decoded JSON: {data}")

        json_body = [
            {
                "measurement": "sensor_data",
                "tags": {
                    "device": "esp32"
                },
                "fields": data
            }
        ]
        print(f"Writing data to InfluxDB: {json_body}")
        success = influx_client.write_points(json_body)
        if success:
            print("Data written to InfluxDB successfully")
        else:
            print("Failed to write data to InfluxDB")
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error: {e}")

# Initialize MQTT client
mqtt_client = mqtt.Client()
mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

# Connect to MQTT broker
mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start the MQTT client loop
mqtt_client.loop_forever()
