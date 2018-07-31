#pragma once

#include <render_pipeline/rppanda/showbase/direct_object.hpp>
#include <crsf/CRAPI/TDynamicModuleInterface.h>

namespace rpcore {
class RenderPipeline;
}

namespace crsf {
class TGraphicRenderEngine;
class TActorObject;
class TAvatarMemoryObject;
}

class OpenVRManager;

class MainApp : public crsf::TDynamicModuleInterface, public rppanda::DirectObject
{
public:
    MainApp();

    void OnLoad() override;
    void OnStart() override;
    void OnExit() override;

    void setup_avatar();
    void setup_chair();
    void setup_gui();

    void update();

private:
    void on_imgui_new_frame();

    crsf::TGraphicRenderEngine* rendering_engine_;
    rpcore::RenderPipeline* pipeline_;

    std::unique_ptr<OpenVRManager> openvr_manager_;

    std::vector<std::shared_ptr<crsf::TActorObject>> actors_;
    crsf::TActorObject* current_actor_ = nullptr;
};
