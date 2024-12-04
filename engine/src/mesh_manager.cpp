#include "mesh_manager.h"

#include <cassert>
#include <gltf_loading.h>

namespace Renderer
{
	MeshManager* MeshManager::mInstance = nullptr;

	MeshManager::MeshManager()
	{
	}

	void MeshManager::Allocate()
	{
		assert(mInstance == nullptr);
		mInstance = new MeshManager();
	}

	MeshManager& MeshManager::Get()
	{
		return *mInstance;
	}

	MeshAsset* MeshManager::GetMesh(const char* pPath)
	{
		auto it = mMeshes.find(pPath);
		if (it != mMeshes.end())
		{
			return it->second;
		}

		return nullptr;
	}

	MeshAsset* MeshManager::LoadMesh(Engine* pEngine, const char* pPath)
	{
		MeshAsset* mesh = LoadGltfMeshes(pEngine, pPath)[0];
		mMeshes[pPath] = mesh;
		return mesh;
	}
}


