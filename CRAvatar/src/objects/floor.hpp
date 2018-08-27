#pragma once

#include <luse.h>

#include <memory>

namespace crsf {
class TCube;
}

class Floor
{
public:
    Floor(const std::string& name, const LMatrix4f& mat = LMatrix4f::ident_mat());
    ~Floor();

    void setup_graphics();
    void setup_physics();

    crsf::TCube* get_object() const;

private:
    std::shared_ptr<crsf::TCube> floor_;
};

inline crsf::TCube* Floor::get_object() const
{
    return floor_.get();
}
