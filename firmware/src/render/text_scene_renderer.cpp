#include "text_scene_renderer.h"

#include <stdio.h>
#include <string.h>

void TextSceneRenderer::render(IRenderer &renderer, const char *lifeMode, const char *activity,
                               const char *emotion, const char *behaviorLabel,
                               const char *messageText) {
  renderer.fillScreen(0x0000);

  char header[48];
  snprintf(header, sizeof(header), "%s · %s", lifeMode ? lifeMode : "work",
           activity ? activity : "idle");
  renderer.drawText(4, 8, header, 0xFFFF);

  if (emotion && emotion[0] && strcmp(emotion, "neutral") != 0) {
    renderer.drawText(4, 24, emotion, 0xAD55);
  }

  if (behaviorLabel && behaviorLabel[0]) {
    renderer.drawText(4, 40, behaviorLabel, 0xFFFF);
  }

  if (messageText && messageText[0]) {
    renderer.drawText(4, renderer.height() - 20, messageText, 0xFFFF);
  }
}
