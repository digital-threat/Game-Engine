#pragma once

#include <renderer_vk_types.h>

class Engine;

namespace Renderer
{
	class MeshManager
	{
		MeshManager();

	public:
		static void Allocate();
		static MeshManager& Get();
		MeshAsset* GetMesh(const char* pPath);
		MeshAsset* LoadMesh(Engine* pEngine, const char* pPath);

	private:
		static MeshManager* mInstance;
		std::unordered_map<const char*, MeshAsset*> mMeshes;
	};
}


