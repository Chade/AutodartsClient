#ifndef AutodartsBoard_h_
#define AutodartsBoard_h_

#include <ArduinoJson.h>

#include <WebSocketsClient.h>
/*
#include <ArduinoWebsockets.h>
*/


#include "AutodartsDefines.h"
#include "AutodartsDetector.h"

namespace autodarts {

  class Board {
  public:
    Board() = delete;

    Board(const JsonObjectConst& json)  {
      fromJson(json);
    };

    Board(const String& name, const String& id, const String& version, const String& url) : 
      _name(name), _id(id), _version(version), _url(url) {

    };

    Board(const String& name, const String& id, const String& version, const IPAddress& address, uint16_t port = 3180) : 
      _name(name), _id(id), _version(version) {
      _url = String("ws://") + address.toString() + ':' + String(port);
    };
    
    String getName() const {
      return _name;
    }

    void setName(const String& name) {
      _name = name;
    }

    String getId() const {
      return _id;
    }

    void setId(const String& id) {
      _id = id;
    }

    String getVersion() const {
      return _version;
    }

    void setVersion(const String& version) {
      _version = version;
    }
    
    String getUrl() const {
      return _url;
    }
    
    void setUrl(const String& url) {
      _url = url;
    }

    bool isOpen() const {
      return _open;
    }

    bool open(bool force = false) {
      // Check if already open
      if (!force && isOpen()) {
        return true;
      }

      // Check if url is valid
      if (_url.isEmpty()) {
        return false;
      }

      // Open websocket
      LOG_DEBUG(_name.c_str(), F("Opening connection"));
      _websocket.begin("192.168.178.89", 3180, "/api/events");

      // Register event callback
      _websocket.onEvent([this](WStype_t type, uint8_t * payload, size_t length) {
        switch(type) {
          case WStype_CONNECTED: {
            LOG_DEBUG(_name.c_str(), F("Connection opened"));
            _open = true;
            _onConnectionChangeCallback(*this);
            break;
          }
          case WStype_DISCONNECTED: {
            LOG_DEBUG(_name.c_str(), F("Connection closed"));
            _open = false;
            _onConnectionChangeCallback(*this);
            break;
          }
          case WStype_TEXT: {
            LOG_DEBUG(_name.c_str(), F("Received data"));
            StaticJsonDocument<2048> json;
            deserializeJson(json, payload);
            _detector.fromJson(json.as<JsonObjectConst>());
            _onDataCallback(*this);
            break;
          }
          case WStype_BIN:
          case WStype_ERROR:		
          case WStype_FRAGMENT_TEXT_START:
          case WStype_FRAGMENT_BIN_START:
          case WStype_FRAGMENT:
          case WStype_FRAGMENT_FIN:
            break;
        }
        resetAlive();
      });

      // try ever 5000 again if connection has failed
      _websocket.setReconnectInterval(5000);

      return true;
    }
/*
    bool open(bool force = false) {
      // Check if already open
      if (!force && isOpen()) {
        return true;
      }

      // Check if url is valid
      if (_url.isEmpty()) {
        return false;
      }

      // Register message callback
      _websocket.onMessage([this](websockets::WebsocketsMessage message) {
        LOG_DEBUG(_name.c_str(), F("Received data"));
        StaticJsonDocument<2048> json;
        deserializeJson(json, message.data());
        _detector.fromJson(json.as<JsonObjectConst>());
        _onDataCallback(*this);
        resetAlive();
      });

      // Register event callback
      _websocket.onEvent([this](websockets::WebsocketsEvent event, String data) {
        if(event == websockets::WebsocketsEvent::ConnectionOpened) {
            LOG_DEBUG(_name.c_str(), F("Connection opened"));
            _open = true;
            _onConnectionChangeCallback(*this);
        } else if(event == websockets::WebsocketsEvent::ConnectionClosed) {
            LOG_DEBUG(_name.c_str(), F("Connection closed"));
            _open = false;
            _onConnectionChangeCallback(*this);
        } else if(event == websockets::WebsocketsEvent::GotPing) {
            LOG_DEBUG(_name.c_str(), F("Ping"));
        } else if(event == websockets::WebsocketsEvent::GotPong) {
            LOG_DEBUG(_name.c_str(), F("Pong"));
        }
        resetAlive();
      });

      // Open websocket
      char websocketUrl[48];
      sprintf(websocketUrl, AUTODARTS_WS_LOCAL_URL, _url.c_str());
      return _websocket.connect(websocketUrl);
    }
*/

    void close() {
      _websocket.disconnect();
/*
      _websocket.close();
*/
      _open = false;
    }

    bool update() {
      _websocket.loop();
/*
      if(_websocket.available() && _websocket.poll()) {
        return true;
      }
*/

      if (isOpen() && !isAlive()) {
        LOG_ERROR(_name.c_str(), F("Connection timeout!"));
        close();
      }

      return false;
    }

    bool isAlive() const {
      return (millis() - _lastAlive) < 10000;
    }

    void resetAlive() {
      _lastAlive = millis();
    }

    void fromJson(const JsonObjectConst& root) {
      _id      = String(root[F("id")].as<const char*>());
      _name    = String(root[F("name")].as<const char*>());
      _url     = String(root[F("ip")].as<const char*>());
      _version = String(root[F("version")].as<const char*>());
    }

    void toJson(JsonObject& root) const {
      root[F("id")]      = _id.c_str();
      root[F("name")]    = _name.c_str();
      root[F("ip")]      = _url.c_str();
      root[F("version")] = _version.c_str();
    }

    void onData(BoardCallback callback) {
      _onDataCallback = callback;
    }

    void onConnectionChange(BoardCallback callback) {
      _onConnectionChangeCallback = callback;
    }

    void onCameraStats(CameraStatsCallback callback) {
      _onCameraStatsCallback = callback;
    }

    void onCameraSystemState(CameraSystemStateCallback callback) {
      _onCameraSystemStateCallback = callback;
    }

    void onDetectionState(DetectionStateCallback callback) {
      _onDetectionStateCallback = callback;
    }

    void onDetectionEvent(DetectionEventCallback callback) {
      _onDetectionEventCallback = callback;
    }

  private:
    String _name = "";
    String _id = "";
    String _url = "";
    String _version = "";
    bool _open = false;
    uint64_t _lastAlive = 0;
    Detector _detector;


    WebSocketsClient _websocket;
/*
    websockets::WebsocketsClient _websocket;
*/

    BoardCallback             _onDataCallback              = [this](const Board&){};
    BoardCallback             _onConnectionChangeCallback  = [this](const Board&){};
    CameraStatsCallback       _onCameraStatsCallback       = [this](int8_t, int8_t, int16_t, int16_t){};
    CameraSystemStateCallback _onCameraSystemStateCallback = [this](State, State){};
    DetectionStateCallback    _onDetectionStateCallback    = [this](State, State,  int16_t){};
    DetectionEventCallback    _onDetectionEventCallback    = [this](Status::Code, Event::Code){};
  };
} // autodarts


#endif // AutodartsBoard_h_
