#include "pch.h"
#include "RendScene.h"
#include "pbe/Scene/Entity.h"
#include "pbe/Renderer/GpuBuffer.h"

namespace pbe {

	struct MeshInstData
	{
		Mat4 transform;
	};

	void RendScene::OnRender(RendCamera* cams, uint nCams)
	{
		for (int i = 0; i < nCams; ++i) {
			RenderView(cams[i]);
		}
	}

	void RendScene::AddMesh(Entity e)
	{
		HZ_CORE_ASSERT(!HasMesh(e));
		auto& mc = e.GetComponent<MeshComponent>();
		auto objMesh = new RendObjMesh;
		objMesh->instIndx = meshMap.size();
		objMesh->mesh = mc.Mesh;
		meshMap[e.GetUUID()] = objMesh;
	}

	void RendScene::RemoveMesh(Entity e)
	{
	}

	void RendScene::RenderView(RendCamera& cam)
	{
	}

	void RendScene::ResizeMeshInstData(uint newSize)
	{
		if (meshInstData.GetElementCount() >= newSize) {
			return;
		}
		
		DWORD index;
		char res = BitScanReverse(&index, newSize);
		HZ_CORE_ASSERT(res);
		if (res) {
			int count = 1 << (index + 1);
			meshInstData.Create(L"meshInstData", count, sizeof MeshInstData);
		}
	}

	bool RendScene::HasMesh(Entity e)
	{
		return meshMap.find(e.GetUUID()) != meshMap.end();
	}
}
