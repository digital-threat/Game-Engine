#include "mesh_manager.h"

#include <cassert>
#include <engine.h>
#include <obj_loading.h>
#include <iostream>
#include <mesh_serialization.h>
#include <windows.h>

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

MeshManager& MeshManager::Instance()
{
	return *mInstance;
}

GpuMesh* MeshManager::GetMesh(const char* path)
{
	auto it = mMeshes.find(path);
	if (it != mMeshes.end())
	{
		return &it->second;
	}

	return LoadMesh(path);
}

GpuMesh* MeshManager::LoadMesh(const char* path)
{
	CpuMesh cpuMesh{};
	GpuMesh gpuMesh{};

	std::filesystem::path systemPath = path;
	if (DeserializeMesh(systemPath, cpuMesh))
	{
		mEngine.UploadMesh(cpuMesh.indices, cpuMesh.vertices, gpuMesh);

		mMeshes[path] = gpuMesh;
	}

	return &mMeshes[path];
}