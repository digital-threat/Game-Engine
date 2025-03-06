#pragma once

#include <fstream>
#include <mesh_structs.h>

bool SerializeMesh(const MeshData &meshData, std::filesystem::path &path);

bool DeserializeMesh(std::filesystem::path &path, MeshData &outMeshData);
