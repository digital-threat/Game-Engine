#include "message_queue.h"

void MessageQueue::QueueMessage(Message *pMessage)
{
	mMutex.lock();
	mMessages.push(pMessage);
	mMutex.unlock();
}

void MessageQueue::ProcessMessages()
{
	mMutex.lock();
	while (mMessages.size() > 0)
	{
		Message* message = mMessages.front();
		ProcessMessage(message);
		mMessages.pop();
		delete message;
	}
	mMutex.unlock();
}
