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
#include "collision.h"
#include <mesh_serialization.h>
#include <obj_loading.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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

    Ray ray{};
    RayHit hit{};
    ray.direction = glm::vec3(0, 0, 1);
    Raycast(ray, hit);
}

void MySandbox::PhysicsUpdate(f64 deltaTime)
{
    if (mIsSimulating)
    {
        std::vector<ColliderComponent*> colliders;
        for (auto entity : mEntityManager.All())
        {
            auto collider = static_cast<ColliderComponent*>(entity->GetComponent(ComponentType::SPHERE_COLLIDER));
            if (collider != nullptr)
            {
                colliders.push_back(collider);
            }

            collider = static_cast<ColliderComponent*>(entity->GetComponent(ComponentType::BOX_COLLIDER));
            if (collider != nullptr)
            {
                colliders.push_back(collider);
            }
        }

        std::vector<Collision> collisions;

        for (int i = 0; i < colliders.size(); i++)
        {
            for (int j = i + 1; j < colliders.size(); j++)
            {
                Collider& c1 = colliders[i]->GetCollider();
                Collider& c2 = colliders[j]->GetCollider();

                Collision collision { c1, c2 };
                if (CheckIntersection(c1, c2, collision))
                {
                    collisions.push_back(collision);
                }
            }
        }

        // Apply gravity
        for (ColliderComponent* colliderComponent : colliders)
        {
            Collider& collider = colliderComponent->GetCollider();
            if (collider.hasGravity && !collider.isKinematic)
            {
                collider.velocity += glm::vec3(0, mGravity, 0) * static_cast<float>(deltaTime);
            }
        }

        // Handle collisions
        for (Collision collision : collisions)
        {
            if (!collision.c1.isKinematic || !collision.c2.isKinematic)
            {
                glm::vec3 relativeVelocity = collision.c2.velocity - collision.c1.velocity;
                float velocityAlongNormal = glm::dot(relativeVelocity, collision.normal);

                if (velocityAlongNormal < 0)
                {
                    float impulse = (1 + mRestitution) * velocityAlongNormal;

                    if (!collision.c1.isKinematic)
                    {
                        glm::vec3 impulseVector = impulse * collision.normal;
                        collision.c1.velocity += impulseVector;
                    }

                    if (!collision.c2.isKinematic)
                    {
                        glm::vec3 impulseVector = impulse * collision.normal;
                        collision.c2.velocity -= impulseVector;
                    }
                }
            }
        }

        // Apply velocity
        for (ColliderComponent* colliderComponent : colliders)
        {
            Collider& collider = colliderComponent->GetCollider();
            if (!collider.isKinematic)
            {
                auto transform = static_cast<TransformComponent*>(colliderComponent->parent.GetComponent(ComponentType::TRANSFORM));
                transform->mPosition += collider.velocity * static_cast<float>(deltaTime);
            }
        }
    }
}

