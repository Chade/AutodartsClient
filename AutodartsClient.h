#include <memory>
#ifndef AutodartsClient_h_
#define AutodartsClient_h_

#include <StreamUtils.h>

#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

#include "AutodartsDefines.h"
#include "AutodartsBoard.h"

namespace autodarts {

  class Client {

  public:
    typedef std::pair<String, unsigned long> Token;
    typedef std::unique_ptr<Board> BoardPtr;
    typedef std::vector<BoardPtr> BoardArray;

    void addBoard(const JsonObjectConst& json)  {
      BoardPtr board(new Board(json));
      board->onData(_onDataCallback);
      board->onConnectionChange(_onConnectionChangeCallback);
      board->onCameraStats(_onCameraStatsCallback);
      board->onCameraSystemState(_onCameraSystemStateCallback);
      board->onDetectionState(_onDetectionStateCallback);
      board->onDetectionEvent(_onDetectionEventCallback);
      _boards.push_back(std::move(board));
    };

    void addBoard(const String& name, const String& id, const String& version, const String& url) {
      BoardPtr board(new Board(name, id, version, url));
      board->onData(_onDataCallback);
      board->onConnectionChange(_onConnectionChangeCallback);
      board->onCameraStats(_onCameraStatsCallback);
      board->onCameraSystemState(_onCameraSystemStateCallback);
      board->onDetectionState(_onDetectionStateCallback);
      board->onDetectionEvent(_onDetectionEventCallback);
      _boards.push_back(std::move(board));
    };

    void addBoard(const String& name, const String& id, const String& version, const IPAddress& address, uint16_t port = 3180) {
      BoardPtr board(new Board(name, id, version, address, port));
      board->onData(_onDataCallback);
      board->onConnectionChange(_onConnectionChangeCallback);
      board->onCameraStats(_onCameraStatsCallback);
      board->onCameraSystemState(_onCameraSystemStateCallback);
      board->onDetectionState(_onDetectionStateCallback);
      board->onDetectionEvent(_onDetectionEventCallback);
      _boards.push_back(std::move(board));
    }

    void addBoard(BoardPtr& board) {
      board->onData(_onDataCallback);
      board->onConnectionChange(_onConnectionChangeCallback);
      board->onCameraStats(_onCameraStatsCallback);
      board->onCameraSystemState(_onCameraSystemStateCallback);
      board->onDetectionState(_onDetectionStateCallback);
      board->onDetectionEvent(_onDetectionEventCallback);
      _boards.push_back(std::move(board));
    }

    void deleteBoard(int8_t idx) {
      if (idx < _boards.size()) {
        _boards.erase(_boards.begin() + idx);
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Index out of bounds!"));
      }

    }

    void printBoard(uint8_t idx) const {
      if (idx < _boards.size()) {
        LOG_INFO(_boards[idx]->getName().c_str(), F("Id: ") << _boards[idx]->getId() << F(" Url: ") << _boards[idx]->getUrl() << F(" Version: ") << _boards[idx]->getVersion());
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Index out of bounds!"));
      }
    }

    void printBoards() const {
      for (uint8_t idx = 0; idx < _boards.size(); idx++) {
          printBoard(idx);
      }
    }

    bool openBoard(uint8_t idx, bool force = false) const {
      if (idx < _boards.size()) {
        if (!_boards[idx]->open(force)) {
          LOG_ERROR(__FUNCTION__, F("Could not open board: Name: ") << _boards[idx]->getName() << F(" Id: ") << _boards[idx]->getId() << F(" Url: ") << _boards[idx]->getUrl());
          return false;
        }
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Index out of bounds!"));
        return false;
      }
      return true;
    }

    void openBoards(bool force = false) const {
      for (uint8_t idx = 0; idx < _boards.size(); idx++) {
          openBoard(idx, force);
      }
    }

    bool updateBoard(uint8_t idx) const {
      if (idx < _boards.size()) {
        return _boards[idx]->update();
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Index out of bounds!"));
        return false;
      }
      return true;
    }

