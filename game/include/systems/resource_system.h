#pragma once

#include <message_queue.h>
#include <ecs/coordinator.h>

class EntityManager;
class ComponentManager;

class ResourceSystem : public MessageQueue
{
public:
	ResourceSystem(Coordinator& coordinator);
	void Update();

private:
	void ProcessMessage(Message *message) override;

private:
	Coordinator& mCoordinator;
};