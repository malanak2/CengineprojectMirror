#pragma once

#include <meta>
#include <string_view>
#include <type_traits>

namespace Util
{

  template <typename E>
  requires std::is_enum_v<E>
  constexpr std::string_view enum_to_string(E value) {
    template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
      if (value == [:e:]) {
        return std::meta::identifier_of(e);
      }
    }
    
    return "Unknown";
  }
}