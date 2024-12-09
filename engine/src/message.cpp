#include "message.h"

Message::Message(MessageType pType, std::string pMessage)
	: type(pType), message(pMessage)
{
}

Message::~Message()
{
}

StringMessage::StringMessage(std::string pMessage, std::string param, MessageQueue *instigator)
	: Message(MessageType::STRING, pMessage), instigator(instigator), param(param)
{
}

MeshMessage::MeshMessage(std::string pMessage, Renderer::MeshAsset *param, MessageQueue *instigator)
	: Message(MessageType::MESH, pMessage), instigator(instigator), param(param)
{
}
