#include "MeshRenderer.h"

#include "shared/SharedStorage.h"
#include "render/GeometryData.h"
#include "render/RenderContext.h"
#include "render/RenderSystem.h"
#include "render/Renderer.h"
#include "render/Shader.h"

// TEMP --------------------------------
#include "components/ThirdPersonCamera.h"
#include "runtime/EngineRuntime.h"
#include "game/Game.h"
#include "render/RenderResources.h"
// TEMP --------------------------------

void MeshRenderer::DrawGeometryOnly(RenderContext& ctx, const Renderable& obj)
{
	if (!obj.mesh) {
		return;
	}
	auto renderer = gRenderSys->GetRenderer();
	auto& shader = ctx.activeShader;
	if (!shader) {
		// TODO: add error log
		return;
	}
	auto cbVS = MeshRenderer::CBVS{};
	cbVS.viewProj = gRenderSys->cameraManager.GetViewProjTransposed(); // already transposed
	auto cbPS = MeshRenderer::CBPSGeom{};
	auto material = obj.material ? obj.material : RenderResources::Instance().materials.Get(ToStrid("res://materials/default_mesh.material")); // TEMP
	cbPS.ambientColor = Math::Color{ 0.2f, 0.2f, 0.2f };
	cbPS.isTextureSet = obj.material->HasBindedTextures() ? 1 : 0;
	cbPS.entityId = obj.entityId;
	shader->SetCBPS(ctx.context, 0, &cbPS);
	obj.material->Use(ctx);
	for (const Mesh::MeshNode& node : obj.mesh->GetNodes()) {
		// TODO: multiply component world matrix by node local transform (if not identity)
		cbVS.worldTransform = SharedStorage::Instance().transforms.AccessRead(obj.transform).GetMatrix().Transpose();
		shader->SetCBVS(ctx.context, 0, &cbVS);
		for (auto& geom : node.geoms) {
			geom->Activate(ctx.context);
			ctx->DrawIndexed(geom->idxNum, 0, 0);
		}
	}
}
