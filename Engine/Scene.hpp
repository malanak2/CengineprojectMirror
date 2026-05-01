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
    json data;
};

struct SceneJson {

};

    // nullptr Parent means its in root - multiple objects can be that
class SceneObject {
    std::shared_ptr<IObject> instance;
    std::shared_ptr<SceneObject> Parent;
    std::vector<std::shared_ptr<SceneObject>> Children;
};
class Scene {
Scene(std::string path, std::map<ENGINE_COMPONENT_TYPE,void(json js)> constructors);
};
}
