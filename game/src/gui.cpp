#include <filesystem>
#include <imgui.h>
#include <iostream>
#include <material_manager.h>
#include <sandbox.h>
#include <texture_manager.h>

void Sandbox::ImGuiMainLight(Scene& scene)
{
	if (ImGui::Begin("Main Light"))
	{
		ImGui::InputFloat3("Position", reinterpret_cast<float*>(&scene.mainLightPosition));
		ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&scene.mainLightColor));
		ImGui::SliderFloat("Intensity", &scene.mainLightIntensity, 0.0f, 1.0f);
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
