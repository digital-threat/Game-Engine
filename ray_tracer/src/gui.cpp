#include <imgui.h>
#include <sandbox.h>

void Sandbox::ImGuiScene(Scene& scene)
{
	if (ImGui::Begin("Scene"))
	{
		ImGui::SliderInt("Scene", reinterpret_cast<int*>(&mCurrentScene), 0, mScenes.size() - 1);
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
		ImGui::SliderInt("Samples Per Pixel", &mRenderContext.samplesPerPixel, 1, 32);
	}
	ImGui::End();
}
