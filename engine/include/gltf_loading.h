#pragma once

#include <filesystem>

#include "renderer_vk_types.h"

class Engine;

std::vector<MeshAsset*> LoadMeshFromGltf(Engine* pEngine, std::filesystem::path pPath);