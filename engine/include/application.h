#pragma once

#include <render_context.h>

class Engine;

class Application
{
public:
	Application() = delete;
	explicit Application(Engine& engine);
	virtual void Awake() = 0;
	virtual void Update(f64 deltaTime) = 0;
	virtual void PhysicsUpdate(f64 deltaTime) = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;

public:
	RenderContext mRenderContext;
	Engine& mEngine;
};
