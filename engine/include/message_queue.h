#pragma once
#include <queue>
#include <message.h>

class MessageQueue
{
public:
	void QueueMessage(Message* pMessage);
	void ProcessMessages();
private:
	virtual void ProcessMessage(Message* pMessage) = 0;

	std::queue<Message*> mMessages;
};
