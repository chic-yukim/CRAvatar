#include "floor.hpp"

#include <render_pipeline/rpcore/util/rpmaterial.hpp>
#include <render_pipeline/rpcore/util/rpgeomnode.hpp>

#include <crsf/CRModel/TCube.h>
#include <crsf/CREngine/TPhysicsManager.h>

Floor::Floor(const std::string& name, const LMatrix4f& mat)
{
    LVecBase3f pos;
    LVecBase3f scale;
    LVecBase3f hpr;
    LVecBase3f shear;

    decompose_matrix(mat, scale, shear, hpr, pos);

    floor_ = crsf::CreateObject<crsf::TCube>(name, pos, scale);
}

Floor::~Floor()
{
    crsf::TPhysicsManager::GetInstance()->RemoveModel(floor_);
    floor_->DetachWorldObject();
}

void Floor::setup_graphics()
{
    floor_->CreateGraphicModel();
    floor_->SetRealObject();

    rpcore::RPGeomNode gn(floor_->GetNodePath());
    auto material = gn.get_material(0);
    material.set_roughness(1.0f);
    material.set_base_color(LColorf(1.0f));
    material.set_specular_ior(1.0f);
}

void Floor::setup_physics()
{
    crsf::TPhysicsModel::Parameters params;
    params.m_fMass = 0;
    params.m_fFriction = 10.0f;
    floor_->CreatePhysicsModel(params);
    crsf::TPhysicsManager::GetInstance()->AddModel(floor_);
}
