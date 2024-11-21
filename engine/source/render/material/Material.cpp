#include "Material.h"

#include "render/RenderContext.h"
#include "render/Renderer.h"
#include "render/Shader.h"
#include "render/Texture.h"

Material::~Material()
{
	if (!buffer) {
		return;
	}
	buffer->Release();
}

// TODO: replace Renderer param with context ptr?
void Material::Use(RenderContext& ctx)
{
	if (!buffer) {
		const std::vector<Batch> buf = BuildBuffer();
		assert(!buf.empty());
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = bufferSize = (uint32_t)(buf.size() * sizeof(Batch));
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = buf.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		ctx.device->CreateBuffer(&desc, &data, &buffer);
	}
	// currently only pixel shader constant buffers are supported
	ctx->PSSetConstantBuffers(MATERIAL_BIND_SLOT, 1, &buffer);

	// bind textures
	for (const TextureProp& prop : textures) {
		auto tex = prop.tex ? prop.tex->View() : nullptr;
		ctx->PSSetShaderResources(prop.slot, 1, &tex);
	}
}

bool Material::HasBindedTextures() const
{
	for (const TextureProp& prop : textures) {
		if (prop.tex != nullptr) {
			return true;
		}
	}
	return false;
}

std::shared_ptr<Material> Material::CreateChild(const std::shared_ptr<Material>& parent)
{
	auto child = std::make_shared<Material>(*parent);
	child->parent = parent;
	child->buffer = nullptr;
	child->bufferSize = 0;
	return child;
}

std::vector<Material::Batch> Material::BuildBuffer() const
{
	std::vector<Batch> res;
	int size = 0;
	for (int i = 0; i < properties.size(); ++i) {
		const Prop& prop = properties[i];
		prop.AppendTo(res, size);
	}
	return res;
}

auto Material::Prop::ParseType(Strid typeStr) -> Type
{
	constexpr Strid::STORAGE_TYPE INTEGER = "int"_id;
	constexpr Strid::STORAGE_TYPE FLOAT = "float"_id;
	constexpr Strid::STORAGE_TYPE VECTOR4 = "vector4"_id;
	switch (typeStr.hash_code()) {
		case INTEGER:
			return Integer;
		case FLOAT:
			return Float;
		case VECTOR4:
			return Vector4;
		default:
			return Invalid;
	}
}

void Material::Prop::AppendTo(std::vector<Batch>& dest, int& size) const
{
	std::visit(Visitor{dest, size}, val);
}