void MySandbox::Render()
{
	ImGuiApplication();
    ImGuiCamera();
    ImGuiEntities();
    ImGuiMaterials();
    ImGuiMainLight();

    SceneRenderData sceneData{};
    sceneData.cameraPos = mCamera.position;
    sceneData.cameraLookAt = mCamera.lookAt;
    sceneData.cameraFOV = mCamera.fov;
    sceneData.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
    sceneData.mainLightPos = mMainLightPosition;
    sceneData.mainLightColor = glm::vec4(mMainLightColor, mMainLightIntensity);
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
                            Texture albedoTexture = { texture->imageView, VK_NULL_HANDLE };
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
                            Texture specularTexture = { texture->imageView, VK_NULL_HANDLE };
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

        if (ImGui::Button("Select Sampler"))
        {
            ImGui::OpenPopup("Sampler Selector");
        }

        if (ImGui::BeginPopup("Sampler Selector"))
        {
            ImGui::Text("SAMPLERS:");
            ImGui::Separator();

            TextureManager& textureManager = TextureManager::Get();
            for (auto& sampler : textureManager.GetSamplers())
            {
                if (ImGui::Selectable(sampler.first.c_str()))
                {
                    Texture diffuseTexture = { VK_NULL_HANDLE, sampler.second };
                    Texture specularTexture = { VK_NULL_HANDLE, sampler.second };
                    materialManager.SetTexture(materials[currentMaterial].handle, diffuseTexture, 0);
                    materialManager.SetTexture(materials[currentMaterial].handle, specularTexture, 1);
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void MySandbox::ImGuiMainLight()
{
    if (ImGui::Begin("Main Light"))
    {
        ImGui::InputFloat3("Position", reinterpret_cast<float *>(&mMainLightPosition));
        ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&mMainLightColor));
        ImGui::SliderFloat("Intensity", &mMainLightIntensity, 0.0f, 1.0f);
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

    Texture albedoTexture = { boxAlbedo->imageView, textureManager.GetSampler("LINEAR_MIPMAP_LINEAR") };
    Texture specularTexture = { boxSpecular->imageView, textureManager.GetSampler("LINEAR_MIPMAP_LINEAR") };

    MaterialManager& materialManager = MaterialManager::Get();
    MaterialHandle crateHandle = materialManager.CreateMaterial("Crate");
    materialManager.SetTexture(crateHandle, albedoTexture, 0);
    materialManager.SetTexture(crateHandle, specularTexture, 1);

    MaterialHandle whiteHandle = materialManager.CreateMaterial("White");
    VulkanImage* whiteImage = textureManager.LoadTexture("assets/textures/white.png");
    Texture whiteTexture = { whiteImage->imageView, textureManager.GetSampler("LINEAR_MIPMAP_LINEAR") };
    materialManager.SetTexture(whiteHandle, whiteTexture, 0);
    materialManager.SetTexture(whiteHandle, whiteTexture, 1);


    MeshManager& meshManager = MeshManager::Get();

    for (i32 i = 0; i < 3; ++i)
    {
        Entity &newEntity = mEntityManager.CreateEntity();
        newEntity.mName = "Default Name";

        TransformComponent* transformComponent = new TransformComponent(newEntity);
        transformComponent->mPosition = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        transformComponent->mRotation = glm::vec3();
        transformComponent->mScale = 1;
        newEntity.AddComponent(transformComponent);

        BoxColliderComponent* boxColliderComponent = new BoxColliderComponent(newEntity);
        newEntity.AddComponent(boxColliderComponent);

        MeshComponent* meshComponent = new MeshComponent(newEntity);
        meshComponent->mMaterial = crateHandle;
        newEntity.AddComponent(meshComponent);

        StringMessage* message = new StringMessage("LoadMesh", "assets/meshes/cube.bin", static_cast<MessageQueue *>(meshComponent));
        meshManager.QueueMessage(message);
    }

    // Entity &newEntity = mEntityManager.CreateEntity();
    // newEntity.mName = "Light";
    //
    // LightComponent* lightComponent = new LightComponent(newEntity);
    // newEntity.AddComponent(lightComponent);
}

bool MySandbox::Raycast(Ray &ray, RayHit& hit)
{
    std::vector<ColliderComponent*> colliders;
    for (auto entity : mEntityManager.All())
    {
        auto collider = static_cast<ColliderComponent*>(entity->GetComponent(ComponentType::SPHERE_COLLIDER));
        if (collider != nullptr)
        {
            colliders.push_back(collider);
        }

        collider = static_cast<ColliderComponent*>(entity->GetComponent(ComponentType::BOX_COLLIDER));
        if (collider != nullptr)
        {
            colliders.push_back(collider);
        }
    }

    for (ColliderComponent* collider : colliders)
    {
        Collider& c = collider->GetCollider();
        if (CheckRayIntersection(ray, hit, c))
        {
            return true;
        }
    }

    return false;
}

void MySandbox::ProcessMessage(Message *pMessage)
{
    std::string& message = pMessage->message;
    switch(pMessage->type)
    {

    }
}
