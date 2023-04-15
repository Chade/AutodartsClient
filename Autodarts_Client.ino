#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>



const char* ssid = ""; //Enter SSID
const char* password = ""; //Enter Password
const char* websockets_server_host = "192.168.178.89"; //Enter server adress
const uint16_t websockets_server_port = 3180; // Enter server port

using namespace websockets;

SET_LOOP_TASK_STACK_SIZE(16*1024); // 16KB

WebsocketsClient ws_client;
HTTPClient http_client;
WiFiClient wifi_client;

bool connected = false;
bool token = false;

bool connect() {
    http_client.begin("https://login.autodarts.io/realms/autodarts/protocol/openid-connect/token/");
    http_client.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int ret = http_client.sendRequest("POST", "grant_type=password&username=Chade&password=4562dxde&client_id=autodarts-app&scope=openid");
    if (ret == HTTP_CODE_OK) {
      StaticJsonDocument<5000> doc;
      StaticJsonDocument<16> filter;
      filter["access_token"] = true;
      DeserializationError error = deserializeJson(doc, http_client.getStream(), DeserializationOption::Filter(filter));

      serializeJsonPretty(doc, Serial);
    }
    http_client.end();
  
    connected = ws_client.connect("ws://192.168.178.89:3180/api/events");
    if(connected) {
        Serial.println("Connected!");
    } else {
        Serial.println("Not Connected!");
    }

    ws_client.onMessage(onMessageCallback);
    ws_client.onEvent(onEventCallback);

    return connected;
}


void onMessageCallback(WebsocketsMessage message) {
    StaticJsonDocument<1024> json;
    deserializeJson(json, message.data());
    serializeJsonPretty(json, Serial);
    Serial.printf("\nonMessageCallback() - Free Stack Space: %d", uxTaskGetStackHighWaterMark(NULL));
}

void onEventCallback(WebsocketsEvent event, String data) {
    if(event == WebsocketsEvent::ConnectionOpened) {
        Serial.print("Connnection Opened: ");
    } else if(event == WebsocketsEvent::ConnectionClosed) {
        Serial.print("Connnection Closed: ");
    } else if(event == WebsocketsEvent::GotPing) {
        Serial.print("Got a Ping: ");
    } else if(event == WebsocketsEvent::GotPong) {
        Serial.print("Got a Pong: ");
    }
    Serial.println(data);
}



void setup() {
    Serial.begin(115200);
    Serial.printf("Arduino Stack was set to %d bytes", getArduinoLoopTaskStackSize());
    // Connect to wifi
    WiFi.begin(ssid, password);

    // Wait some time to connect to wifi
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }

    // Check if connected to wifi
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }

    Serial.println("Connected to Wifi, Connecting to server.");
    // try to connect to Websockets server

    connect();

    Serial.printf("\nSetup() - Free Stack Space: %d\n", uxTaskGetStackHighWaterMark(NULL));
}

void loop() {
    // let the websockets client check for incoming messages
    if(ws_client.available()) {
        ws_client.poll();
    }
    delay(500);
}