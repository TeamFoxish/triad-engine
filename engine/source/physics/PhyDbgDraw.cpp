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

void MyDebugDraw::depthMask(bool state)
{
    m_depthEnabled = true;

    // Enable/disable depth testing in the DirectX pipeline
    ID3D11DepthStencilState* depthStencilState = nullptr;

    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = m_depthEnabled;
    depthDesc.DepthWriteMask = m_depthEnabled ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS;

    m_device->CreateDepthStencilState(&depthDesc, &depthStencilState);
    m_context->OMSetDepthStencilState(depthStencilState, 0);
    if (depthStencilState)
        depthStencilState->Release();
}

void MyDebugDraw::texture(bool state)
{
    // ignore
}

void MyDebugDraw::begin(duDebugDrawPrimitives prim, float size)
{
    m_currentPrim = prim;
    m_pointSize = size;
    m_vertices.clear();
}

void MyDebugDraw::vertex(const float* pos, unsigned int color)
{
    vertex(pos[0], pos[1], pos[2], color);
}

void MyDebugDraw::vertex(const float x, const float y, const float z, unsigned int color)
{
    DirectX::XMFLOAT4 colorFloat(
        ((color >> 0) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF) / 255.0f,
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 24) & 0xFF) / 255.0f
    );
    m_vertices.push_back({ {x, y, z}, colorFloat });
}

void MyDebugDraw::vertex(const float* pos, unsigned int color, const float* uv)
{
    vertex(pos[0], pos[1], pos[2], color);
}

void MyDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
    vertex(x, y, z, color);
}

void MyDebugDraw::end()
{
    Draw();
}

void MyDebugDraw::DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor)
{
    duDebugDrawArrow(this,
        inFrom.GetX(), inFrom.GetY(), inFrom.GetZ(),
        inTo.GetX(), inTo.GetY(), inTo.GetZ(),
        0.f, 0.f,
        inColor.GetUInt32(),
        m_lineWidth
        );
}

void MyDebugDraw::DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow)
{
    DrawLine(inV1, inV2, inColor);
    DrawLine(inV2, inV3, inColor);
    DrawLine(inV3, inV1, inColor);
}

void MyDebugDraw::DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight)
{
}

void MyDebugDraw::Draw()
{
    if (m_vertices.empty())
        return;

    MeshRenderer::CBVS cbvs;
    cbvs.viewProj = gRenderSys->cameraManager.GetViewProjTransposed();
    shader->Activate(gRenderSys->GetContext(), shader);
    shader->SetCBVS(m_context, 0, &cbvs);

    D3D11_PRIMITIVE_TOPOLOGY topology;
    switch (m_currentPrim) {
    case DU_DRAW_POINTS:
        topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        break;
    case DU_DRAW_LINES:
        topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
        break;
    case DU_DRAW_TRIS:
        topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        break;
    default:
        return;
    }
    m_context->IASetPrimitiveTopology(topology);

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = static_cast<UINT>(sizeof(MyVertex) * m_vertices.size());
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = m_vertices.data();

    ID3D11Buffer* vertexBuffer = nullptr;
    HRESULT hr = m_device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
    if (FAILED(hr))
        return;

    UINT stride = sizeof(MyVertex);
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_context->OMSetBlendState(blendState, blendFactor, 0xffffffff);

    m_context->Draw(static_cast<UINT>(m_vertices.size()), 0);

    vertexBuffer->Release();
}
