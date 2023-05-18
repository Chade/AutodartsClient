#ifndef AutodartsDefines_h_
#define AutodartsDefines_h_

#define LOG_FORMATTING LOG_FORMATTING_NOTIME
#define LOG_LEVEL LOG_LEVEL_DEBUG
#include <EasyLogger.h>

SET_LOOP_TASK_STACK_SIZE(16*1024); // 16KB

namespace autodarts {

  class Board;

  struct Status {
    enum class Code : int8_t {
      UNKNOWN          =  -1,
      STOPPED          =   0,
      STARTING         =   2,
      THROW            =   8,
      TAKEOUT          =   16,
      TAKEOUT_PROGRESS =   32,
    };

    Status(Code value) {
      _value = value;
    }

    Status(int8_t value) {
      _value = static_cast<Code>(value);
    }

    Status& operator=(const Code& value) {
      _value = value;
      return *this;
    }

    Status& operator=(int8_t value) {
      _value = static_cast<Code>(value);
      return *this;
    }

    Code value() const  {
      return _value;
    }

    void value(Code value) {
      _value = value;
    }

    static String toString(Code value) {
      switch (value) {
        case Code::STOPPED:          return F("Stopped");
        case Code::STARTING:         return F("Starting");
        case Code::THROW:            return F("Throw");
        case Code::TAKEOUT:          return F("Takeout");
        case Code::TAKEOUT_PROGRESS: return F("Takeout in progress");
        default:                     return F("Unknown");
      }
    }

    String toString() const {
      return toString(_value);
    }

    static Code fromString(const String& value) {
      if      (value == F("Stopped"))             return Code::STOPPED;
      else if (value == F("Starting"))            return Code::STARTING;
      else if (value == F("Throw"))               return Code::THROW;
      else if (value == F("Takeout"))             return Code::TAKEOUT;
      else if (value == F("Takeout in progress")) return Code::TAKEOUT_PROGRESS;
      else                                        return Code::UNKNOWN;
    }

  private:
    Code _value = Code::UNKNOWN;
  };

  struct Event {
    enum class Code : int8_t {
      UNKNOWN          =  -1,
      STOPPED          =   0,
      STOPPING         =   1,
      STARTING         =   2,
      STARTED          =   4,
      THROW_DETECTED   =   8,
      TAKEOUT_STARTED  =  16,
      TAKEOUT_FINISHED =  32,
      RESET            =  64,
    };

    Event(Code value) {
      _value = value;
    }

    Event(int8_t value) {
      _value = static_cast<Code>(value);
    }

    Event& operator=(const Code& value) {
      _value = value;
      return *this;
    }

    Event& operator=(int8_t value) {
      _value = static_cast<Code>(value);
      return *this;
    }

    Code value() const  {
      return _value;
    }

    void value(Code value) {
      _value = value;
    }

    static String toString(Code value) {
      switch (value) {
        case Code::STOPPED:          return F("Stopped");
        case Code::STOPPING:         return F("Stopping");
        case Code::STARTING:         return F("Starting");
        case Code::STARTED:          return F("Started");
        case Code::THROW_DETECTED:   return F("Throw detected");
        case Code::TAKEOUT_STARTED:  return F("Takeout started");
        case Code::TAKEOUT_FINISHED: return F("Takeout finished");
        case Code::RESET:            return F("Manual reset");
        default:                     return F("Unknown");
      }
    }

    String toString() const {
      return toString(_value);
    }

    static Code fromString(const String& value) {
      if      (value == F("Stopped"))             return Code::STOPPED;
      else if (value == F("Stopping"))            return Code::STOPPING;
      else if (value == F("Starting"))            return Code::STARTING;
      else if (value == F("Started"))             return Code::STARTED;
      else if (value == F("Throw detected"))      return Code::THROW_DETECTED;
      else if (value == F("Takeout started"))     return Code::TAKEOUT_STARTED;
      else if (value == F("Takeout finished"))    return Code::TAKEOUT_FINISHED;
      else if (value == F("Manual reset"))        return Code::RESET;
      else                                        return Code::UNKNOWN;
    }

  private:
    Code _value = Code::UNKNOWN;
  };

  enum class State : int8_t {
    TURNED_FALSE = -1,
    IS_FALSE     =  0,
    IS_TRUE      =  1,
    TURNED_TRUE  =  2
  };

  typedef std::function<void(int8_t id, int8_t fps, int16_t width, int16_t height)> CameraStatsCallback;
  typedef std::function<void(State opened, State running)>                          CameraSystemStateCallback;
  typedef std::function<void(State connected, State running, int16_t numThrows)>    DetectionStateCallback;
  typedef std::function<void(Status::Code status, Event::Code event)>               DetectionEventCallback;
  typedef std::function<void(State connected)>                                      BoardConnectionCallback;
  typedef std::function<void(const Board& board)>                                   BoardCallback;

  static const char* AUTODARTS_URL                   = F("https://autodarts.io");
  static const char* AUTODARTS_AUTH_KEYCLOAK_URL     = F("https://login.autodarts.io/realms/autodarts/protocol/openid-connect/token");
  static const char* AUTODARTS_AUTH_KEYCLOAK_REQUEST = F("client_id=autodarts-app&scope=openid&grant_type=password&username=%s&password=%s");
  static const char* AUTODARTS_API_MATCHES_URL       = F("https://api.autodarts.io/gs/v0/matches");
  static const char* AUTODARTS_API_BOARDS_URL        = F("https://api.autodarts.io/bs/v0/boards");
  static const char* AUTODARTS_API_TICKET_URL        = F("https://api.autodarts.io/ms/v0/ticket");
  static const char* AUTODARTS_WS_SECURE_URL         = F("ws://api.autodarts.io/ms/v0/subscribe?ticket=");
  static const char* AUTODARTS_WS_LOCAL_URL          = F("ws://%s/api/events");
};

#endif // AutodartsDefines_h_
