#include "mesh_manager.h"

#include <cassert>
#include <engine.h>
#include <iostream>
#include <mesh_serialization.h>
#include <obj_loading.h>
#include <windows.h>

MeshManager* MeshManager::mInstance = nullptr;


MeshManager::MeshManager(Engine& engine) : mEngine(engine) {}

MeshManager& MeshManager::Allocate(Engine& engine)
{
	assert(mInstance == nullptr);
	mInstance = new MeshManager(engine);
	return *mInstance;
}

MeshManager& MeshManager::Instance() { return *mInstance; }

GpuMesh MeshManager::GetMesh(MeshHandle handle) { return mMeshes[handle]; }

MeshHandle MeshManager::LoadMesh(std::filesystem::path path)
{
	std::vector<std::string> textures;
	auto textureCount = TextureManager::Instance().GetTextureCount();

	CpuMesh mesh = ParseObj(path, textures);
	mesh.textureOffset = textureCount;

	for (u32 i = 0; i < textures.size(); i++)
	{
		TextureManager::Instance().LoadTexture(path.parent_path(), textures[i]);
	}

	mMeshes.emplace_back();
	mEngine.UploadMesh(mesh, mMeshes.back());

	return mMeshes.size() - 1;
}
