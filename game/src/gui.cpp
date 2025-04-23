#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <material_manager.h>
#include <sandbox.h>
#include <texture_manager.h>

// void Sandbox::ImGuiMaterials()
// {
//     static int currentMaterial = 0;
//     if (ImGui::Begin("Materials"))
//     {
//         auto& materialManager = MaterialManager::Get();
//         const std::vector<Material_>& materials = materialManager.GetAll();
//
//         ImGui::SliderInt("Index", &currentMaterial,0, materials.size() - 1);
//         ImGui::Text(materials[currentMaterial].name.c_str());
//
//         if (ImGui::Button("Select Albedo"))
//         {
//             ImGui::OpenPopup("Albedo Selector");
//         }
//
//         if (ImGui::BeginPopup("Albedo Selector"))
//         {
//             ImGui::Text("TEXTURES:");
//             ImGui::Separator();
//
//             for (const auto& file : std::filesystem::directory_iterator("assets/textures/"))
//             {
//                 if (ImGui::Selectable(file.path().filename().string().c_str()))
//                 {
//                     TextureManager& textureManager = TextureManager::Instance();
//                     VulkanImage* texture = textureManager.GetTexture(file.path().string().c_str());
//                     if (texture == nullptr)
//                     {
//                         try
//                         {
//                             texture = textureManager.LoadTexture(file.path().string().c_str());
//                             Texture albedoTexture = { texture->imageView, VK_NULL_HANDLE };
//                             materialManager.SetTexture(materials[currentMaterial].handle, albedoTexture, 1);
//
//                         }
//                         catch (const std::exception& e)
//                         {
//                             std::cerr << e.what() << std::endl;
//                         }
//                     }
//                     ImGui::CloseCurrentPopup();
//                 }
//             }
//
//             ImGui::EndPopup();
//         }
//
//         if (ImGui::Button("Select Specular"))
//         {
//             ImGui::OpenPopup("Specular Selector");
//         }
//
//         if (ImGui::BeginPopup("Specular Selector"))
//         {
//             ImGui::Text("TEXTURES:");
//             ImGui::Separator();
//
//             for (const auto& file : std::filesystem::directory_iterator("assets/textures/"))
//             {
//                 if (ImGui::Selectable(file.path().filename().string().c_str()))
//                 {
//                     TextureManager& textureManager = TextureManager::Instance();
//                     VulkanImage* texture = textureManager.GetTexture(file.path().string().c_str());
//                     if (texture == nullptr)
//                     {
//                         try
//                         {
//                             texture = textureManager.LoadTexture(file.path().string().c_str());
//                             Texture specularTexture = { texture->imageView, VK_NULL_HANDLE };
//                             materialManager.SetTexture(materials[currentMaterial].handle, specularTexture, 2);
//
//                         }
//                         catch (const std::exception& e)
//                         {
//                             std::cerr << e.what() << std::endl;
//                         }
//                     }
//                     ImGui::CloseCurrentPopup();
//                 }
//             }
//
//             ImGui::EndPopup();
//         }
//
//         if (ImGui::Button("Select Sampler"))
//         {
//             ImGui::OpenPopup("Sampler Selector");
//         }
//
//         if (ImGui::BeginPopup("Sampler Selector"))
//         {
//             ImGui::Text("SAMPLERS:");
//             ImGui::Separator();
//
//             TextureManager& textureManager = TextureManager::Instance();
//             for (auto& sampler : textureManager.GetSamplers())
//             {
//                 if (ImGui::Selectable(sampler.first.c_str()))
//                 {
//                     Texture diffuseTexture = { VK_NULL_HANDLE, sampler.second };
//                     Texture specularTexture = { VK_NULL_HANDLE, sampler.second };
//                     materialManager.SetTexture(materials[currentMaterial].handle, diffuseTexture, 1);
//                     materialManager.SetTexture(materials[currentMaterial].handle, specularTexture, 2);
//                     ImGui::CloseCurrentPopup();
//                 }
//             }
//
//             ImGui::EndPopup();
//         }
//     }
//     ImGui::End();
// }

void Sandbox::ImGuiMainLight()
{
	if (ImGui::Begin("Main Light"))
	{
		ImGui::InputFloat3("Position", reinterpret_cast<float*>(&mMainLightPosition));
		ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&mMainLightColor));
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
