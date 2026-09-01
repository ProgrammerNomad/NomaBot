#pragma once

#include <string>

class CharacterRuntime;

class WeatherService {
public:
  void begin(CharacterRuntime *runtime);
  void tick();

private:
  bool fetchWeather();

  CharacterRuntime *_runtime = nullptr;
  unsigned long _lastFetchMs = 0;
  bool _lastSuccess = false;
};
