#pragma once

#include <filesystem>

#include "renderer_vk_types.h"

using namespace Renderer;

class Engine;

std::vector<MeshAsset*> LoadGltfMeshes(Engine* pEngine, std::filesystem::path pPath);