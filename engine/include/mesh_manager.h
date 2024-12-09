#pragma once

#include <renderer_vk_types.h>

class Engine;

namespace Renderer
{
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
		static MeshManager& Get();
		MeshAsset* GetMesh(const char* pPath);
		MeshAsset* LoadMesh(const char* pPath);

	private:
		Engine& mEngine;
		static MeshManager* mInstance;
		std::unordered_map<const char*, MeshAsset*> mMeshes;
	};
}


