#include "floor.hpp"

#include <render_pipeline/rpcore/util/rpmaterial.hpp>
#include <render_pipeline/rpcore/util/rpgeomnode.hpp>
#include <render_pipeline/rpcore/globals.hpp>
#include <render_pipeline/rppanda/showbase/showbase.hpp>

#include <crsf/RenderingEngine/TGraphicRenderEngine.h>
#include <crsf/CRModel/TWorld.h>
#include <crsf/CRModel/TCube.h>
#include <crsf/CREngine/TPhysicsManager.h>

Floor::Floor(const std::string& name, const LMatrix4f& mat)
{
    crsf::TGraphicRenderEngine* rendering_engine = crsf::TGraphicRenderEngine::GetInstance();
    crsf::TWorld* cr_world = rendering_engine->GetWorld();

    LVecBase3f pos;
    LVecBase3f scale;
    LVecBase3f hpr;
    LVecBase3f shear;

    decompose_matrix(mat, scale, shear, hpr, pos);

    floor_ = crsf::CreateObject<crsf::TCube>(name, LVecBase3f(0, -3.5, 0), LVecBase3f(20, 20, 0.02f));
    floor_->CreateGraphicModel();
    floor_->SetRealObject();
    cr_world->AddWorldObject(floor_);

    rpcore::RPGeomNode gn(floor_->GetNodePath());
    auto material = gn.get_material(0);
    material.set_roughness(1.0f);
    material.set_base_color(LColorf(1.0f));
    material.set_specular_ior(1.0f);

    crsf::TPhysicsModel::Parameters params;
    params.m_fMass = 0;
    params.m_fFriction = 10.0f;
    floor_->CreatePhysicsModel(params);
    crsf::TPhysicsManager::GetInstance()->AddModel(floor_);

    //floor_->SetMatrix(mat, cr_world);

    rppanda::ShowBase::get_global_ptr()->accept("SceneBox::change_to_virtual", [this](const Event* ev) {
        floor_->Hide(crsf::TWorldObject::GetVirtualObjectMask());
    });

    rppanda::ShowBase::get_global_ptr()->accept("SceneBox::change_to_real", [this](const Event* ev) {
        floor_->Show(crsf::TWorldObject::GetVirtualObjectMask());
    });
}

Floor::~Floor()
{
    crsf::TPhysicsManager::GetInstance()->RemoveModel(floor_);
    floor_->DetachWorldObject();
}
