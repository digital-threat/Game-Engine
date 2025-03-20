#pragma once

#include <atomic>
#include <unordered_map>
#include <message_queue.h>

class Engine;

class MeshManager : public MessageQueue
{
public:
	MeshManager() = delete;
	MeshManager(const MeshManager&) = delete;
	MeshManager(MeshManager&&) = delete;

	MeshManager& operator=(const MeshManager&) = delete;
	MeshManager& operator=(MeshManager&&) = delete;

	explicit MeshManager(Engine& engine);

public:
	void Update(std::atomic_bool& pCancellationToken);

	static MeshManager& Allocate(Engine &engine);
	static MeshManager& Get();

private:
	void ProcessMessage(Message *pMessage) override;

	GpuMesh GetMesh(const char* pPath);
	GpuMesh LoadMesh(const char* pPath);

private:
	Engine& mEngine;
	static MeshManager* mInstance;
	std::unordered_map<const char*, GpuMesh> mMeshes;
};