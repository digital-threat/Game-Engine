#include "message.h"

Message::Message(MessageType pType, std::string pMessage)
	: type(pType), message(pMessage)
{
}

Message::~Message()
{
}

StringMessage::StringMessage(std::string pMessage, std::string param, int entityId, MessageQueue *instigator)
	: Message(MessageType::STRING, pMessage), instigator(instigator), param(param), entityId(entityId)
{
}

MeshMessage::MeshMessage(std::string pMessage, MeshAsset param, int entityId, MessageQueue *instigator)
	: Message(MessageType::MESH, pMessage), instigator(instigator), param(param), entityId(entityId)
{
}
