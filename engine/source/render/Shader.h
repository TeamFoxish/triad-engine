#pragma once

#include <string>
#include <memory>
#include <vector>

struct ID3D11Device;
struct D3D11_INPUT_ELEMENT_DESC;
struct D3D11_BUFFER_DESC;
struct ID3D11Buffer;
struct _D3D_SHADER_MACRO;
using D3D_SHADER_MACRO = _D3D_SHADER_MACRO;

struct Shader {
	using PTR = std::shared_ptr<Shader>;

	Shader(
		const std::wstring& path, ID3D11Device* device,
		const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
		const D3D_SHADER_MACRO* macros = 0, int macrosCount = 0);

	Shader(
		const std::wstring& path, ID3D11Device* device,
		const D3D11_INPUT_ELEMENT_DESC* inpDescs, int inpDescsNum,
		const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum,
		const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum,
		const D3D_SHADER_MACRO* macros = nullptr, int macrosCount = 0);

	void Activate(struct ID3D11DeviceContext* context);

	void InitCB(
		ID3D11Device* device,
		const D3D11_BUFFER_DESC* vsCBDescs, int vsCBDescsNum,
		const D3D11_BUFFER_DESC* psCBDescs, int psCBDescsNum);
	void SetCBVS(ID3D11DeviceContext* context, int slot, void* data);
	void SetCBPS(ID3D11DeviceContext* context, int slot, void* data);

	struct ID3D11InputLayout* layout;
	struct ID3D11VertexShader* vertShader = nullptr;
	struct ID3D11PixelShader* pixShader = nullptr;

	std::vector<ID3D11Buffer*> cbVS;
	std::vector<int> cbVSSizes;
	std::vector<ID3D11Buffer*> cbPS;
	std::vector<int> cbPSSizes;
};
