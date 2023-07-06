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
	mesh_->Draw();
}

void MeshComponent::SetModel(glm::mat4 const& model)
{
	mesh_->SetModel(model);
}

void MeshComponent::SetView(glm::vec3 const& viewPos, glm::mat4 const& view)
{
	mesh_->SetViewPos(viewPos);
	mesh_->SetView(view);
}
void MeshComponent::SetProjection(glm::mat4 const& projection)
{
	mesh_->SetProjection(projection);
}

void MeshComponent::SetDirectionalLight(DirectionalLight const& light)
{
	PassDirectionalLightToShader(mesh_->GetShaderProgram(), light);
}

Mesh* MeshComponent::GetMesh()
{
	return mesh_;
}
