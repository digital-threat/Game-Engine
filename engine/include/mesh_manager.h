#pragma once

#include <atomic>
#include <mesh_structs.h>
#include <unordered_map>
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
	static MeshManager& Allocate(Engine &engine);
	static MeshManager& Instance();

	GpuMesh* GetMesh(MeshHandle handle);
	MeshHandle LoadMesh(std::string path);

private:
	Engine& mEngine;
	static MeshManager* mInstance;
	std::vector<GpuMesh> mMeshes;

};