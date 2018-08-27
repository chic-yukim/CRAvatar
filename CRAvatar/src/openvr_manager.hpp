/**
 * Coexistence Reality Software Framework (CRSF)
 * Copyright (c) Center of Human-centered Interaction for Coexistence. All rights reserved.
 * See the LICENSE.md file for more details.
 */

#pragma once

#include <render_pipeline/rppanda/gui/onscreen_text.hpp>
#include <render_pipeline/rppanda/showbase/direct_object.hpp>

#include <openvr.h>

namespace rpcore {
class RenderPipeline;
}

namespace rpplugins {
class OpenVRPlugin;
}

class OpenVRManager : public rppanda::DirectObject
{
public:
    OpenVRManager(rpcore::RenderPipeline& pipeline);
    virtual ~OpenVRManager();

    bool is_available() const;

    rpplugins::OpenVRPlugin* get_plugin() const;

    void print_serials();

    void toggle_ar();

    void create_devices_label();

    void create_text2d();

    NodePath get_hmd_nodepath() const;
    const std::vector<NodePath>& get_basestation_nodepaths() const;
    const std::vector<NodePath>& get_controller_nodepaths() const;
    const std::vector<NodePath>& get_tracker_nodepaths() const;

private:
    void caching_devices();
    void process_controller_event();
    void update_texts();

    rpcore::RenderPipeline& pipeline_;
    rpplugins::OpenVRPlugin* openvr_plugin_ = nullptr;

    uint64_t controller_last_states_[vr::k_unMaxTrackedDeviceCount];

    std::vector<rppanda::OnscreenText> texts_;

    NodePath hmd_np_;
    std::vector<NodePath> basestation_np_list_;
    std::vector<NodePath> controller_np_list_;
    std::vector<NodePath> tracker_np_list_;
};

inline bool OpenVRManager::is_available() const
{
    return openvr_plugin_ != nullptr;
}

inline rpplugins::OpenVRPlugin* OpenVRManager::get_plugin() const
{
    return openvr_plugin_;
}

inline NodePath OpenVRManager::get_hmd_nodepath() const
{
    return hmd_np_;
}

inline const std::vector<NodePath>& OpenVRManager::get_basestation_nodepaths() const
{
    return basestation_np_list_;
}

inline const std::vector<NodePath>& OpenVRManager::get_controller_nodepaths() const
{
    return controller_np_list_;
}

inline const std::vector<NodePath>& OpenVRManager::get_tracker_nodepaths() const
{
    return tracker_np_list_;
}
