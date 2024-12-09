#include "mesh_manager.h"

#include <cassert>
#include <gltf_loading.h>

namespace Renderer
{
	MeshManager* MeshManager::mInstance = nullptr;


	MeshManager::MeshManager(Engine &engine)
		: mEngine(engine)
	{
	}

	MeshManager& MeshManager::Allocate(Engine &engine)
	{
		assert(mInstance == nullptr);
		mInstance = new MeshManager(engine);
		return *mInstance;
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

	MeshAsset* MeshManager::LoadMesh(const char* pPath)
	{
		MeshAsset* mesh = LoadGltfMeshes(&mEngine, pPath)[0];
		mMeshes[pPath] = mesh;
		return mesh;
	}
}


