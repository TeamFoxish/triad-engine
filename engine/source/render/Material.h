#pragma once

#include <memory>

struct Shader;

class Material {
public:
	Material(std::shared_ptr<Shader> shader);

	virtual void Use(class Renderer* renderer);

	virtual int GetCBVSSize() const = 0;
	virtual int GetCBPSSize() const = 0;

	Shader& GetShader() { return *shader.get(); }

protected:
	std::shared_ptr<Shader> shader;
};
