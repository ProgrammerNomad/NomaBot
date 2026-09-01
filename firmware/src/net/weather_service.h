#pragma once

#include <string>

class CharacterRuntime;

class WeatherService {
public:
  void begin(CharacterRuntime *runtime);
  void tick();
  bool lastSuccess() const { return _lastSuccess; }
  bool isStale() const;
  float lastTempC() const { return _lastTempC; }
  const char *lastIcon() const { return _lastIcon.c_str(); }

private:
  bool fetchWeather();

  CharacterRuntime *_runtime = nullptr;
  unsigned long _lastFetchMs = 0;
  unsigned long _lastSuccessMs = 0;
  bool _lastSuccess = false;
  float _lastTempC = 0.0f;
  std::string _lastIcon;
};
