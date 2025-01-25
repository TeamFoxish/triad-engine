#pragma once

#include "DebugDraw.h"

#include "Jolt/Jolt.h"
#include "Jolt/Renderer/DebugRendererSimple.h"

#include "render/RenderContext.h"

JPH_SUPPRESS_WARNINGS

class MyDebugDraw : public JPH::DebugRendererSimple
{
public:
    MyDebugDraw();
    virtual ~MyDebugDraw();

public:
    virtual void DrawLine(JPH::RVec3Arg inFrom, JPH::RVec3Arg inTo, JPH::ColorArg inColor) override;

    virtual void DrawTriangle(JPH::RVec3Arg inV1, JPH::RVec3Arg inV2, JPH::RVec3Arg inV3, JPH::ColorArg inColor, ECastShadow inCastShadow) override;

    virtual void DrawText3D(JPH::RVec3Arg inPosition, const JPH::string_view& inString, JPH::ColorArg inColor, float inHeight) override;

public:
    void Draw();

private:
    struct MyVertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };
    void DrawImpl(const std::vector<MyVertex>& vertices, D3D11_PRIMITIVE_TOPOLOGY primitive);

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11BlendState* blendState;
    std::shared_ptr<Shader> shader;

    std::vector<MyVertex> m_lineVertices;
    std::vector<MyVertex> m_triVertices;

    float m_pointSize;
    float m_lineWidth;
    bool m_depthEnabled;
};