    void updateBoards() const {
      for (uint8_t idx = 0; idx < _boards.size(); idx++) {
        updateBoard(idx);
      }
    }

    int autoDetectBoards(const String& username, const String& password) {
      // Get access token to connect to autodarts.io account
      int ret = requestAccessToken(username, password, _accessToken);
      if (ret != HTTP_CODE_OK) {
        LOG_ERROR(__FUNCTION__, F("Could not get token to connect to autodarts.io"));
        return ret;
      }

      // Get boards from autodarts.io account
      ret = requestBoards(_boards, _accessToken);
      if (ret != HTTP_CODE_OK) {
        LOG_ERROR(__FUNCTION__, F("Could not get all boards from autodarts.io"));
        return ret;
      }
      
      return ret;
    }

    int refreshBoards(const String& username, const String& password, uint64_t everyMillis) {
      if (_lastChecked > 0 && (millis() - _lastChecked) < everyMillis) {
        return HTTP_CODE_NOT_MODIFIED;
      }
      
      _lastChecked = millis();
      return autoDetectBoards(username, password);
    }
/*
    bool connect(const String& username, const String& password) {
      if (requestAccessToken(username, password) != HTTP_CODE_OK) {
        return false;
      }
      if (requestTicket()!= HTTP_CODE_OK) {
        return false;
      }
      
      bool connected = _websocket.connect(AUTODARTS_WS_SECURE_URL + _ticket);
      if (!connected) {
        LOG_ERROR("connect", "Could not connect to websocket");
      }

      _websocket.onMessage([this](websockets::WebsocketsMessage message) {
        DynamicJsonDocument json(1024);
        deserializeJson(json, message.data());
        serializeJsonPretty(json, Serial);
      });

      _websocket.onEvent([this](websockets::WebsocketsEvent event, String data) {
        if(event == websockets::WebsocketsEvent::ConnectionOpened) {
            Serial.print("Connnection Opened: ");
        } else if(event == websockets::WebsocketsEvent::ConnectionClosed) {
            Serial.print("Connnection Closed: ");
        } else if(event == websockets::WebsocketsEvent::GotPing) {
            Serial.print("Got a Ping: ");
        } else if(event == websockets::WebsocketsEvent::GotPong) {
            Serial.print("Got a Pong: ");
        }
      });
      return connected;
    }
*/
    int requestAccessToken(const String& username, const String& password, Token& accessToken, bool forceUpdate = false) const {
      // Check if token is still valid    
      if (!forceUpdate && accessToken.second > millis()) {
        return HTTP_CODE_OK;
      }

      // Assemble keycloak request
      char request[256];
      sprintf(request, AUTODARTS_AUTH_KEYCLOAK_REQUEST, username.c_str(), password.c_str());

      // Send POST to keycloak to retrieve access token
      HTTPClient httpClient;
      httpClient.useHTTP10(true);
      httpClient.begin(AUTODARTS_AUTH_KEYCLOAK_URL);
      httpClient.addHeader(F("Content-Type"), F("application/x-www-form-urlencoded"));
      int ret = httpClient.POST(request);
      
      if (ret == HTTP_CODE_OK) {
        // Prepare filter   
        DynamicJsonDocument filter(32);
        filter[F("access_token")] = true;
        filter[F("expires_in")] = true;
        
        // Read json from stream
        DynamicJsonDocument doc(4096);
        deserializeJson(doc, httpClient.getStream(), DeserializationOption::Filter(filter));
        
        accessToken.first = String(doc[F("access_token")].as<const char*>());
        accessToken.second = millis() + doc[F("expires_in")].as<unsigned int>() * 1000;
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Could not retrieve access token [") << ret << F("]: ") << httpClient.getString());
      }
      
      httpClient.end();
      return ret;
    }

