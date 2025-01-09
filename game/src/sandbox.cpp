#include "sandbox.h"

#include <imgui.h>
#include <iostream>
#include <mesh_manager.h>
#include <ostream>
#include <filesystem>
#include <renderer_vk_images.h>
#include <texture_manager.h>
#include <utility.h>
#include <vendor/stb/stb_image.h>
#include <mesh_serialization.h>
#include <obj_loading.h>

void MySandbox::Awake()
{
    std::filesystem::path path = "assets/meshes/";
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().extension() == ".obj")
        {
            MeshData meshData = ParseOBJ(entry.path());
            std::filesystem::path path2 = "assets/meshes/" + meshData.name + ".bin";
            SerializeMesh(meshData, path2);
        }
    }

    LoadDefaultScene();
}

void MySandbox::Update()
{
    ProcessMessages();
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
        ImGui::InputFloat3("Look At:", reinterpret_cast<float *>(&mCamera.lookAt));
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

            static char textureBuffer[64]{};
            ImGui::InputText("Path to Texture: ", textureBuffer, IM_ARRAYSIZE(textureBuffer));

            if (ImGui::Button("Set Mesh"))
            {
                std::string path = meshBuffer;
                rtrim(path);
                MeshManager& meshManager = MeshManager::Get();
                StringMessage* message = new StringMessage("LoadMesh", path.c_str(), selected->id, this);
                meshManager.QueueMessage(message);
            }

            if (ImGui::Button("Set Texture"))
            {
                std::string path = textureBuffer;
                rtrim(path);
                TextureManager& textureManager = TextureManager::Get();
                VulkanImage* texture = textureManager.GetTexture(path.c_str());
                if (texture == nullptr)
                {
                    try
                    {
                        texture = textureManager.LoadTexture(path.c_str());
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << std::endl;
                    }
                }

                selected->texture = texture;
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
    TextureManager& textureManager = TextureManager::Get();
    VulkanImage* defaultTexture = textureManager.GetTexture("assets/textures/brick.png");
    if (defaultTexture == nullptr)
    {
        try
        {
            defaultTexture = textureManager.LoadTexture("assets/textures/brick.png");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    MeshManager& meshManager = MeshManager::Get();

    for (i32 i = 0; i < 3; ++i)
    {
        Entity &newEntity = mEntityManager.CreateEntity();
        newEntity.name = "Default Name";
        newEntity.mesh = nullptr;
        newEntity.texture = defaultTexture;
        newEntity.position = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        newEntity.rotation = glm::vec3();
        newEntity.scale = 1;

        StringMessage* message = new StringMessage("LoadMesh", "assets/meshes/cube.bin", newEntity.id, this);
        meshManager.QueueMessage(message);
    }
}

void MySandbox::ProcessMessage(Message *pMessage)
{
    std::string& message = pMessage->message;
    switch(pMessage->type)
    {
        case MessageType::MESH:
        {
            if (message == "MeshLoaded")
            {
                auto meshMessage = static_cast<MeshMessage *>(pMessage);

                Entity *entity = mEntityManager.GetById(meshMessage->entityId);
                if (entity != nullptr)
                {
                    entity->mesh = new MeshAsset(meshMessage->param);
                }
            }
        } break;
        default:
        {

        } break;
    }
}
