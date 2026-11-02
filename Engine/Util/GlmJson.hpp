#pragma once
#include "Util/LoggerUtil.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

namespace glm { // must be namespace glm so nlohmann finds these via ADL

  namespace detail_json {
    template <class V>
    bool read_floats(const nlohmann::json &j, V &out, int n) {
      bool ok = j.is_array() && (int)j.size() == n;
      for (int i = 0; ok && i < n; ++i)
        ok = j[i].is_number();
      if (!ok) {
        SPDLOG_LOGGER_WARN(ENGINE_UTIL_LOGGER,
                           "Expected array of {} numbers, got {}", n, j.dump());
        return false; // caller keeps the default value
      }
      for (int i = 0; i < n; ++i)
        out[i] = j[i].get<float>();
      return true;
    }
  } // namespace detail_json

  inline void to_json(nlohmann::json &j, const vec2 &v) { j = nlohmann::json::array({v.x, v.y}); }
  inline void to_json(nlohmann::json &j, const vec3 &v) { j = nlohmann::json::array({v.x, v.y, v.z}); }
  inline void to_json(nlohmann::json &j, const vec4 &v) { j = nlohmann::json::array({v.x, v.y, v.z, v.w}); }
  inline void from_json(const nlohmann::json &j, vec2 &v) { detail_json::read_floats(j, v, 2); }
  inline void from_json(const nlohmann::json &j, vec3 &v) { detail_json::read_floats(j, v, 3); }
  inline void from_json(const nlohmann::json &j, vec4 &v) { detail_json::read_floats(j, v, 4); }

  // quat stored [w,x,y,z], same layout as Object's "rotation"
  inline void to_json(nlohmann::json &j, const quat &q) {
    j = nlohmann::json::array({q.w, q.x, q.y, q.z});
  }
  inline void from_json(const nlohmann::json &j, quat &q) {
    glm::vec4 t;
    if (!detail_json::read_floats(j, t, 4))
      return;
    q = glm::normalize(quat(t[0], t[1], t[2], t[3]));
  }
} // namespace glm