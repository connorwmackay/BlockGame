#pragma once
#include "component.h"
#include "mesh.h"
#include <glm/glm.hpp>

#include "light.h"

class MeshComponent : public Component 
{
protected:
	Mesh* mesh_;
public:
	MeshComponent();
	MeshComponent(Entity* owner, Mesh* mesh);

	void SetMesh(Mesh* mesh);

	void Draw();
	void SetModel(glm::mat4 const& model);
	void SetView(glm::vec3 const& viewPos, glm::mat4 const& view);
	void SetProjection(glm::mat4 const& projection);

	void SetDirectionalLight(DirectionalLight const& light);

	Mesh* GetMesh();

	void Start() override;
};
