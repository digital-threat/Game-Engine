#pragma once

#include <fstream>
#include <mesh_structs.h>

bool SerializeMesh(const CpuMesh &mesh, std::filesystem::path &path);

bool DeserializeMesh(std::filesystem::path &path, CpuMesh &outMesh);
