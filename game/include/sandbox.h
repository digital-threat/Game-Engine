#pragma once
#include <application.h>
#include <message_queue.h>

using namespace Renderer;

struct MySandbox : Application, MessageQueue
{
	void Awake() override;
	void Update() override;
	void Render() override;
	void Destroy() override;

	void ImGuiApplication();
	void ImGuiCamera();
	void ImGuiEntity();

	void LoadDefaultScene();

	void ProcessMessage(Message *pMessage) override;
};

