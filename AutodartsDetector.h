#include "WString.h"
#ifndef AutodartsDetector_h_
#define AutodartsDetector_h_

#include <ArduinoJson.h>

#include "AutodartsDefines.h"
#include "AutodartsCameras.h"

namespace autodarts {

  class Detector {
  public:
    bool isConnected() const {
      return _isConnected;
    }

    bool isRunning() const {
      return _isRunning;
    }

    int16_t getNumThrows() const {
      return _numThrows;
    }

    Status getStatus() const {
      return _status;  
    }

    Event getEvent() const {
      return _event;
    }

    CameraSystem& getCameraSystem() {
      return _cameraSystem;
    }

    void fromJson(const JsonObjectConst& root) {
      if (root["type"] == "state") {
        _wasConnected = _isConnected;
        _wasRunning   = _isRunning;

        _isConnected = root["data"]["connected"];
        _isRunning   = root["data"]["running"];
        _numThrows   = root["data"]["numThrows"];
        _status.fromString(root["data"]["status"]);
        _event.fromString(root["data"]["event"]);

        State connected = static_cast<State>(2*_isConnected - _wasConnected);
        State running   = static_cast<State>(2*_isRunning   - _wasRunning);
        _onDetectionStateCallback(connected, running, _numThrows);
        _onDetectionEventCallback(_status.value(), _event.value());
      }
      else {
        _cameraSystem.fromJson(root);
      }
    }

    void toJson(JsonObject& root) const {
      JsonObject data = root.createNestedObject("data");
      data["connected"] = _isConnected;
      data["running"]   = _isRunning;
      data["status"]    = _status.toString();
      data["event"]     = _event.toString();
      data["numThrows"] = _numThrows;
      root["type"]      = "state";
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
    CameraSystem _cameraSystem;
    
    bool _isConnected = false;
    bool _isRunning = false;
    bool _wasConnected = false;
    bool _wasRunning = false;
    int16_t _numThrows = -1;

    Status _status = Status::Code::UNKNOWN;
    Event _event = Event::Code::UNKNOWN;

    CameraStatsCallback       _onCameraStatsCallback       = [](int8_t, int8_t, int16_t, int16_t){};
    CameraSystemStateCallback _onCameraSystemStateCallback = [](State, State){};
    DetectionStateCallback    _onDetectionStateCallback    = [](State, State,  int16_t){};
    DetectionEventCallback    _onDetectionEventCallback    = [](Status::Code, Event::Code){};
  };


} // autodarts


#endif // AutodartsDetector_h_
