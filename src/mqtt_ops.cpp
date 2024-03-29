#include "mqtt_ops.h"

PubSubClient mqclient;

uint8_t mqtt_on = 0;
String mqtt_broker_address = "";
String mqtt_username = "";
String mqtt_password = "";

String esp_chip_id(ESP.getChipId(), HEX);
String base_topic = String("ingest/system/")+esp_chip_id;
String node_name = String("esp-") + esp_chip_id;

boolean process_connection() {
    if (mqtt_username.length())
        return mqclient.connect(
            node_name.c_str(),
            mqtt_username.c_str(),
            mqtt_password.c_str()
        );
    else return mqclient.connect(node_name.c_str());
}

void mqtt_init() {
    // TODO: Query IAM if MQI token unparsed
    mqtt_broker_address = param::get_mqtt_address();
    mqtt_username = param::get_mqtt_username();
    mqtt_password = param::get_mqtt_password();
}

void mqtt_refresh_state() {
    if (!mqtt_on) {
        mqtt_on = 1;
        mqclient = PubSubClient(mqtt_broker_address.c_str(), 1883, mqtt_callback, wclient);
        if (process_connection()) {
            Serial.print(F("[MQTT] Connected to "));
            Serial.println(mqtt_broker_address);
            Serial.print("Base topic: ");
            Serial.println(base_topic);
            String topic = base_topic;
            String message = "{\"msg\": \"connected\"}";
            mqclient.publish(topic.c_str(), message.c_str());
            mqclient.subscribe((base_topic + "ingest/command/#").c_str());
        } else {
            Serial.println(F("[MQTT] Connection failed"));
            mqtt_on = 0;
        }
    }
    if (mqtt_on && !mqclient.connected()) mqtt_on = 0;
}

void print_mqtt_info() {
    lcd_mqtt(1, mqtt_on, mqtt_broker_address);
}

void mqtt_loop() {
    if (mqtt_on)
        if (!mqclient.loop()) mqtt_on = 0;
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    Serial.print(F("[MQTT] Received message on MQTT, node#"));
    String t(topic);
    uint32_t nodeID = *(uint32_t*) payload;
    Serial.print(nodeID);
    Serial.print(", size ");
    Serial.println(length);
    mesh.write(payload, 121, length, nodeID);
}
