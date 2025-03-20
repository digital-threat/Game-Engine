#include "message.h"

Message::Message(MessageType pType, std::string pMessage)
	: type(pType), message(pMessage)
{
}

Message::~Message()
{
}

StringMessage::StringMessage(std::string pMessage, std::string param, Entity entity, MessageQueue *sender)
	: Message(MessageType::STRING, pMessage), sender(sender), entity(entity), param(param)
{
}

MeshMessage::MeshMessage(std::string pMessage, GpuMesh param, Entity entity, MessageQueue *sender)
	: Message(MessageType::MESH, pMessage), sender(sender), entity(entity), param(param)
{
}
