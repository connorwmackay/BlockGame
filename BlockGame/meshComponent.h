#pragma once
#include "component.h"
#include "mesh.h"
#include <glm/glm.hpp>

class MeshComponent : public Component 
{
protected:
	Mesh mesh_;
public:
	MeshComponent();
	MeshComponent(Entity* owner, Mesh& mesh);

	void SetMesh(Mesh& mesh);

	void Draw();
	void SetModel(glm::mat4 const& model);
	void SetView(glm::mat4 const& view);
	void SetProjection(glm::mat4 const& projection);

	Mesh* GetMesh();

	void Start() override;
};
