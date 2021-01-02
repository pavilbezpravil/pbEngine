#pragma once

#include "RendObj.h"
#include "pbe/Core/UUID.h"

namespace pbe {
	
	class Entity;


	class RendScene : public RefCounted
	{
	public:

		void OnRender(RendCamera* cams, uint nCams);
		
		void AddMesh(Entity e);
		void RemoveMesh(Entity e);
		
	private:
		std::unordered_map<UUID, RendObjMesh*> meshMap;
		StructuredBuffer meshInstData;

		void RenderView(RendCamera& cam);
		
		void ResizeMeshInstData(uint newSize);

		bool HasMesh(Entity e);
	};

}
