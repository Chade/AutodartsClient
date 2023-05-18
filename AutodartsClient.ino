#include <WiFiManager.h> 
#include "AutodartsClient.h"

bool configured = false;
bool webportal = true;

autodarts::Client client;
WiFiManager wifiManager;
WiFiManagerParameter autodartsUsername("username", "Username", "", 40);
WiFiManagerParameter autodartsPassword("password", "Password", "", 20);


void onDataCallback(const autodarts::Board& board) {
  Serial.println("Received new data");
}

void onConnectionChangeCallback(const autodarts::Board& board) {
  Serial.println("Connection changed");
}

void onSaveWifiParams () {
  if (strlen(autodartsUsername.getValue()) != 0 && strlen(autodartsPassword.getValue()) != 0) {
    LOG_INFO("WifiManager", autodartsUsername.getLabel() << " = " << autodartsUsername.getValue());
    LOG_INFO("WifiManager", autodartsPassword.getLabel() << " = " << autodartsPassword.getValue());
    client.autoDetectBoards(autodartsUsername.getValue(), autodartsPassword.getValue());
    client.openBoards();
  }
}

void setup() {
    Serial.begin(115200);
    
    // Set wifi mode
    WiFi.mode(WIFI_STA);

    wifiManager.addParameter(&autodartsUsername);
    wifiManager.addParameter(&autodartsPassword);
    wifiManager.setSaveParamsCallback(onSaveWifiParams);

    // Automatically connect using saved credentials if they exist
    // If connection fails it starts an access point with the specified name
    if(wifiManager.autoConnect("AutoConnectAP")){
        Serial.println("Wifi connected");
    }
    else {
        Serial.println("Wifi portal running");
    }

    // Start webportal
    wifiManager.startWebPortal();

    client.onData(onDataCallback);
    client.onConnectionChange(onConnectionChangeCallback);
}

void loop() {
  wifiManager.process();
  
  client.updateBoards();
  delay(1);
}