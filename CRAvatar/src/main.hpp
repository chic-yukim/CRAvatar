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

struct ik_solver_t;
struct ik_effector_t;
struct ik_node_t;

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
    void setup_gui();

    void update();

private:
    void on_imgui_new_frame();

    void change_actor(crsf::TActorObject* new_actor);
    void rebuild_ik();
    void update_ik();

    crsf::TGraphicRenderEngine* rendering_engine_;
    rpcore::RenderPipeline* pipeline_;

    std::unique_ptr<OpenVRManager> openvr_manager_;

    std::unique_ptr<Floor> floor_;
    std::vector<std::shared_ptr<crsf::TActorObject>> actors_;
    crsf::TActorObject* current_actor_ = nullptr;

    ik_solver_t* ik_solver_ = nullptr;
    ik_effector_t* ik_effector_ = nullptr;
    std::vector<ik_node_t*> ik_nodes_;
    std::vector<NodePath> actor_joints_;
    NodePath np_effector_;
    NodePath r_acromioclavicular_;
};
