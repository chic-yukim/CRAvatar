#include "ar_system.hpp"

#include <spdlog/spdlog.h>

extern spdlog::logger* global_logger;

ARSystem::~ARSystem() = default;

void ARSystem::set_sync_event_name(const std::string& ev_name)
{
    if (!sync_event_name_.empty())
        ignore(sync_event_name_);

    sync_event_name_ = ev_name;
    accept(sync_event_name_, [this](const Event* ev) { sync_pose(); });
}

void ARSystem::push_pose(NodePath np, const LMatrix4f& pose)
{
    auto& q = pose_queue_[np];
    while (q.size() >= queue_size_)
        q.pop_front();
    q.push_back(pose);
}

void ARSystem::sync_pose()
{
    for (auto&& np_pose : pose_queue_)
    {
        if (np_pose.second.empty())
            continue;

        NodePath np = np_pose.first;
        np.set_mat(np_pose.second.front());
    }
}
