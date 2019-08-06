#if ENGINEIMPL

namespace PhysicsCode
{
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;

#define PVD_HOST "127.0.0.1"	//Set this to the IP address of the system running the PhysX Visual Debugger that you want to connect to.

    physx::PxRigidStatic* test_rigid_body;
    physx::PxTriangleMeshGeometry test_mesh_geo;
    physx::PxPhysics* physics;
    void Init()
    {
//Connect to Physx visual debugger

        PxFoundation* mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gDefaultAllocatorCallback, gDefaultErrorCallback);
        //PxCreateFoundation(gDefaultAllocatorCallback,
		//gDefaultErrorCallback);

        if(!mFoundation)
        {
            Assert(false);
        }
        bool recordMemoryAllocations = true;

        PxPvd* mPvd = PxCreatePvd(*mFoundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate(PVD_HOST, 5425, 100);
        //PxPvdTransport* transport = PxDefaultPvdFileTransportCreate("pvd");
        bool is_connected_to_pvd = mPvd->connect(*transport, PxPvdInstrumentationFlag::ePROFILE);
        if (!mPvd->isConnected())
        {
            //Assert(false);
        }

        PxTolerancesScale scale{};
        if(!scale.isValid())
        {
            Assert(false);
        }

        physics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation,
		scale, recordMemoryAllocations);

        if (!physics)
            Assert(false);
    }
};

#endif
