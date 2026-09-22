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
#define IMGUI_TEXT_INPUT(title, deftext)                                       \
  [] {                                                                         \
    static std::string text = deftext;                                         \
    ImGui::InputText(title, &text[0], 255);                                    \
    return text;                                                               \
  }()
