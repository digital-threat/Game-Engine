#include "sandbox.h"

#include <components.h>
#include <imgui.h>
#include <iostream>
#include <mesh_manager.h>
#include <ostream>
#include <filesystem>
#include <material_manager.h>
#include <renderer_vk_images.h>
#include <texture_manager.h>
#include <utility.h>
#include <vendor/stb/stb_image.h>
#include <mesh_serialization.h>
#include <obj_loading.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

    for (auto entity : mEntityManager.All())
    {
        entity->Update();
    }
}

void MySandbox::Render()
{
	ImGuiApplication();
    ImGuiCamera();
    ImGuiEntities();
    ImGuiMaterials();

    SceneRenderData sceneData{};
    sceneData.cameraPos = mCamera.position;
    sceneData.cameraLookAt = mCamera.lookAt;
    sceneData.cameraFOV = mCamera.fov;
    sceneData.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    mRenderContext.sceneData = sceneData;
    mRenderContext.modelData.clear();
    mRenderContext.lightData.lightCount = 0;
    for (auto entity : mEntityManager.All())
    {
        entity->Render(mRenderContext);
    }
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

void MySandbox::ImGuiEntities()
{
    if (ImGui::Begin("Entities"))
    {
        if (mEntityManager.Count() > 0)
        {
            Entity* selected = mEntityManager.All()[mCurrentEntity];

            ImGui::SliderInt("Entity Index", &mCurrentEntity,0, mEntityManager.Count() - 1);

            selected->OnGUI();

            ImGui::Separator();

            if (ImGui::Button("Delete Entity"))
            {
                mEntityManager.DeleteEntity(selected);
                if (mEntityManager.Count() > 0)
                {
                    mCurrentEntity %= mEntityManager.Count();
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Create Entity"))
        {
            mEntityManager.CreateEntity();
        }

    }
    ImGui::End();
}

void MySandbox::ImGuiMaterials()
{
    static int currentMaterial = 0;
    if (ImGui::Begin("Materials"))
    {
        auto& materialManager = MaterialManager::Get();
        const std::vector<Material>& materials = materialManager.GetAll();

        ImGui::SliderInt("Index", &currentMaterial,0, materials.size() - 1);
        ImGui::Text(materials[currentMaterial].name.c_str());

        if (ImGui::Button("Select Albedo"))
        {
            ImGui::OpenPopup("Albedo Selector");
        }

        if (ImGui::BeginPopup("Albedo Selector"))
        {
            ImGui::Text("TEXTURES:");
            ImGui::Separator();

            for (const auto& file : std::filesystem::directory_iterator("assets/textures/"))
            {
                if (ImGui::Selectable(file.path().filename().string().c_str()))
                {
                    TextureManager& textureManager = TextureManager::Get();
                    VulkanImage* texture = textureManager.GetTexture(file.path().string().c_str());
                    if (texture == nullptr)
                    {
                        try
                        {
                            texture = textureManager.LoadTexture(file.path().string().c_str());
                            Texture albedoTexture = { texture->imageView, nullptr };
                            materialManager.SetTexture(materials[currentMaterial].handle, albedoTexture, 0);

                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << e.what() << std::endl;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Select Specular"))
        {
            ImGui::OpenPopup("Specular Selector");
        }

        if (ImGui::BeginPopup("Specular Selector"))
        {
            ImGui::Text("TEXTURES:");
            ImGui::Separator();

            for (const auto& file : std::filesystem::directory_iterator("assets/textures/"))
            {
                if (ImGui::Selectable(file.path().filename().string().c_str()))
                {
                    TextureManager& textureManager = TextureManager::Get();
                    VulkanImage* texture = textureManager.GetTexture(file.path().string().c_str());
                    if (texture == nullptr)
                    {
                        try
                        {
                            texture = textureManager.LoadTexture(file.path().string().c_str());
                            Texture specularTexture = { texture->imageView, nullptr };
                            materialManager.SetTexture(materials[currentMaterial].handle, specularTexture, 1);

                        }
                        catch (const std::exception& e)
                        {
                            std::cerr << e.what() << std::endl;
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void MySandbox::ImGuiApplication()
{
    if (ImGui::Begin("Application"))
    {
        ImGui::SliderFloat("Render Scale", &mRenderContext.renderScale, 0.3f, 1.0f);
    }
    ImGui::End();
}

void MySandbox::LoadDefaultScene()
{
    TextureManager& textureManager = TextureManager::Get();
    VulkanImage* boxAlbedo = textureManager.GetTexture("assets/textures/container2.png");
    if (boxAlbedo == nullptr)
    {
        try
        {
            boxAlbedo = textureManager.LoadTexture("assets/textures/container2.png");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    VulkanImage* boxSpecular = textureManager.GetTexture("assets/textures/container2_specular.png");
    if (boxSpecular == nullptr)
    {
        try
        {
            boxSpecular = textureManager.LoadTexture("assets/textures/container2_specular.png");
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }

    Texture albedoTexture = { boxAlbedo->imageView, nullptr };
    Texture specularTexture = { boxSpecular->imageView, nullptr };

    MaterialManager& materialManager = MaterialManager::Get();
    auto handle = materialManager.CreateMaterial("Crate");
    materialManager.SetTexture(handle, albedoTexture, 0);
    materialManager.SetTexture(handle, specularTexture, 1);

    materialManager.CreateMaterial("Nothing");


    MeshManager& meshManager = MeshManager::Get();

    for (i32 i = 0; i < 3; ++i)
    {
        Entity &newEntity = mEntityManager.CreateEntity();
        newEntity.mName = "Default Name";

        TransformComponent* transformComponent = new TransformComponent();
        transformComponent->mPosition = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        transformComponent->mRotation = glm::vec3();
        transformComponent->mScale = 1;
        newEntity.AddComponent(transformComponent);

        MeshComponent* meshComponent = new MeshComponent();
        meshComponent->mMaterial = handle;
        newEntity.AddComponent(meshComponent);

        StringMessage* message = new StringMessage("LoadMesh", "assets/meshes/cube.bin", static_cast<MessageQueue *>(meshComponent));
        meshManager.QueueMessage(message);
    }

    Entity &newEntity = mEntityManager.CreateEntity();
    newEntity.mName = "Light";

    LightComponent* lightComponent = new LightComponent();
    newEntity.AddComponent(lightComponent);
}

void MySandbox::ProcessMessage(Message *pMessage)
{
    std::string& message = pMessage->message;
    switch(pMessage->type)
    {

    }
}
