#ifndef AutodartsCameras_h_
#define AutodartsCameras_h_

#include <ArduinoJson.h>

#include "AutodartsDefines.h"

namespace autodarts {
  class Camera {
  public:
    int8_t getId() const {
      return _id;
    }

    int8_t getFPS() const {
      return _fps;
    }

    int16_t getWidth() const {
      return _width;
    }

    int16_t getHeight() const {
      return _height;
    }

    void fromJson(const JsonObjectConst& root) {
      if (root["type"] == "cam_stats") {
        _id     = root["data"]["id"];
        _fps    = root["data"]["fps"];
        _width  = root["data"]["resolution"]["width"];
        _height = root["data"]["resolution"]["height"];
        _onCameraStatsCallback(_id, _fps, _width, _height);
      }
      else {
        LOG_WARNING("Camera", F("Unknown message type:") << root["type"].as<String>());
      }
    }

    void toJson(JsonObject& root) const {
      JsonObject data = root.createNestedObject("data");
      data["id"] = _id;
      data["fps"] = _fps;
      data["resolution"]["width"] = _width;
      data["resolution"]["height"] = _height;
      root["type"] = "cam_stats";
    }

    void onCameraStats(CameraStatsCallback callback) {
      _onCameraStatsCallback = callback;
    }

  private:
    int8_t  _id = -1;
    int8_t  _fps = -1;
    int16_t _width = -1;
    int16_t _height = -1;

    CameraStatsCallback _onCameraStatsCallback = [](int8_t, int8_t, int16_t, int16_t){};
  };


  class CameraSystem {
  public:
    bool isOpen() const {
      return _isOpened;
    }

    bool isRunning() const {
      return _isRunning;
    }

    uint8_t getNumCameras() const {
      return _cameras.size();
    }

    Camera* getCameraById(int8_t id) {
      for (auto it = _cameras.begin(); it < _cameras.end(); it++) {
        if (it->getId() == id) {
          return it;
        }
      }
      return nullptr;
    }

    Camera& operator [](uint8_t idx) {
      return _cameras[idx];
    }

    const Camera& operator [](uint8_t idx) const {
      return _cameras[idx];
    }

    void fromJson(const JsonObjectConst& root) {
      if (root["type"] == "cam_state") {
        LOG_DEBUG(__FUNCTION__, "Camera state changed");
        _wasOpened  = _isOpened;
        _wasRunning = _isRunning;
        _isOpened   = root["data"]["isOpened"];
        _isRunning  = root["data"]["isRunning"];
        
        State opened  = static_cast<State>(2*_isOpened  - _wasOpened);
        State running = static_cast<State>(2*_isRunning - _wasRunning);
        _onCameraSystemStateCallback(opened, running);
      }
      else if (root["type"] == "cam_stats") {
        LOG_DEBUG(__FUNCTION__, "Camera stats");
        int8_t id = root["data"]["id"];
        if (id >= 0 && id < _cameras.size()) {
          _cameras[id].fromJson(root);
        }
      }
      else {
        LOG_WARNING("CameraSystem", F("Unknown message type:") << root["type"].as<String>());
      }
    }

    void toJson(JsonObject& root) const {
      JsonObject data = root.createNestedObject("data");
      data["isOpened"]  = _isOpened;
      data["isRunning"] = _isRunning;
      root["type"]      = "cam_state";
    }

    void onCameraStats(CameraStatsCallback callback) {
      _onCameraStatsCallback = callback;
      for (Camera& camera : _cameras) {
          camera.onCameraStats(_onCameraStatsCallback);
      }
    }

    void onCameraSystemState(CameraSystemStateCallback callback) {
      _onCameraSystemStateCallback = callback;
    }

  private:
    std::array<Camera, 3> _cameras;

    bool   _isOpened = false;
    bool   _isRunning = false;
    bool   _wasOpened = false;
    bool   _wasRunning = false;

    CameraStatsCallback       _onCameraStatsCallback       = [](int8_t, int8_t, int16_t, int16_t){};
    CameraSystemStateCallback _onCameraSystemStateCallback = [](State, State){};
  };

} // autodarts


#endif // AutodartsCameras_h_
