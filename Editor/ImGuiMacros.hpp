#pragma once

#include <imgui.h>

#define IMGUI_CHECKBOX(text, defaultState, onSwitch)                           \
  {                                                                            \
    static bool state = defaultState;                                          \
    static bool oldState = state;                                              \
    if (ImGui::Checkbox(text, &state)) {                                       \
      if (state != oldState) {                                                 \
        onSwitch(state);                                                       \
        oldState = state;                                                      \
      }                                                                        \
    }                                                                          \
  }
