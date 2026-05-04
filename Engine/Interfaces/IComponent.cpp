#include "IComponent.hpp"

namespace Engine {
std::map<ENGINE_COMPONENT_TYPE, void(json &js)> constructors = {};
}
