#pragma once

#include <platform.h>

class Context
{
	void Begin();
	void End();
	//void ResourceBarrier(BarrierDescription);
	//(...)

	CommandBuffer mCommandBuffer;
};

class GraphicsContext : public Context
{
	//void SetPipeline(Pipeline);
	void SetVertexBuffer(Buffer);
	void SetIndexBuffer(Buffer);
	//void Draw(...);
	//(...)
};

class ComputeContext : public Context
{
	//void SetPipeline(Pipeline);
	//void Dispatch(...);
	//(...)
};

class UploadContext : public Context
{
	//void UploadBuffer(Buffer, Data);
	//void UploadTexture(Texture, Data);
	//(...)
};

class RayTracingContext : public Context
{

};
