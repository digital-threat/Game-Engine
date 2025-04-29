#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <rapidobj/rapidobj.hpp>

template<>
struct std::hash<Vertex>
{
	size_t operator()(const Vertex& v) const
	{
		size_t h1 = std::hash<glm::vec3>()(v.position);
		size_t h2 = std::hash<glm::vec3>()(v.normal);
		size_t h3 = std::hash<glm::vec2>()(v.uv);
		return h1 ^ (h2 << 1) ^ (h3 << 2);
	}
};

inline CpuMesh ParseOBJ(std::filesystem::path path)
{
	CpuMesh mesh;

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
			// iss >> mesh.name;
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
			std::vector<std::string> vertexStrings;
			std::string firstVertex, prevVertex, currVertex;
			if (iss >> firstVertex)
			{
				if (iss >> prevVertex)
				{
					while (iss >> currVertex)
					{
						vertexStrings.insert(vertexStrings.end(), {firstVertex, prevVertex, currVertex});
						prevVertex = currVertex;
					}
				}
			}

			for (auto& vertexString : vertexStrings)
			{
				std::replace(vertexString.begin(), vertexString.end(), '/', ' ');
				std::istringstream iss2(vertexString);
				int pIndex = 0, tIndex = 0, nIndex = 0;

				iss2 >> pIndex;
				if (vertexString.find("  ") != std::string::npos)
				{
					iss >> nIndex;
				}
				else
				{
					iss2 >> tIndex >> nIndex;
				}

				Vertex vertex{};
				if (pIndex != 0)
				{
					pIndex = (pIndex > 0) ? pIndex - 1 : positions.size() + pIndex;
					vertex.position = positions[pIndex];
				}
				if (tIndex != 0)
				{
					tIndex = (tIndex > 0) ? tIndex - 1 : uvs.size() + tIndex;
					vertex.uv = uvs[tIndex];
				}
				if (nIndex != 0)
				{
					nIndex = (nIndex > 0) ? nIndex - 1 : normals.size() + nIndex;
					vertex.normal = normals[nIndex];
				}

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

inline CpuMesh ParseObj(std::filesystem::path path, std::vector<std::string>& textures)
{
	rapidobj::MaterialLibrary mtllib = rapidobj::MaterialLibrary::SearchPaths({"materials", "../materials", "."});
	rapidobj::Result result = rapidobj::ParseFile(path, mtllib);

	if (result.error)
	{
		std::cout << result.error.code.message() << std::endl;
	}

	rapidobj::Triangulate(result);

	CpuMesh mesh{};
	mesh.materials.reserve(result.materials.size());

	for (u32 i = 0; i < result.materials.size(); i++)
	{
		Material material;
		material.ambient = glm::vec3(result.materials[i].ambient[0], result.materials[i].ambient[1], result.materials[i].ambient[2]);
		material.diffuse = glm::vec3(result.materials[i].diffuse[0], result.materials[i].diffuse[1], result.materials[i].diffuse[2]);
		material.specular =
				glm::vec3(result.materials[i].specular[0], result.materials[i].specular[1], result.materials[i].specular[2]);
		material.transmittance = glm::vec3(result.materials[i].transmittance[0], result.materials[i].transmittance[1],
										   result.materials[i].transmittance[2]);
		material.emission =
				glm::vec3(result.materials[i].emission[0], result.materials[i].emission[1], result.materials[i].emission[2]);
		material.shininess = result.materials[i].shininess;
		material.ior = result.materials[i].ior;
		material.dissolve = result.materials[i].dissolve;
		material.illum = result.materials[i].illum;
		if (!result.materials[i].diffuse_texname.empty())
		{
			textures.push_back(result.materials[i].diffuse_texname);
			material.diffuseTextureIndex = static_cast<int>(textures.size()) - 1;
		}

		mesh.materials.push_back(material);
	}

	for (u32 i = 0; i < result.shapes.size(); i++)
	{
		size_t indexCount = result.shapes[i].mesh.indices.size();

		mesh.vertices.reserve(indexCount + mesh.vertices.size());
		mesh.indices.reserve(indexCount + mesh.indices.size());
		mesh.matIds.insert(mesh.matIds.end(), result.shapes[i].mesh.material_ids.begin(), result.shapes[i].mesh.material_ids.end());

		for (u32 j = 0; j < indexCount; j++)
		{
			rapidobj::Index index = result.shapes[i].mesh.indices[j];
			auto positionIndex = index.position_index * 3;
			auto normalIndex = index.normal_index * 3;
			auto uvIndex = index.texcoord_index * 2;

			Vertex vertex{};
			vertex.position.x = result.attributes.positions[positionIndex];
			vertex.position.y = result.attributes.positions[positionIndex + 1];
			vertex.position.z = result.attributes.positions[positionIndex + 2];

			if (!result.attributes.normals.empty() && normalIndex >= 0)
			{
				vertex.normal.x = result.attributes.normals[normalIndex];
				vertex.normal.y = result.attributes.normals[normalIndex + 1];
				vertex.normal.z = result.attributes.normals[normalIndex + 2];
			}

			if (!result.attributes.texcoords.empty() && uvIndex >= 0)
			{
				vertex.uv.x = result.attributes.texcoords[uvIndex];
				vertex.uv.y = result.attributes.texcoords[uvIndex + 1];
			}

			mesh.vertices.push_back(vertex);
			mesh.indices.push_back(mesh.indices.size());
		}
	}

	if (result.attributes.normals.empty())
	{
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			Vertex& v0 = mesh.vertices[mesh.indices[i + 0]];
			Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
			Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

			glm::vec3 n = glm::normalize(glm::cross((v1.position - v0.position), (v2.position - v0.position)));
			v0.normal = n;
			v1.normal = n;
			v2.normal = n;
		}
	}

	return mesh;
}
