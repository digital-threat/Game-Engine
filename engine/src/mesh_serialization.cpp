#include <iostream>
#include <filesystem>
#include <mesh_serialization.h>
#include <mesh_structs.h>

bool SerializeMesh(const CpuMesh &mesh, std::filesystem::path &path)
{
	std::ofstream file;
	file.open(path, std::ios::out | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to serialize mesh at path: " << path << std::endl;
		return false;
	}

	size_t numVertices = mesh.vertices.size();
	size_t numIndices = mesh.indices.size();
	size_t numMaterials = mesh.materials.size();
	size_t numMatIds = mesh.matIds.size();

	file.write(reinterpret_cast<const char*>(&numVertices), sizeof(size_t));
	file.write(reinterpret_cast<const char*>(&numIndices), sizeof(size_t));
	file.write(reinterpret_cast<const char*>(&numMaterials), sizeof(size_t));
	file.write(reinterpret_cast<const char*>(&numMatIds), sizeof(size_t));

	for (size_t i = 0; i < numVertices; i++)
	{
		file.write(reinterpret_cast<const char*>(&mesh.vertices[i]), sizeof(Vertex));
	}

	for (size_t i = 0; i < numIndices; i++)
	{
		file.write(reinterpret_cast<const char*>(&mesh.indices[i]), sizeof(u32));
	}

	for (size_t i = 0; i < numMaterials; i++)
	{
		file.write(reinterpret_cast<const char*>(&mesh.materials[i]), sizeof(Material));
	}

	for (size_t i = 0; i < numMatIds; i++)
	{
		file.write(reinterpret_cast<const char*>(&mesh.matIds[i]), sizeof(i32));
	}

	file.close();
	if (!file.good())
	{
		std::cerr << "Failed to serialize mesh at path: " << path << std::endl;
		return false;
	}
	return true;
}

bool DeserializeMesh(std::filesystem::path &path, CpuMesh &outMesh)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Failed to deserialize mesh at path: " << path << std::endl;
		return false;
	}

	size_t numVertices, numIndices, numMaterials, numMatIds;

	file.read(reinterpret_cast<char*>(&numVertices), sizeof(size_t));
	file.read(reinterpret_cast<char*>(&numIndices), sizeof(size_t));
	file.read(reinterpret_cast<char*>(&numMaterials), sizeof(size_t));
	file.read(reinterpret_cast<char*>(&numMatIds), sizeof(size_t));

	std::vector<Vertex> vertices(numVertices);
	std::vector<u32> indices(numIndices);
	std::vector<Material> materials(numMaterials);
	std::vector<i32> matIds(numMatIds);

	for (size_t i = 0; i < numVertices; i++)
	{
		file.read(reinterpret_cast<char*>(&vertices[i]), sizeof(Vertex));
	}

	for (size_t i = 0; i < numIndices; i++)
	{
		file.read(reinterpret_cast<char*>(&indices[i]), sizeof(u32));
	}

	for (size_t i = 0; i < numMaterials; i++)
	{
		file.read(reinterpret_cast<char*>(&materials[i]), sizeof(Material));
	}

	for (size_t i = 0; i < numMatIds; i++)
	{
		file.read(reinterpret_cast<char*>(&matIds[i]), sizeof(i32));
	}

	outMesh.vertices = vertices;
	outMesh.indices = indices;
	outMesh.materials = materials;
	outMesh.matIds = matIds;

	file.close();
	if (!file.good())
	{
		std::cerr << "Failed to deserialize mesh at path: " << path << std::endl;
		return false;
	}
	return true;
}
