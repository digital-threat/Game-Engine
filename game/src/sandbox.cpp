#include <sandbox.h>

#include <iostream>
#include <filesystem>

#include <imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <mesh_manager.h>
#include <material_manager.h>
#include <renderer_vk_images.h>
#include <texture_manager.h>
#include <utility.h>
#include <collision.h>
#include <mesh_serialization.h>
#include <mesh_structs.h>
#include <obj_loading.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <components/camera.h>
#include <ecs/coordinator.h>
#include <systems/camera_system.h>
#include <systems/physics_system.h>
#include <systems/render_system.h>

Sandbox::Sandbox(): mResourceSystem(mCoordinator)
{
}

void Sandbox::Awake()
{
    mCoordinator.RegisterComponent<Transform>();
    mCoordinator.RegisterComponent<Renderer>();
    mCoordinator.RegisterComponent<SphereCollider>();
    mCoordinator.RegisterComponent<BoxCollider>();
    mCoordinator.RegisterComponent<Camera>();

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

void Sandbox::Update()
{
    ProcessMessages();

    mResourceSystem.Update();

    CameraSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext.sceneData);

    Ray ray{};
    RayHit hit{};
    ray.direction = glm::vec3(0, 0, 1);
    Raycast(ray, hit);
}

void Sandbox::PhysicsUpdate(f64 deltaTime)
{
    if (isSimulating)
    {
        PhysicsSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, deltaTime);
    }
}

void Sandbox::Render()
{
    CameraSystem::OnGUI(mCoordinator.mEntityManager, mCoordinator.mComponentManager);

	ImGuiApplication();
    //ImGuiEntities();
    ImGuiMaterials();
    ImGuiMainLight();

    mRenderContext.sceneData.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
    mRenderContext.sceneData.mainLightPos = mMainLightPosition;
    mRenderContext.sceneData.mainLightColor = glm::vec4(mMainLightColor, mMainLightIntensity);
    mRenderContext.renderObjects.clear();
    mRenderContext.lightData.lightCount = 0;

    RenderSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext);
}

void Sandbox::Destroy()
{
	
}

// void MySandbox::ImGuiEntities()
// {
//     if (ImGui::Begin("Entities"))
//     {
//         if (mEntityManager.Count() > 0)
//         {
//             Entity* selected = mEntityManager.All()[mCurrentEntity];
//
//             ImGui::SliderInt("Entity Index", &mCurrentEntity,0, mEntityManager.Count() - 1);
//
//             selected->OnGUI();
//
//             ImGui::Separator();
//
//             if (ImGui::Button("Delete Entity"))
//             {
//                 mEntityManager.DeleteEntity(selected);
//                 if (mEntityManager.Count() > 0)
//                 {
//                     mCurrentEntity %= mEntityManager.Count();
//                 }
//             }
//         }
//
//         ImGui::SameLine();
//
//         if (ImGui::Button("Create Entity"))
//         {
//             mEntityManager.CreateEntity();
//         }
//
//     }
//     ImGui::End();
// }

void Sandbox::ImGuiMaterials()
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

void Sandbox::ImGuiMainLight()
{
    if (ImGui::Begin("Main Light"))
    {
        ImGui::InputFloat3("Position", reinterpret_cast<float *>(&mMainLightPosition));
        ImGui::ColorEdit3("Color", reinterpret_cast<float *>(&mMainLightColor));
        ImGui::SliderFloat("Intensity", &mMainLightIntensity, 0.0f, 1.0f);
    }
    ImGui::End();
}

void Sandbox::ImGuiApplication()
{
    if (ImGui::Begin("Application"))
    {
        ImGui::SliderFloat("Render Scale", &mRenderContext.renderScale, 0.3f, 1.0f);
    }
    ImGui::End();
}

void Sandbox::LoadDefaultScene()
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
        Entity entity = mCoordinator.CreateEntity();

        Transform transform;
        transform.position = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        transform.scale = 1;
        mCoordinator.AddComponent<Transform>(entity, transform);

        Renderer renderer;
        renderer.mesh = nullptr;
        renderer.material = crateHandle;
        mCoordinator.AddComponent<Renderer>(entity, renderer);

        StringMessage* message = new StringMessage("LoadMesh", "assets/meshes/cube.bin", entity, static_cast<MessageQueue *>(&mResourceSystem));
        meshManager.QueueMessage(message);
    }

    Entity cameraEntity = mCoordinator.CreateEntity();

    Camera camera;
    camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
    camera.lookAt = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.fov = 60.0f;
    mCoordinator.AddComponent<Camera>(cameraEntity, camera);
}

bool Sandbox::Raycast(Ray &ray, RayHit& hit)
{
    auto sphereColliders = mCoordinator.mComponentManager.GetComponentsOfType<SphereCollider>();
    auto boxColliders = mCoordinator.mComponentManager.GetComponentsOfType<BoxCollider>();

    std::vector<Collision> collisions;

    for (u32 i = 0; i < sphereColliders.mSize; i++)
    {
        SphereCollider& c = sphereColliders.mComponentArray[i];
        if (IntersectRaySphere(ray, hit, c))
        {
            return true;
        }
    }

    for (u32 i = 0; i < boxColliders.mSize; i++)
    {
        BoxCollider& c = boxColliders.mComponentArray[i];
        if (IntersectRayOBB(ray, hit, c))
        {
            return true;
        }
    }

    return false;
}

void Sandbox::ProcessMessage(Message *pMessage)
{
    std::string& message = pMessage->message;
    switch(pMessage->type)
    {

    }
}
