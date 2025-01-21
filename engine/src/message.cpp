#include "message.h"

Message::Message(MessageType pType, std::string pMessage)
	: type(pType), message(pMessage)
{
}

Message::~Message()
{
}

StringMessage::StringMessage(std::string pMessage, std::string param, MessageQueue *sender)
	: Message(MessageType::STRING, pMessage), sender(sender), param(param)
{
}

MeshMessage::MeshMessage(std::string pMessage, MeshAsset param, MessageQueue *sender)
	: Message(MessageType::MESH, pMessage), sender(sender), param(param)
{
}
