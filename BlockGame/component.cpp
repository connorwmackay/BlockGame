#include "component.h"

#include "logging.h"

Component::Component()
{
	
}

void Component::Start()
{
	LOG("Starting component\n");
}

void Component::Update()
{
	LOG("Updating component\n");
}

void TestComponent::Start()
{
	LOG("Starting test component\n");
}

void TestComponent::Update()
{
	LOG("Updating test component\n");
}
