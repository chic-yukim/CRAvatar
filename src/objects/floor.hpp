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

    const std::shared_ptr<crsf::TCube>& get_object() const;

private:
    std::shared_ptr<crsf::TCube> floor_;
};

inline const std::shared_ptr<crsf::TCube>& Floor::get_object() const
{
    return floor_;
}
