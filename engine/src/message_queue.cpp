#include "message_queue.h"

void MessageQueue::QueueMessage(Message *pMessage)
{
	mMessages.push(pMessage);
}

void MessageQueue::ProcessMessages()
{
	while (mMessages.size() > 0)
	{
		Message* message = mMessages.front();
		ProcessMessage(message);
		mMessages.pop();
		delete message;
	}
}
