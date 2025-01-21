#pragma once

#include <renderer_vk_types.h>
#include <string>
#include "message_type.h"


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
	StringMessage(std::string pMessage, std::string param, MessageQueue *sender = nullptr);

	MessageQueue *sender;
	std::string param;
};

struct MeshMessage : Message
{
	MeshMessage(std::string pMessage, MeshAsset param, MessageQueue *sender = nullptr);

	MessageQueue *sender;
	MeshAsset param;
};
