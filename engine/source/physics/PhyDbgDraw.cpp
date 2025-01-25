#include "PhyDbgDraw.h"

#include "render/RenderSystem.h"
#include "render/mesh/MeshRenderer.h"


MyDebugDraw::MyDebugDraw()
    : m_device(gRenderSys->GetContext().device), m_context(gRenderSys->GetContext().context),
    m_depthEnabled(true), m_pointSize(1.f), m_lineWidth(5.f)
{
    const D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        D3D11_INPUT_ELEMENT_DESC {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
        D3D11_INPUT_ELEMENT_DESC {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            D3D11_APPEND_ALIGNED_ELEMENT,
            D3D11_INPUT_PER_VERTEX_DATA,
            0
        },
    };

    const D3D11_BUFFER_DESC cbVSDescs[] = {
        {
            sizeof(MeshRenderer::CBVS),	// UINT ByteWidth;
            D3D11_USAGE_DYNAMIC,		// D3D11_USAGE Usage;
            D3D11_BIND_CONSTANT_BUFFER, // UINT BindFlags;
            D3D11_CPU_ACCESS_WRITE,		// UINT CPUAccessFlags;
            0,							// UINT MiscFlags;
            0,							// UINT StructureByteStride;
        }
    };

    shader = std::make_shared<Shader>(
        L"shaders/dbg_navmesh.hlsl",
        (Shader::CreationFlags)(Shader::VERTEX_SH | Shader::PIXEL_SH),
        m_device,
        inputElements,
        (int)std::size(inputElements),
        cbVSDescs, 1, nullptr, 0);

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));

    D3D11_RENDER_TARGET_BLEND_DESC rtbd;
    ZeroMemory(&rtbd, sizeof(rtbd));
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    m_device->CreateBlendState(&blendDesc, &blendState);
}

MyDebugDraw::~MyDebugDraw()
{
}

void MyDebugDraw::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    {
        MyVertex vert;
        vert.position.x = inFrom.GetX();
        vert.position.y = inFrom.GetY();
        vert.position.z = inFrom.GetZ();
        const JPH::Vec4 col4 = inColor.ToVec4();
        memcpy(&vert.color, &col4, sizeof(col4));
        m_lineVertices.push_back(vert);
    }
    {
        MyVertex vert;
        vert.position.x = inTo.GetX();
        vert.position.y = inTo.GetY();
        vert.position.z = inTo.GetZ();
        const JPH::Vec4 col4 = inColor.ToVec4();
        memcpy(&vert.color, &col4, sizeof(col4));
        m_lineVertices.push_back(vert);
    }
}

void MyDebugDraw::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow)
{
    {
        MyVertex vert;
        vert.position.x = inV1.GetX();
        vert.position.y = inV1.GetY();
        vert.position.z = inV1.GetZ();
        const JPH::Vec4 col4 = inColor.ToVec4();
        memcpy(&vert.color, &col4, sizeof(col4));
        m_triVertices.push_back(vert);
    }
    {
        MyVertex vert;
        vert.position.x = inV2.GetX();
        vert.position.y = inV2.GetY();
        vert.position.z = inV2.GetZ();
        const JPH::Vec4 col4 = inColor.ToVec4();
        memcpy(&vert.color, &col4, sizeof(col4));
        m_triVertices.push_back(vert);
    }
    {
        MyVertex vert;
        vert.position.x = inV3.GetX();
        vert.position.y = inV3.GetY();
        vert.position.z = inV3.GetZ();
        const JPH::Vec4 col4 = inColor.ToVec4();
        memcpy(&vert.color, &col4, sizeof(col4));
        m_triVertices.push_back(vert);
    }
}

void MyDebugDraw::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight)
{
}

void MyDebugDraw::Draw()
{
    DrawImpl(m_lineVertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    DrawImpl(m_triVertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_lineVertices.clear();
    m_triVertices.clear();
}

void MyDebugDraw::DrawImpl(const std::vector<MyVertex>& vertices, D3D11_PRIMITIVE_TOPOLOGY primitive)
{
    if (vertices.empty())
        return;

    MeshRenderer::CBVS cbvs;
    cbvs.viewProj = gRenderSys->cameraManager.GetViewProjTransposed();
    shader->Activate(gRenderSys->GetContext(), shader);
    shader->SetCBVS(m_context, 0, &cbvs);

    m_context->IASetPrimitiveTopology(primitive);

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(MyVertex) * vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr))
        return;

    UINT stride = sizeof(MyVertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

    m_context->Draw(static_cast<UINT>(vertices.size()), 0);

    vertexBuffer->Release();
}
