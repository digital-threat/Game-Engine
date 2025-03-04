#pragma once

#include <renderer_vk_types.h>

class Application
{
public:
	virtual void Awake() = 0;
	virtual void Update() = 0;
	virtual void PhysicsUpdate(f64 deltaTime) = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;

public:
	RenderContext mRenderContext{};
};
