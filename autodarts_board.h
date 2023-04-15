#ifndef AutodartsBoard_h_
#define AutodartsBoard_h_

namespace autodarts {

enum class status : int8_t {
  UNKNOWN  =  -1,
  STOPPED  =   0,
  STOPPING =   1,
  STARTING =   2,
  STARTED  =   4,
  THROW    =   8,
  TAKEOUT  =  16,
  ERROR    = 127,
}

class camera {
public:
  int8_t getId() {
    return _id;
  }

  void setId(int8_t id) {
    _id = id;
  }

  int8_t getFPS() {
    return _fps;
  }

  void setFPS(int8_t fps) {
    _fps = fps;
  }

  int16_t getWidth() {
    return _width;
  }

  void setWidth(int16_t width) {
    _width = width;
  }

  int16_t getHeight() {
    return _height;
  }

  void setHeight(int16_t height) {
    _height = height;
  }

private:
  int8_t  _id = -1;
  int8_t  _fps = -1;
  int16_t _width = -1;
  int16_t _height = -1;
}


class cameras {
public:
  bool isOpen() {
    return _open;
  }

  void open(bool state) {
    _open = state;
  }

  bool isRunning() {
    return _running;
  }

  void running(bool state) {
    _running = state;
  }

  camera* getCamera(int8_t id) {
    for (uint8_t idx = 0; i < 3; i++) {
      if (_camera[idx].getId() == id) {
        return _camera[idx];
      }
    }
    return nullptr;
  }

  camera& operator [](uint8_t idx) {
    return _camera[idx];
  }

  const camera& operator [](uint8_t idx) const {
    return _camera[idx];
  }


private:
  camera _camera[3];
  bool   _open = false;
  bool   _running = false;

}

class board {
public:
  board() = delete;
  board(std::string name) : _name(name) {

  };
  

private:
  std::string _name = "";
  std::string _url = "";
  int8_t _connected = -1,
  int8_t _running = -1,
  int32_t _numThrows = -1;
  cameras _cameras;
  status state = status::UNKNOWN;
  status event = status::UNKNOWN;
}


} // autodarts


#endif // AutodartsBoard_h_
