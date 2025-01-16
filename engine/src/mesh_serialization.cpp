#include <iostream>
#include <filesystem>
#include <mesh_serialization.h>
#include <renderer_vk_types.h>

bool SerializeMesh(const MeshData &meshData, std::filesystem::path &path)
{
	std::ofstream file;
	file.open(path, std::ios::out | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to serialize mesh at path: " << path << std::endl;
		return false;
	}

	size_t numVertices = meshData.vertices.size();
	size_t numIndices = meshData.indices.size();

	file.write(reinterpret_cast<const char *>(&numVertices), sizeof(size_t));
	file.write(reinterpret_cast<const char *>(&numIndices), sizeof(size_t));

	for (size_t i = 0; i < numVertices; i++)
	{
		file.write(reinterpret_cast<const char *>(&meshData.vertices[i]), sizeof(Vertex));
	}

	for (size_t i = 0; i < numIndices; i++)
	{
		file.write(reinterpret_cast<const char *>(&meshData.indices[i]), sizeof(u32));
	}

	file.close();
	if (!file.good())
	{
		std::cerr << "Failed to serialize mesh at path: " << path << std::endl;
		return false;
	}
	return true;
}

bool DeserializeMesh(std::filesystem::path &path, MeshData &outMeshData)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to deserialize mesh at path: " << path << std::endl;
		return false;
	}

	size_t numVertices, numIndices;

	file.read(reinterpret_cast<char *>(&numVertices), sizeof(size_t));
	file.read(reinterpret_cast<char *>(&numIndices), sizeof(size_t));

	std::vector<Vertex> vertices(numVertices);
	std::vector<u32> indices(numIndices);

	for (size_t i = 0; i < numVertices; i++)
	{
		file.read(reinterpret_cast<char *>(&vertices[i]), sizeof(Vertex));
	}

	for (size_t i = 0; i < numIndices; i++)
	{
		file.read(reinterpret_cast<char *>(&indices[i]), sizeof(u32));
	}

	outMeshData.vertices = vertices;
	outMeshData.indices = indices;

	file.close();
	if (!file.good())
	{
		std::cerr << "Failed to deserialize mesh at path: " << path << std::endl;
		return false;
	}
	return true;
}
