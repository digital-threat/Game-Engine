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

MeshManager& MeshManager::Get()
{
	return *mInstance;
}

Mesh MeshManager::GetMesh(const char* pPath)
{
	auto it = mMeshes.find(pPath);
	if (it != mMeshes.end())
	{
		return it->second;
	}

	return Mesh();
}

Mesh MeshManager::LoadMesh(const char* pPath)
{
	MeshData meshData{};
	Mesh meshAsset{};

	std::filesystem::path path = pPath;
	if (DeserializeMesh(path, meshData))
	{
		MeshBuffers meshBuffers = mEngine.UploadMesh(meshData.indices, meshData.vertices);
		meshAsset.meshBuffers = meshBuffers;
		meshAsset.indexCount = meshData.indices.size();
		mMeshes[pPath] = meshAsset;
	}

	return meshAsset;
}

void MeshManager::Update(std::atomic_bool &pCancellationToken)
{
	while(!pCancellationToken)
	{
		ProcessMessages();
	}
}

void MeshManager::ProcessMessage(Message *pMessage)
{
	std::string& message = pMessage->message;
	switch(pMessage->type)
	{
		case MessageType::STRING:
		{
			if (message == "LoadMesh")
			{
				auto stringMsg = static_cast<StringMessage*>(pMessage);
				std::string& path = stringMsg->param;

				Mesh mesh = GetMesh(path.c_str());
				if (mesh.indexCount == 0)
				{
					mesh = LoadMesh(path.c_str());
					if (mesh.indexCount == 0)
					{
						std::cerr << "Failed to load mesh from path: " << path << std::endl;
						return;
					}
				}

				if (stringMsg->sender != nullptr)
				{
					MeshMessage* reply = new MeshMessage("MeshLoaded", mesh, stringMsg->entity);
					stringMsg->sender->QueueMessage(reply);
				}
			}
		} break;
		default:
		{

		} break;
	}
}