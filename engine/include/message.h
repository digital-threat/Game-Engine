#pragma once

#include <message_type.h>
#include <mesh_structs.h>
#include <ecs/typedefs.h>
#include <string>


class MessageQueue;

struct Message
{
	Message(MessageType pType, std::string pMessage);
	virtual ~Message();

	MessageType type;
	std::string message;
};

struct StringMessage : Message
{
	StringMessage(std::string pMessage, std::string param, Entity entity, MessageQueue *sender = nullptr);

	MessageQueue *sender;
	Entity entity;
	std::string param;
};

struct MeshMessage : Message
{
	MeshMessage(std::string pMessage, Mesh param, Entity entity, MessageQueue *sender = nullptr);

	MessageQueue *sender;
	Entity entity;
	Mesh param;
};