    int requestTicket(String& ticket, const Token& accessToken) const {
      // Check if input data is avialable
      if (accessToken.first.isEmpty() || accessToken.second < millis()) {
        LOG_ERROR(__FUNCTION__, F("Access token is invalid!"));
        return HTTP_CODE_UNAUTHORIZED;
      }

      // Send POST to retrieve ticket
      HTTPClient httpClient;
      httpClient.useHTTP10(true);
      httpClient.begin(AUTODARTS_API_TICKET_URL);
      httpClient.addHeader(F("Authorization"), F("Bearer ") + accessToken.first);
      int ret = httpClient.POST("");
      
      if (ret == HTTP_CODE_OK) {
        ticket = httpClient.getString();
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Could not retrieve ticket [") << ret << F("]: ") << httpClient.getString());
      }
      
      httpClient.end();
      return ret;
    }

    int requestBoards(BoardArray& boards, const Token& accessToken) {
      // Check if input data is avialable
      if (accessToken.first.isEmpty() || accessToken.second < millis()) {
        LOG_ERROR(__FUNCTION__, F("Access token is invalid!"));
        return HTTP_CODE_UNAUTHORIZED;
      }

      // Send POST to retrieve boards
      HTTPClient httpClient;
      httpClient.useHTTP10(true);
      httpClient.begin(AUTODARTS_API_BOARDS_URL);
      httpClient.addHeader(F("Authorization"), F("Bearer ") + accessToken.first);
      int ret = httpClient.GET();
      
      if (ret == HTTP_CODE_OK) {
        // Prepare filter      
        DynamicJsonDocument filter(80);
        filter[F("id")] = true;
        filter[F("name")] = true;
        filter[F("ip")] = true;
        filter[F("version")] = true;

        // Read json from stream in chunks
        httpClient.getStream().find('[');
        do {
          DynamicJsonDocument doc(1024);
          DeserializationError err = deserializeJson(doc, httpClient.getStream(), DeserializationOption::Filter(filter));        
          if (err) {
            LOG_ERROR(__FUNCTION__, F("Could not deserialize board information: ") << err.c_str());
            continue;
          }

          bool found = false;
          const char* id = doc["id"];
          // Check if board with the given id is already present
          for (BoardPtr& board : boards) {
            // If board already exists, only update data
            if (board->getId().equals(id)) {
              LOG_INFO(__FUNCTION__, F("Found an existing board [") << board->getName() << F("][") << board->getId() << F("]"));
              board->fromJson(doc.as<JsonObject>());
              found = true;
              break;
            }
          }
          // If no board with the given id is found add a new one
          if (!found) {
            BoardPtr board(new Board(doc.as<JsonObject>()));
            if (board->getUrl().isEmpty()) {
              LOG_WARNING(__FUNCTION__, F("Skipping board with empty url [") << board->getName() << F("][") << board->getId() << F("]"));
            }
            else {
              LOG_INFO(__FUNCTION__, F("Found a new board [")  << board->getName() << F("][") << board->getId() << F("]"));
              addBoard(board);
            }
          }
        } while (httpClient.getStream().findUntil(",", "]"));
      }
      else {
        LOG_ERROR(__FUNCTION__, F("Could not retrieve boards [") << ret << F("]: ") << httpClient.getString());
      }
      
      httpClient.end();
      return ret;
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
    String _ticket;
    Token _accessToken;
    BoardArray _boards;
    uint64_t _lastChecked = 0;

    BoardCallback             _onDataCallback              = [](const Board&){};
    BoardCallback             _onConnectionChangeCallback  = [](const Board&){};
    CameraStatsCallback       _onCameraStatsCallback       = [](int8_t, int8_t, int16_t, int16_t){};
    CameraSystemStateCallback _onCameraSystemStateCallback = [](State, State){};
    DetectionStateCallback    _onDetectionStateCallback    = [](State, State,  int16_t){};
    DetectionEventCallback    _onDetectionEventCallback    = [](Status::Code, Event::Code){};
  };

} // autodarts

#endif // AutodartsClient_h_
