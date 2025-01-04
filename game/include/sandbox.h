#pragma once
#include <application.h>
#include <message_queue.h>

struct MySandbox : Application, public MessageQueue
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

