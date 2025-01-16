#include "message.h"

Message::Message(MessageType pType, std::string pMessage)
	: type(pType), message(pMessage)
{
}

Message::~Message()
{
}

StringMessage::StringMessage(std::string pMessage, std::string param, int entityId, MessageQueue *sender)
	: Message(MessageType::STRING, pMessage), sender(sender), param(param), entityId(entityId)
{
}

MeshMessage::MeshMessage(std::string pMessage, MeshAsset param, int entityId, MessageQueue *sender)
	: Message(MessageType::MESH, pMessage), sender(sender), param(param), entityId(entityId)
{
}
