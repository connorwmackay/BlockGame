#include "meshComponent.h"

#include "entity.h"
#include "transformComponent.h"

MeshComponent::MeshComponent()
{
	mesh_ = nullptr;
}

MeshComponent::MeshComponent(Entity* owner, Mesh* mesh)
	: Component(owner)
{
	SetMesh(mesh);
}

void MeshComponent::SetMesh(Mesh* mesh)
{
	mesh_ = mesh;
}

void MeshComponent::Start()
{
}

void MeshComponent::Draw()
{
	mesh_->Draw(model_);
}

void MeshComponent::SetModel(glm::mat4 const& model)
{
	model_ = model;
	mesh_->SetModel(model);
}

void MeshComponent::SetDirectionalLight(DirectionalLight const& light)
{
	PassDirectionalLightToShader(mesh_->GetShaderProgram(), light);
}

Mesh* MeshComponent::GetMesh()
{
	return mesh_;
}
