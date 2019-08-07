#if !defined(PHYSICS_H)

#define _DEBUG
#include "PxPhysicsAPI.h"

//NOTE(Ray): API is defined here but the hardwork is done by NVIDIA physics lib.
using namespace physx;

struct Physics
{
    
};

namespace PhysicsCode
{
//    extern physx::PxRigidStatic* test_rigid_body;
//    extern physx::PxTriangleMeshGeometry test_mesh_geo;
    void Init();    
};

#define PHYSICS_H
#endif
