#pragma once
#include "pbe/Core/Math/Common.h"


namespace pbe
{
	class Entity;
	
	using namespace physx;

	namespace physics
	{
		/**
		\brief Combines a shape pointer and the actor the shape belongs to into one memory location.

		Serves as a base class for PxQueryHit.

		@see PxQueryHit
		*/
		struct ActorShape
		{
			PX_INLINE ActorShape(Entity* e = NULL) : entity(e){}

			Entity* entity;
		};


		/**
		\brief Scene query hit information.
		*/
		struct QueryHit : public ActorShape
		{
			PX_INLINE			QueryHit() : faceIndex(0xFFFFffff) {}

			/**
			Face index of touched triangle, for triangle meshes, convex meshes and height fields.

			\note This index will default to 0xFFFFffff value for overlap queries.
			\note Please refer to the user guide for more details for sweep queries.
			\note This index is remapped by mesh cooking. Use #PxTriangleMesh::getTrianglesRemap() to convert to original mesh index.
			\note For convex meshes use #PxConvexMesh::getPolygonData() to retrieve touched polygon data.
			*/
			uint				faceIndex;
		};

		/**
		\brief Scene query hit information for raycasts and sweeps returning hit position and normal information.

		::PxHitFlag flags can be passed to scene query functions, as an optimization, to cause the SDK to
		only generate specific members of this structure.
		*/
		struct LocationHit : public QueryHit
		{
			PX_INLINE			LocationHit() : flags(0), position(Vec3(0)), normal(Vec3(0)), distance(PX_MAX_REAL) {}

			/**
			\note For raycast hits: true for shapes overlapping with raycast origin.
			\note For sweep hits: true for shapes overlapping at zero sweep distance.

			@see PxRaycastHit PxSweepHit
			*/
			PX_INLINE bool		hadInitialOverlap() const { return (distance <= 0.0f); }

			// the following fields are set in accordance with the #PxHitFlags
			PxHitFlags			flags;		//!< Hit flags specifying which members contain valid values.
			Vec3				position;	//!< World-space hit position (flag: #PxHitFlag::ePOSITION)
			Vec3				normal;		//!< World-space hit normal (flag: #PxHitFlag::eNORMAL)

			/**
			\brief	Distance to hit.
			\note	If the eMTD flag is used, distance will be a negative value if shapes are overlapping indicating the penetration depth.
			\note	Otherwise, this value will be >= 0 */
			PxF32				distance;
		};


		/**
		\brief Stores results of raycast queries.

		::PxHitFlag flags can be passed to raycast function, as an optimization, to cause the SDK to only compute specified members of this
		structure.

		Some members like barycentric coordinates are currently only computed for triangle meshes and height fields, but next versions
		might provide them in other cases. The client code should check #flags to make sure returned values are valid.

		@see PxScene.raycast PxBatchQuery.raycast
		*/
		struct RaycastHit : public LocationHit
		{
			PX_INLINE			RaycastHit() : u(0.0f), v(0.0f) {}

			// the following fields are set in accordance with the #PxHitFlags

			PxReal	u, v;			//!< barycentric coordinates of hit point, for triangle mesh and height field (flag: #PxHitFlag::eUV)
#if !PX_P64_FAMILY
			PxU32	padTo16Bytes[3];
#endif
		};

		/**
		\brief Stores results of overlap queries.

		@see PxScene.overlap PxBatchQuery.overlap
		*/
		struct OverlapHit : public QueryHit { PxU32 padTo16Bytes; };
		

	}
}

