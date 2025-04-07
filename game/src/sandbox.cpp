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
#include <rapidobj/rapidobj.hpp>

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

    rapidobj::Result result = rapidobj::ParseFile("assets/meshes/cube.obj", rapidobj::MaterialLibrary::Ignore());
    rapidobj::Triangulate(result);

    for (u32 i = 0; i < result.materials.size(); i++)
    {
        CpuMaterial material;
        material.ambient = glm::vec3(result.materials[i].ambient[0], result.materials[i].ambient[1], result.materials[i].ambient[2]);
        material.diffuse = glm::vec3(result.materials[i].diffuse[0], result.materials[i].diffuse[1], result.materials[i].diffuse[2]);
        material.specular = glm::vec3(result.materials[i].specular[0], result.materials[i].specular[1], result.materials[i].specular[2]);
        material.transmittance = glm::vec3(result.materials[i].transmittance[0], result.materials[i].transmittance[1], result.materials[i].transmittance[2]);
        material.emission = glm::vec3(result.materials[i].emission[0], result.materials[i].emission[1], result.materials[i].emission[2]);
        material.shininess = result.materials[i].shininess;
        material.ior = result.materials[i].ior;
        material.dissolve = result.materials[i].dissolve;
        material.illum = result.materials[i].illum;
        material.diffuseTexture = result.materials[i].diffuse_texname;

    }

    for (u32 i = 0; i < result.shapes.size(); i++)
    {
        size_t indexCount = result.shapes[i].mesh.indices.size();
        size_t matIdCount = result.shapes[i].mesh.material_ids.size();

        CpuMesh mesh{};
        mesh.name = result.shapes[i].name;
        mesh.vertices.reserve(indexCount);
        mesh.indices.reserve(indexCount);
        for (u32 j = 0; j < indexCount; j++)
        {
            rapidobj::Index index = result.shapes[i].mesh.indices[j];
            auto positionIndex = index.position_index * 3;
            auto normalIndex = index.normal_index * 3;
            auto uvIndex = index.texcoord_index * 2;

            Vertex vertex{};
            vertex.position.x = result.attributes.positions[positionIndex];
            vertex.position.y = result.attributes.positions[positionIndex + 1];
            vertex.position.z = result.attributes.positions[positionIndex + 2];
            vertex.normal.x = result.attributes.normals[normalIndex];
            vertex.normal.y = result.attributes.normals[normalIndex + 1];
            vertex.normal.z = result.attributes.normals[normalIndex + 2];
            vertex.uv.x = result.attributes.texcoords[uvIndex];
            vertex.uv.y = result.attributes.texcoords[uvIndex + 1];

            mesh.vertices.push_back(vertex);
            mesh.indices.push_back(j);
        }

        mesh.matIds.reserve(matIdCount);
        for (u32 j = 0; j < matIdCount; j++)
        {
            mesh.matIds.push_back(result.shapes[i].mesh.material_ids[j]);
        }

        std::filesystem::path path = "assets/meshes/" + mesh.name + ".bin";
        SerializeMesh(mesh, path);
    }

    CreateBlas();
    CreateTlas();

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

void Sandbox::CreateTlas()
{
    std::vector<VkAccelerationStructureInstanceKHR> tlas;

    Transform transform;
    transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    transform.scale = 1;

    glm::mat4 matrixM = glm::translate(glm::mat4(1.0f), transform.position);
    matrixM *= glm::mat4_cast(transform.rotation);
    matrixM = glm::scale(matrixM, glm::vec3(transform.scale));

    VkAccelerationStructureInstanceKHR instance{};
    instance.transform = ToVkTransformMatrixKHR(matrixM);
    instance.instanceCustomIndex = 0;
    instance.accelerationStructureReference = mRtBuilder.GetBlasDeviceAddress(0);
    instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    instance.mask = 0xFF;
    instance.instanceShaderBindingTableRecordOffset = 0;
    tlas.emplace_back(instance);

    mRtBuilder.BuildTlas(tlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
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
    materialManager.SetTexture(crateHandle, albedoTexture, 1);
    materialManager.SetTexture(crateHandle, specularTexture, 2);

    MaterialHandle whiteHandle = materialManager.CreateMaterial("White");
    VulkanImage* whiteImage = textureManager.LoadTexture("assets/textures/white.png");
    Texture whiteTexture = { whiteImage->imageView, textureManager.GetSampler("LINEAR_MIPMAP_LINEAR") };
    materialManager.SetTexture(whiteHandle, whiteTexture, 1);
    materialManager.SetTexture(whiteHandle, whiteTexture, 2);


    MeshManager& meshManager = MeshManager::Instance();

    for (i32 i = 0; i < 3; ++i)
    {
        Entity entity = mCoordinator.CreateEntity();

        Transform transform;
        transform.position = glm::vec3(static_cast<float>(i - 1) * 1.5f, i, 0.0f);
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
