#include "text_scene_renderer.h"

#include <stdio.h>
#include <string.h>

void TextSceneRenderer::render(IRenderer &renderer, const char *lifeMode, const char *activity,
                               const char *emotion, const char *goal, int goalProgress, int energy,
                               bool curiosity, const char *behaviorLabel, const char *messageText) {
  renderer.fillScreen(0x0000);

  char header[48];
  snprintf(header, sizeof(header), "%s · %s", lifeMode ? lifeMode : "work",
           activity ? activity : "idle");
  renderer.drawText(4, 8, header, 0xFFFF);

  char meta[48];
  if (goal && goal[0] && strcmp(goal, "none") != 0) {
    snprintf(meta, sizeof(meta), "%s · %s · %d%%", emotion ? emotion : "neutral", goal,
             goalProgress);
  } else if (emotion && emotion[0] && strcmp(emotion, "neutral") != 0) {
    snprintf(meta, sizeof(meta), "%s", emotion);
  } else {
    meta[0] = '\0';
  }
  if (meta[0]) {
    renderer.drawText(4, 24, meta, 0xAD55);
  }

  char energyLine[24];
  snprintf(energyLine, sizeof(energyLine), "Energy: %d", energy);
  renderer.drawText(4, 40, energyLine, 0x7BEF);

  if (curiosity) {
    renderer.drawText(4, 56, "I wonder...", 0xFD20);
  } else if (behaviorLabel && behaviorLabel[0]) {
    renderer.drawText(4, 56, behaviorLabel, 0xFFFF);
  }

  if (messageText && messageText[0]) {
    renderer.drawText(4, renderer.height() - 20, messageText, 0xFFFF);
  }
}
