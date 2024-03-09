#pragma once
#include "component.h"
#include "mesh.h"
#include <glm/glm.hpp>

#include "light.h"

class MeshComponent : public Component 
{
protected:
	Mesh* mesh_;
	glm::mat4 model_;
public:
	MeshComponent();
	MeshComponent(Entity* owner, Mesh* mesh);

	void SetMesh(Mesh* mesh);

	void Draw();
	void SetModel(glm::mat4 const& model);

	void SetDirectionalLight(DirectionalLight const& light);

	Mesh* GetMesh();

	void Start() override;
};
