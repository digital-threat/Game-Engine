#pragma once
#include <application.h>

using namespace Renderer;

struct MySandbox : Application
{
	void Awake() override;
	void Update() override;
	void Render() override;
	void Destroy() override;

	void ImGuiApplication();
	void ImGuiCamera();
	void ImGuiEntity();

	void LoadDefaultScene();
};

