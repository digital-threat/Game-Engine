#pragma once
#include <iostream>
#include <filesystem>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

template <>
struct std::hash<Vertex>
{
	size_t operator()(const Vertex& v) const
	{
		size_t h1 = std::hash<glm::vec3>()(v.position);
		size_t h2 = std::hash<float>()(v.u);
		size_t h3 = std::hash<float>()(v.v);
		size_t h4 = std::hash<glm::vec3>()(v.normal);
		return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
	}
};

inline MeshData ParseOBJ(std::filesystem::path path)
{
	MeshData mesh;

	std::ifstream file(path);
	std::string line;
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return mesh;
	}

	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;

	std::unordered_map<Vertex, u32> uniqueVertices;
	std::vector<Vertex> vertices;
	std::vector<u32> indices;


	while (std::getline(file, line))
	{
		std::istringstream iss(line);
		std::string prefix;
		iss >> prefix;
		if (prefix == "o")
		{
			iss >> mesh.name;
		}
		else if (prefix == "v")
		{
			glm::vec3 position;
			iss >> position.x >> position.y >> position.z;
			positions.push_back(position);
		}
		else if (prefix == "vt")
		{
			glm::vec2 uv;
			iss >> uv.x >> uv.y;
			uvs.push_back(uv);
		}
		else if (prefix == "vn")
		{
			glm::vec3 normal;
			iss >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (prefix == "f")
		{
			std::string face;
			for (int i = 0; i < 3; i++)
			{
				iss >> face;

				std::replace(face.begin(), face.end(), '/', ' ');
				std::istringstream iss2(face);
				int pIndex, tIndex, nIndex;
				iss2 >> pIndex >> tIndex >> nIndex;

				pIndex = (pIndex > 0) ? pIndex - 1 : positions.size() + pIndex;
				tIndex = (tIndex > 0) ? tIndex - 1 : uvs.size() + tIndex;
				nIndex = (nIndex > 0) ? nIndex - 1 : normals.size() + nIndex;

				Vertex vertex;
				vertex.position = positions[pIndex];
				vertex.normal = normals[nIndex];
				vertex.u = uvs[tIndex].x;
				vertex.v = uvs[tIndex].y;

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<u32>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}

	mesh.vertices = vertices;
	mesh.indices = indices;

	file.close();
	return mesh;
}
