#pragma once

#include <fstream>
#include <renderer_vk_types.h>

bool SerializeMesh(const MeshData &meshData, std::filesystem::path &path);

bool DeserializeMesh(std::filesystem::path &path, MeshData &outMeshData);
