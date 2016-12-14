/**
\file d:\Chrysalis\Code\ChrysalisSDK\GameObjects\EntityAwareness.h

Declares the entity awareness class. Refactored from CryEngine\CryAction\GameObjects\WorldQuery.h.

This adds spatial awareness of other entities to the actor it extends. It uses raycasts and AABB queries to track entities
with which the actor might wish to or need to interact.
**/
#pragma once

#include <Entities/Interaction/IEntityAwarenessComponent.h>


struct IActor;
struct IViewSystem;


class CEntityAwarenessComponent : public CGameObjectExtensionHelper<CEntityAwarenessComponent, IEntityAwarenessComponent>
{
public:

	// ISimpleExtension
	bool Init(IGameObject * pGameObject) override;
	void PostInit(IGameObject * pGameObject) override;
	void FullSerialize(TSerialize ser) override;
	void Update(SEntityUpdateContext& ctx, int updateSlot) override;
	void GetMemoryUsage(ICrySizer *pSizer) const override;
	// ~ISimpleExtension


	// ***
	// *** IEntityAwarenessComponent
	// ***


	/**
	Gets the position of the actor's eyes.

	\return The eye position.
	**/
	ILINE const Vec3& GetPos() const { return m_eyePosition; }


	/**
	Gets the normalised direction the actor's eye are gazing.
	
	\return The direction.
	**/
	ILINE const Quat& GetDir() const { return m_eyeDirection; }


	/**
	Sets proximity radius, which determines how far away from the actor we will search for entities.

	\param	proximityRadius The proximity radius.
	**/
	ILINE void SetProximityRadius(float proximityRadius) override
	{
		m_proximityRadius = proximityRadius;
		m_validQueries &= ~(eWQ_Proximity | eWQ_InFrontOf);
	}


	ILINE Entities& ProximityQuery() override
	{
		RefreshQueryCache(eWQ_Proximity);

		return m_entitiesInProximity;
	}


	ILINE Entities& NearQuery() override
	{
		RefreshQueryCache(eWQ_Proximity);
		RefreshQueryCache(eWQ_CloseBy);

		return m_entitiesNear;
	}


	/**
	Utilises the base ray-cast query to see if there is a hit within the "forwardCastDistance".
	If so, that hit is returned.

	\param	maxDistance The maximum distance at which a hit from the base ray is to be considered.
	\param	ignoreGlass true to ignore glass.

	\return null if it fails, else the look at point.
	**/
	ILINE const ray_hit* GetLookAtPoint(const float maxDistance = 0.0f, bool ignoreGlass = false)
	{
		RefreshQueryCache(eWQ_Raycast);

		if (m_rayHitAny)
		{
			const ray_hit* hit = !ignoreGlass && m_rayHitPierceable.dist >= 0.0f ? &m_rayHitPierceable : &m_rayHitSolid;

			if ((maxDistance <= 0.0f) || (hit->dist <= maxDistance))
				return hit;
		}

		return nullptr;
	}


	ILINE const EntityId GetLookAtEntityId(bool ignoreGlass = false)
	{
		RefreshQueryCache(eWQ_Raycast);

		return !ignoreGlass && m_rayHitPierceable.dist >= 0.0f ? INVALID_ENTITYID : m_lookAtEntityId;
	}


	/**
	Utilises the base ray-cast query, but filters the result to only include a hit if there is a viable
	result within the proximity radius limit.

	\return null if it fails, else the successful raycast data.
	**/
	ILINE const ray_hit* RaycastQuery() override
	{
		return GetLookAtPoint(m_proximityRadius);
	}


	/**
	A proximity based query that limits the results to only those entities which are considered to be in front of the
	actor. These are limited based on a line segment from the actor's eyes, forward in the direction they are looking.
	
	\return The entities in front of the actor.
	**/
	ILINE const Entities& GetEntitiesInFrontOf()
	{
		return InFrontOfQuery();
	}


	/**
	Based on the results from GetEntitiesInFrontOf(), it will return just one entity from the list.
	NOTE: the list is not sorted in any manner, so which one is returned in non-deterministic.

	\return	null if it fails, else the first entity in the list of those in front of the actor.
	**/
	ILINE IEntity* GetEntityInFrontOf()
	{
		auto entities = InFrontOfQuery();

		// TODO: This should probably be more precise about what it returns, rather than just the first element of the vector.
		return entities.empty() ? nullptr : gEnv->pEntitySystem->GetEntity(entities [0]);
	}


	/**
	Performs a near query, then narrows down the results to ones that fit within the range of dot product results.
	The best match is sorted into the first position in the results vector. All other results are unsorted.

	Results aren't cached for this query, except for the cacheing provided by the unlaying 'near' query.
	
	\param	minDot (Optional) the minimum dot product.
	\param	maxDot (Optional) the maximum dot product.
	
	\return The near dot filtered.
	**/
	const Entities& GetNearDotFiltered(float minDot = 0.9f, float maxDot = 1.0f) override;


	// ***
	// *** CEntityAwarenessComponent
	// ***

	enum EDebugBits
	{
		eDB_Debug					= BIT(0),
		eDB_ProximalEntities		= BIT(1),
		eDB_NearEntities			= BIT(2),
		eDB_RayCast					= BIT(3),
		eDB_InFront					= BIT(4),
		eDB_DotFiltered				= BIT(5),
	};

	CEntityAwarenessComponent();
	virtual ~CEntityAwarenessComponent();


	struct SExternalCVars
	{
		int m_debug;
	};
	const SExternalCVars &GetCVars() const;


