#pragma once

#include <filesystem>
#include <mesh_structs.h>
#include <message_queue.h>

class Engine;

class MeshManager
{
public:
	MeshManager() = delete;
	MeshManager(const MeshManager&) = delete;
	MeshManager(MeshManager&&) = delete;

	MeshManager& operator=(const MeshManager&) = delete;
	MeshManager& operator=(MeshManager&&) = delete;

	explicit MeshManager(Engine& engine);

public:
	static MeshManager& Allocate(Engine& engine);
	static MeshManager& Instance();

	GpuMesh GetMesh(MeshHandle handle);
	MeshHandle LoadMesh(std::filesystem::path path);

public:
	std::vector<GpuMesh> mMeshes;

private:
	Engine& mEngine;
	static MeshManager* mInstance;
};
