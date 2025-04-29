#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <material_manager.h>
#include <sandbox.h>
#include <texture_manager.h>

void Sandbox::ImGuiScene(Scene& scene)
{
	if (ImGui::Begin("Scene"))
	{
		ImGui::InputFloat3("Sun Position", reinterpret_cast<float*>(&scene.mainLightPosition));
		ImGui::ColorEdit3("Sun Color", reinterpret_cast<float*>(&scene.mainLightColor));
		ImGui::SliderFloat("Sun Intensity", &scene.mainLightIntensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("Sky Color", reinterpret_cast<float*>(&scene.skyColor));
	}
	ImGui::End();
}

void Sandbox::ImGuiApplication()
{
	if (ImGui::Begin("Application"))
	{
		ImGui::SliderInt("Samples Per Pixel", &mRenderContext.samplesPerPixel, 1, 16);
	}
	ImGui::End();
}
