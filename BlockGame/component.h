#pragma once

class Component
{
public:
	Component();
	virtual ~Component() = default;

	virtual void Start();
	virtual void Update();
};

/*
 * A test component.
 */
class TestComponent : public Component
{
public:
	void Start() override;
	void Update() override;
};