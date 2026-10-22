#pragma once

#include <implot.h>
#include "Interfaces/IComponent.hpp"
#include "Interfaces/IIdentifiable.hpp"
namespace Editor {
    class ComponentTestTrigger : public Engine::IComponent {
        REGISTER_CLASS(ComponentTestTrigger);

    public:
        virtual ~ComponentTestTrigger() override = default;
        json ToJson() override { return {}; }

        void FromJson(json &js) override {}
        void Setup() override;
        void End() override;
    };
} // namespace Editor
