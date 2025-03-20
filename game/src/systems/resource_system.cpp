#include <components/renderer.h>
#include <systems/resource_system.h>

ResourceSystem::ResourceSystem(Coordinator& coordinator): mCoordinator(coordinator)
{
}

void ResourceSystem::Update()
{
	ProcessMessages();
}

void ResourceSystem::ProcessMessage(Message* message)
{
	std::string& msg = message->message;
	switch(message->type)
	{
		case MessageType::MESH:
		{
			if (msg == "MeshLoaded")
			{
				auto meshMessage = static_cast<MeshMessage*>(message);

				Renderer& renderer = mCoordinator.GetComponent<Renderer>(meshMessage->entity);
				renderer.mesh = new GpuMesh(meshMessage->param);
			}
		} break;
	}
}
