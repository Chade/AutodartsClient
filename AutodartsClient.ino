#include <WiFiManager.h> 
WiFiManager wifiManager;
WiFiManagerParameter autodartsUsername("username", "Username", "", 40);
WiFiManagerParameter autodartsPassword("password", "Password", "", 20);

#include "AutodartsClient.h"
autodarts::Client client;

SET_LOOP_TASK_STACK_SIZE(16*1024); // 16KB

void onDataCallback(const autodarts::Board& board) {
  Serial.println("Received new data");
}

void onConnectionChangeCallback(const autodarts::Board& board) {
  Serial.println("Connection changed");
}

void onCameraSystemStateCallback(autodarts::State opened, autodarts::State running) {
  if (running == autodarts::State::TURNED_TRUE) {
    Serial.println("Cameras started");
  }
  else if(running == autodarts::State::TURNED_FALSE) {
    Serial.println("Cameras stopped");
  }
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
    wifiManager.setConfigPortalBlocking(false);

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
    client.onCameraSystemState(onCameraSystemStateCallback);
}

void loop() {
  wifiManager.process();
  client.updateBoards();
  delay(1);
}