#pragma once

#include <render_pipeline/rppanda/showbase/direct_object.hpp>
#include <crsf/CRAPI/TDynamicModuleInterface.h>

#include <nodePath.h>

namespace rpcore {
class RenderPipeline;
}

namespace crsf {
class TGraphicRenderEngine;
class TActorObject;
class TAvatarMemoryObject;
}

class Floor;
class OpenVRManager;
class SimpleIKModule;

class MainApp : public crsf::TDynamicModuleInterface, public rppanda::DirectObject
{
public:
    MainApp();
    virtual ~MainApp();

    void OnLoad() override;
    void OnStart() override;
    void OnExit() override;

    void setup_physics();
    void setup_ik();
    void setup_avatar();
    void setup_chair();

    void update();

private:
    friend class MainGUI;

    void on_imgui_new_frame();

    void change_actor(crsf::TActorObject* new_actor);

    crsf::TGraphicRenderEngine* rendering_engine_;
    rpcore::RenderPipeline* pipeline_;

    std::unique_ptr<OpenVRManager> openvr_manager_;

    std::unique_ptr<Floor> floor_;
    std::vector<std::shared_ptr<crsf::TActorObject>> actors_;
    crsf::TActorObject* current_actor_ = nullptr;

    NodePath trackers_[2];

    SimpleIKModule* simple_ik_ = nullptr;

    std::unique_ptr<MainGUI> main_gui_;
};
