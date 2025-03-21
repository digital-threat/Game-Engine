#pragma once

#include <atomic>
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

	GpuMesh* GetMesh(const char* path);
	GpuMesh* LoadMesh(const char* path);

private:
	Engine& mEngine;
	static MeshManager* mInstance;
	std::unordered_map<const char*, GpuMesh> mMeshes;
};