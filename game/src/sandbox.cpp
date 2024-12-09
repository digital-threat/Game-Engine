#include "sandbox.h"

#include <imgui.h>
#include <iostream>
#include <mesh_manager.h>
#include <ostream>
#include <renderer_vk_images.h>
#include <texture_manager.h>
#include <utility.h>
#include <vendor/stb/stb_image.h>

void MySandbox::Awake()
{
    LoadDefaultScene();
}

void MySandbox::Update()
{

}

void MySandbox::Render()
{
	ImGuiApplication();
    ImGuiCamera();
    ImGuiEntity();
}

void MySandbox::Destroy()
{
	
}

void MySandbox::ImGuiCamera()
{
    if (ImGui::Begin("Camera"))
    {
        ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&mCamera.position));
        ImGui::InputFloat("FOV", &mCamera.fov);
    }
    ImGui::End();
}

void MySandbox::ImGuiEntity()
{
    if (ImGui::Begin("Transform"))
    {
        if (mEntityManager.Count() > 0)
        {
            Entity* selected = mEntityManager.All()[mCurrentEntity];

            ImGui::SliderInt("Entity Index", &mCurrentEntity,0, mEntityManager.Count() - 1);


            static char nameBuffer[64]{};
            assert(selected->name.size() < 64);
            selected->name.copy(nameBuffer, selected->name.size());
            nameBuffer[selected->name.size()] = '\0';

            if (ImGui::InputText("Name: ", nameBuffer, IM_ARRAYSIZE(nameBuffer)))
            {
                selected->name = std::string(nameBuffer);
            }

            ImGui::InputFloat3("Position:", reinterpret_cast<float *>(&selected->position));
            ImGui::InputFloat3("Rotation:", reinterpret_cast<float *>(&selected->rotation));
            ImGui::InputFloat("Scale", &selected->scale);

            static char meshBuffer[64]{};
            ImGui::InputText("Path to Mesh: ", meshBuffer, IM_ARRAYSIZE(meshBuffer));

            if (ImGui::Button("Set Mesh"))
            {
                std::string path = meshBuffer;
                rtrim(path);
                MeshManager& meshManager = MeshManager::Get();
                MeshAsset* mesh = meshManager.GetMesh(path.c_str());
                if (mesh == nullptr)
                {
                    try
                    {
                        mesh = meshManager.LoadMesh(path.c_str());
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                }

                selected->mesh = mesh;
            }

            if (ImGui::Button("Delete Entity"))
            {
                mEntityManager.DeleteEntity(selected);
                if (mEntityManager.Count() > 0)
                {
                    mCurrentEntity %= mEntityManager.Count();
                }
            }
        }

        if (ImGui::Button("Create Entity"))
        {
            mEntityManager.CreateEntity();
        }

    }
    ImGui::End();
}

void MySandbox::ImGuiApplication()
{
    if (ImGui::Begin("Application"))
    {
        ImGui::SliderFloat("Render Scale", &mRenderScale, 0.3f, 1.0f);
    }
    ImGui::End();
}

void MySandbox::LoadDefaultScene()
{
    MeshManager& meshManager = MeshManager::Get();
    MeshAsset* box = meshManager.GetMesh("assets/meshes/Box.glb");
    if (box == nullptr)
    {
        try
        {
            box = meshManager.LoadMesh("assets/meshes/Box.glb");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    TextureManager& textureManager = TextureManager::Get();
    VulkanImage* defaultTexture = textureManager.GetTexture("assets/textures/texture.jpg");
    if (defaultTexture == nullptr)
    {
        try
        {
            defaultTexture = textureManager.LoadTexture("assets/textures/texture.jpg");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        Entity &newEntity = mEntityManager.CreateEntity();
        newEntity.name = "Default Name";
        newEntity.mesh = box;
        newEntity.texture = defaultTexture;
        newEntity.position = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        newEntity.rotation = glm::vec3();
        newEntity.scale = 1;
    }
}
