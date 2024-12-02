#include "gltf_loading.h"

#include <vendor/stb/stb_image.h>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>

#include "engine.h"
#include "renderer_vk_structures.h"
#include "renderer_vk_types.h"

#include <iostream>


std::vector<MeshAsset*> LoadGltfMeshes(Engine *pEngine, std::filesystem::path pPath)
{
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(pPath);
    if (data.error() != fastgltf::Error::None)
    {
    	throw std::runtime_error("Failed to load glTF data from path: " + pPath.string());
    }

    auto asset = parser.loadGltf(data.get(), pPath.parent_path(), fastgltf::Options::None);
    if (auto error = asset.error(); error != fastgltf::Error::None)
    {
        throw std::runtime_error("Failed to load glTF asset: " + static_cast<int>(error));
    }

#ifndef NDEBUG
	fastgltf::validate(asset.get());
#endif

    std::vector<MeshAsset*> meshes;

    std::vector<u32> indices;
    std::vector<Vertex> vertices;
    for (fastgltf::Mesh& mesh : asset->meshes)
    {
        indices.clear();
        vertices.clear();

        MeshAsset* newMesh = new MeshAsset();
        newMesh->name = mesh.name;

        for (auto&& primitive : mesh.primitives)
        {
            Submesh newSubmesh;
            newSubmesh.startIndex = indices.size();
            newSubmesh.count = asset->accessors[primitive.indicesAccessor.value()].count;

            size_t initial_vtx = vertices.size();

            // Load indices
            {
                fastgltf::Accessor& indexAccessor = asset->accessors[primitive.indicesAccessor.value()];
                indices.reserve(indices.size() + indexAccessor.count);

                auto func = [&](u32 idx)
                {
                    indices.push_back(idx + initial_vtx);
                };

                fastgltf::iterateAccessor<u32>(asset.get(), indexAccessor, func);
            }

            // Load vertex positions
            {
                fastgltf::Attribute* positionIt = primitive.findAttribute("POSITION");
                fastgltf::Accessor& positionAccessor = asset->accessors[positionIt->accessorIndex];

                vertices.resize(vertices.size() + positionAccessor.count);

                auto func = [&](glm::vec3 position, size_t index)
                {
                    Vertex newVertex;
                    newVertex.position = position;
                    newVertex.normal = { 1, 0, 0 };
                    newVertex.color = glm::vec4 { 1.f };
                    newVertex.uvX = 0;
                    newVertex.uvY = 0;
                    vertices[initial_vtx + index] = newVertex;
                };

                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), positionAccessor, func);
            }

            // Load vertex normals
            {
                auto attribute = primitive.findAttribute("NORMAL");
                if (attribute != primitive.attributes.end())
                {
                    auto func = [&](glm::vec3 normal, size_t index)
                    {
                        vertices[initial_vtx + index].normal = normal;
                    };

                    fastgltf::iterateAccessorWithIndex<glm::vec3>(asset.get(), asset->accessors[attribute->accessorIndex], func);
                }
            }


            // Load UVs
            {
                auto attribute = primitive.findAttribute("TEXCOORD_0");
                if (attribute != primitive.attributes.end())
                {
                    auto func = [&](glm::vec2 uv, size_t index)
                    {
                        vertices[initial_vtx + index].uvX = uv.x;
                        vertices[initial_vtx + index].uvY = uv.y;
                    };

                    fastgltf::iterateAccessorWithIndex<glm::vec2>(asset.get(), asset->accessors[attribute->accessorIndex], func);
                }
            }

            // Load vertex colors
            {
                auto attribute = primitive.findAttribute("COLOR_0");
                if (attribute != primitive.attributes.end())
                {
                    auto func = [&](glm::vec4 color, size_t index)
                    {
                        vertices[initial_vtx + index].color = color;
                    };

                    fastgltf::iterateAccessorWithIndex<glm::vec4>(asset.get(), asset->accessors[attribute->accessorIndex], func);
                }
            }

            newMesh->submeshes.push_back(newSubmesh);
        }


        newMesh->meshBuffers = pEngine->UploadMesh(indices, vertices);

        meshes.emplace_back(newMesh);
    }

    return meshes;
}
