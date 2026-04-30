//
// Created by malan on 30.04.2026.
//
#pragma once
#include <memory>
#include <vector>

#include "Interfaces/IObject.hpp"


namespace Engine {
struct SceneObjectJson {
    std::vector<SceneObjectJson> children;
    std::string name;
};

struct SceneJson {

};

class SceneObject {
    std::shared_ptr<IObject> instance;
    std::shared_ptr<SceneObject> Parent;
    std::vector<std::shared_ptr<SceneObject>> Children;
};
class Scene {

};
}