	/**
	Functor that receives the results of deferred ray-cast operations.

	\param	rayID  Identifier for the ray.
	\param	result The result.
	**/
	void OnRayCastDataReceived(const QueuedRayID& rayID, const RayCastResult& result);


private:
	// Keep in sync with m_updateQueryFunctions.
	enum EWorldQuery
	{
		eWQ_Raycast = 0,
		eWQ_Proximity,
		eWQ_CloseBy,
		eWQ_InFrontOf,
	};


	struct SRayInfo
	{
		void Reset()
		{
			if (rayId != 0)
			{
				g_pGame->GetRayCaster().Reset();
				rayId = 0;
			}
			counter = 0;
		}

		QueuedRayID rayId = 0;
		uint32 counter = 0;
	};


	/**
	Checks to see if we have a query result that is still valid for this FrameId. If no fresh query result is found
	it will run a query update and mark the new result as being useable within this FrameId. This works like a simple
	cache that presents multiple calls from needing to run the queries again.
	
	Results are returned as side-effects of this function.
	
	\param	query An enum indicating which query should be refreshed (if required).
	**/
	ILINE void RefreshQueryCache(EWorldQuery query)
	{
		uint32 queryMask = 1u << query;

		int frameid = gEnv->pRenderer->GetFrameID();
		if (m_renderFrameId != frameid)
		{
			// We've changed frames, clear all queries from the valid indicator.
			m_renderFrameId = frameid;
			m_validQueries = 0;
		}
		else
		{
			// If there's a valid result, we can early out, and let the caller use that.
			if (m_validQueries & queryMask)
				return;
		}

		// Run the query the caller was interested in.
		// Results are returned as a side-effect of the function that is run.
		(this->*(m_updateQueryFunctions [query]))();

		// Mark that query as now being valid for this FrameId.
		m_validQueries |= queryMask;
	}


	// Ensure all update query routines conform to this definition.
	typedef void (CEntityAwarenessComponent::*UpdateQueryFunction)();

	// Limit the size of the array used to track the raycasts.
	static const int maxQueuedRays { 6 };

	/** The proximity radius defines the maximum distance we will search for entities that are considered
	"in-proximity". It is used to restrict both proximity queries and ray-cast queries. */
	float m_proximityRadius { 6.0f };

	// A mask of queries which are currently valid for this FrameId.
	uint32 m_validQueries { 0 };

	// Track the current render frame so we can drop query results that are too old.
	int m_renderFrameId { -1 };

	/** The actor associated with this instance. It's critical that this value is non-null or the queries
	will fail to run correctly. */
	IActor * m_pActor { nullptr };

	/** The eye position for our actor. */
	Vec3 m_eyePosition = Vec3 { ZERO };

	/** The direction in which our actor is looking. */
	Quat m_eyeDirection { IDENTITY };

	// An array of functors which run the update queries.
	static UpdateQueryFunction m_updateQueryFunctions [];

	// TODO: Maybe use a vector for this?
	SRayInfo m_queuedRays [maxQueuedRays];

	// Keep a track of how many rays we have queued.
	// TODO: Is there a safer way to handle all of this?
	uint32 m_requestCounter { 0 };

	/** Track the time of the last deferred ray-cast. */
	float m_timeLastDeferredResult { 0.0f };

	// ray-cast query
	bool m_rayHitAny { false };

	/** If there is a solid hit from the forward ray this is it's result. The result may become stale. */
	ray_hit m_rayHitSolid;

	/** If there is a pierceable hit from the forward ray this is it's result. The result may become stale. */
	ray_hit m_rayHitPierceable;

	// The entity the object is currently looking towards.
	EntityId m_lookAtEntityId { INVALID_ENTITYID };

	// The entities within proximity of the AABB surrounding the actor. It may be worth highlighting these as being
	// interactive for the player.
	Entities m_entitiesInProximity;

	// The entities which are close enough to the actor to be interactable or worth highlighting.
	Entities m_entitiesNear;

	// Dot product filtered version of the near entities.
	Entities m_entitiesNearDotFiltered;

	// The entities which are in front of the actor.
	Entities m_entitiesInFrontOf;


	/**
	Performs a deferred raycast from the actors eye in the direction the actor is facing for forwardCastDistance
	metres. OnRayCastDataReceived is called asynchronously with the result.
	**/
	void UpdateRaycastQuery();


	/**
	Creates an AABB around the actor and performs a query on entities within that box. The size of the box is based
	on m_proximityRadius. Any entities which are within the box will be made available in the m_entitiesInProximity
	container as a side effect of running this query. No culling is performed on the entities returned from the query.
	
	You can use this as a base on which to build more nuanced and precise queries.
	**/
	void UpdateProximityQuery();


	/**
	A proximity based query that limits the results to a smaller range centered around the actor's eyes. The area is
	scaled by proximityCloseByFactor, which should provide a fairly neat box around the actor - within reasonable reach
	to indicate they can trigger actions on these entities.
	**/
	void UpdateNearQuery();


	/**
	Performs a proximity query using an AABB based on the actor and m_proximityRadius. The results of this are culled
	by first converting the returned AABB for each entity into an OBB and checking if a line intersection from the
	actor's eyes would intersect this OBB. Results are saved as a side effect of running the query. You can get multiple
	hits in the results, and these are not sorted in any way.
	**/
	void UpdateInFrontOfQuery();


	/**
	A proximity based query that limits the results to only those entities which are considered to be in front of the
	actor. These are limited based on a line segment from the actor's eyes, forward in the direction they are looking.
	
	\return A reference to a const Entities.
	**/
	ILINE const Entities& InFrontOfQuery()
	{
		RefreshQueryCache(eWQ_Proximity);
		RefreshQueryCache(eWQ_InFrontOf);

		return m_entitiesInFrontOf;
	}


	int GetRaySlot();

	int GetSlotForRay(const QueuedRayID& rayId) const;
};
