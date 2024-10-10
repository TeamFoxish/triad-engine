#include "QuadComponent.h"

#include "core/Game.h"
#include "core/Compositer.h"
#include "render/GeometryData.h"
#include "render/Shader.h"
#include "render/Renderer.h"
#include "render/RenderUtils.h"
#include "os/Window.h"

QuadComponent::QuadComponent(Game* game, Compositer* compositer)
	: DrawComponent(game, compositer)
{
	/*Renderer* renderer = GetGame()->GetRenderer();
	GeometryData::PTR geom = renderer->GetUtils()->GetQuadGeom(renderer);
	SetGeometry(geom);
	Shader::PTR shader = renderer->GetUtils()->GetQuadShader(renderer);
	SetShader(shader);*/
}

void QuadComponent::Draw(Renderer* renderer)
{
	using namespace Math;

	/*auto shader = GetShader();
	auto context = renderer->GetDeviceContext();
	auto window = renderer->GetWindow();
	const Vector2 pos = parentRef ? Vector2{parentRef->GetPosition().x, parentRef->GetPosition().y} : Vector2{ 0.0f, 0.0f };
	const Vector2 screenDim{(float)window->GetWidth(), (float)window->GetHeigth()};
	const Vector2 screenPos = (pos / screenDim - Vector2{0.5f}) * Vector2{2.0f};
	const Vector2 screenSize = size / screenDim * Vector2{2.0f};
	auto cbVS = RenderUtils::QuadCBVS{};
	cbVS.offset = DirectX::SimpleMath::Vector4{screenPos.x, screenPos.y, 0, 0};
	cbVS.size = DirectX::SimpleMath::Vector4{screenSize.x, screenSize.y, 0, 0};
	auto cbPS = RenderUtils::QuadCBPS{};
	cbPS.color = color;
	shader.lock()->SetCBVS(context, 0, &cbVS);
	shader.lock()->SetCBPS(context, 0, &cbPS);
	DrawComponent::Draw(renderer);*/
}
