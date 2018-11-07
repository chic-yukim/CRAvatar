#pragma once

#include <nodePath.h>

#include <deque>

#include <render_pipeline/rppanda/showbase/direct_object.hpp>

#include <openvr.h>

namespace rpcore {
class RenderPipeline;
}

class ARSystem : public rppanda::DirectObject
{
public:
    ARSystem(rpcore::RenderPipeline& pipeline);
    virtual ~ARSystem();

    void set_sync_event_name(const std::string& ev_name);

    void push_pose(NodePath np, const LMatrix4f& pose);
    void remove_nodepath(NodePath np);

    size_t get_queue_size() const;
    void set_queue_size(size_t qsize);

private:
    void sync_pose();

    rpcore::RenderPipeline& pipeline_;

    std::string sync_event_name_;

    std::map<NodePath, std::deque<LMatrix4f>> pose_queue_;
    size_t queue_size_ = 1;
};

// ************************************************************************************************

inline ARSystem::ARSystem(rpcore::RenderPipeline& pipeline) : pipeline_(pipeline)
{
}

inline void ARSystem::remove_nodepath(NodePath np)
{
    pose_queue_.erase(np);
}

inline size_t ARSystem::get_queue_size() const
{
    return queue_size_;
}

inline void ARSystem::set_queue_size(size_t qsize)
{
    queue_size_ = qsize;
}
