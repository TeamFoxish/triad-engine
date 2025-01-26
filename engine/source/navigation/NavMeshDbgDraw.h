#ifdef EDITOR

#include "DebugDraw.h"

#include "render/RenderContext.h"

class NavMeshDbgDraw : public duDebugDraw {
public:
    NavMeshDbgDraw();
    ~NavMeshDbgDraw();

    virtual void depthMask(bool state) override;
    virtual void texture(bool state) override;
    virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f) override;
    virtual void vertex(const float* pos, unsigned int color) override;
    virtual void vertex(const float x, const float y, const float z, unsigned int color) override;
    virtual void vertex(const float* pos, unsigned int color, const float* uv) override;
    virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) override;
    virtual void end() override;

    void draw();

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_context;
    ID3D11BlendState* blendState;
    std::shared_ptr<Shader> shader;

    struct Vertex {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 color;
    };

    std::vector<Vertex> m_vertices;
    duDebugDrawPrimitives m_currentPrim;
    float m_pointSize;
    bool m_depthEnabled;
};

#endif // EDITOR
