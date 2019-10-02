
class GamePiecePhysicsCallback : public PxSimulationEventCallback
{
	void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) override
	{
		int a = 0;
	}
	void onWake(PxActor** actors, PxU32 count) override
	{
		int a = 0;
	}
	void onSleep(PxActor** actors, PxU32 count) override
	{
		int a = 0;
	}
	void onTrigger(PxTriggerPair* pairs, PxU32 count) override
	{
		int a = 0;
	}
	void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) override
	{
		int a = 0;
	}
/*
	float Cr = 0.02f;
	//fCr coeffecient of restitution
	float3 ApplySimpleLinearImplulse(float3 v1,float3 c1)
	{
		float d = dot(v1, (c1));
		float j = fmax(-(1 + Cr) * d, 0);
		return  v1 + (j * (c1));
	}
    
	void KartCollisionResponse(PxContactPairHeader pairHeader,const PxContactPair* cp,bool on_enter = false)
	{
		PlatformOutput(true, "KartCollisionResponse");
		PxActor* otherActor = (track_rigid_body == pairHeader.actors[0]) ?
			pairHeader.actors[1] : pairHeader.actors[0];
		Kart* kart = reinterpret_cast<Kart*>(otherActor->userData);
		uint32_t contact_count = cp->extractContacts((PxContactPairPoint*)physx_contact_point_buffer->base, physx_contact_point_buffer->size);
		for (int cpi = 0; cpi < contact_count; ++cpi)
		{
			PxContactPairPoint* cpp = (PxContactPairPoint*)physx_contact_point_buffer->base + cpi;
			PxVec3 pp = cpp->position;
			PxVec3 np = cpp->normal;
			//PlatformOutput(true, "pp.x:%f pp.y:%f pp.z:%f \n", pp.x,pp.y,pp.z);
			//PlatformOutput(true, "pn.x:%f pn.y:%f pn.z:%f \n", np.x,np.y,np.z);
			float3 p = float3(cpp->position);
			float3 n = float3(cpp->normal);
			float sep = cpp->separation;
			float3 rn = (reflect((kart->v), normalize(n)));
	//		if (on_enter)
//				kart->last_v = float3(0);
//			else
			if(sep < 0.0f)
			{
				kart->v = ApplySimpleLinearImplulse(kart->v,n);
				kart->e.t->p = kart->last_p;
				kart->e.t->p += kart->v * mStepSize + kart->last_v * mStepSize;;
				kart->last_v = kart->v;
			}
		}
	}
*/
	void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs) override
	{
#if 0
		for (PxU32 i = 0; i < nbPairs; i++)
		{
			const PxContactPair& cp = pairs[i];
			if ((pairHeader.actors[0] == rigid_body) ||
				(pairHeader.actors[1] == rigid_body))
			{
				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_PERSISTS)
				{
					PlatformOutput(true, "Touch is persisting\n");
				}
				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					PlatformOutput(true, "Touch is Lost\n");
				}
				if (cp.events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					PlatformOutput(true, "Touch is Found\n");
				}
			}
		}
#endif
        PlatformOutput(true, "OnContact");
	}
};
