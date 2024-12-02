#pragma once

#include <optional>
#include <memory>
#include <filesystem>

#include "renderer_vk_types.h"

using namespace Renderer;

class Engine;

std::vector<std::shared_ptr<MeshAsset>> LoadGltfMeshes(Engine* pEngine, std::filesystem::path pPath);