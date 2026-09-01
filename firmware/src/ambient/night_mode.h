#pragma once

class LilygoRenderer;

class NightModeController {
public:
  void begin(LilygoRenderer *renderer);
  void tick(int currentHour);

private:
  LilygoRenderer *_renderer = nullptr;
  bool _nightActive = false;
};
