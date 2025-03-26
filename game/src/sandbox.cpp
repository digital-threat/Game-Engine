#include <sandbox.h>
#include <engine.h>

#include <iostream>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <mesh_manager.h>
#include <material_manager.h>
#include <vk_images.h>
#include <texture_manager.h>
#include <collision.h>
#include <mesh_serialization.h>
#include <mesh_structs.h>
#include <obj_loading.h>
#include <vk_raytracing.h>
#include <components/renderer.h>
#include <components/transform.h>
#include <components/camera.h>
#include <ecs/coordinator.h>
#include <systems/camera_system.h>
#include <systems/physics_system.h>
#include <systems/render_system.h>

Sandbox::Sandbox(Engine& engine): Application(engine), mResourceSystem(mCoordinator), mRtBuilder(engine)
{
    mRenderContext.renderScale = 1.0f;
    mMainLightColor = glm::vec3(1, 1, 1);
    mMainLightPosition = glm::vec3(0, 5, -10);
    mMainLightIntensity = 1.0f;
    isSimulating = true;
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
            CpuMesh meshData = ParseOBJ(entry.path());
            std::filesystem::path path2 = "assets/meshes/" + meshData.name + ".bin";
            SerializeMesh(meshData, path2);
        }
    }

    LoadDefaultScene();
}

void Sandbox::Update(f64 deltaTime)
{
    mResourceSystem.Update();

    CameraSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext.camera, deltaTime);

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

    mRenderContext.scene.ambientColor = glm::vec3(0.05f, 0.05f, 0.05f);
    mRenderContext.scene.mainLightPos = mMainLightPosition;
    mRenderContext.scene.mainLightColor = glm::vec4(mMainLightColor, mMainLightIntensity);
    mRenderContext.renderObjects.clear();
    mRenderContext.light.lightCount = 0;

    RenderSystem::Update(mCoordinator.mEntityManager, mCoordinator.mComponentManager, mRenderContext);
}

void Sandbox::Destroy()
{
	
}

void Sandbox::CreateBlas()
{
    std::vector<BlasInput> allBlas;

    GpuMesh* cube = MeshManager::Instance().GetMesh("assets/meshes/cube.bin");
    auto blas = MeshToVkGeometryKHR(*cube);
    allBlas.emplace_back(blas);

    mRtBuilder.BuildBlas(allBlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
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


    MeshManager& meshManager = MeshManager::Instance();

    for (i32 i = 0; i < 3; ++i)
    {
        Entity entity = mCoordinator.CreateEntity();

        Transform transform;
        transform.position = glm::vec3(static_cast<float>(i - 1) * 1.5f, 0.0f, 0.0f);
        transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        transform.scale = 1;
        mCoordinator.AddComponent<Transform>(entity, transform);

        Renderer renderer;
        renderer.mesh = meshManager.GetMesh("assets/meshes/cube.bin");
        renderer.material = crateHandle;
        mCoordinator.AddComponent<Renderer>(entity, renderer);
    }

    Entity cameraEntity = mCoordinator.CreateEntity();

    Camera camera{};
    camera.position = glm::vec3(0.0f, 2.0f, -3.0f);
    camera.sensitivity = 0.1f;
    camera.speed = 1.0f;
    camera.yaw = 90.0f;
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
