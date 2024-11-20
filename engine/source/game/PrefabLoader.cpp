#include "PrefabLoader.h"

#include "math/Math.h"

#include "render/RenderSystem.h"
#include "render/RenderResources.h"
#include "render/Renderer.h"
#include "render/material/Material.h"

#include "game/Game.h"
#include "components/CompositeComponent.h"
#include "components/ThirdPersonCamera.h"
#include "components/MeshComponent.h"
#include "components/PlayerBall.h"
#include "components/DirectionalLightComponent.h"
#include "components/PointLightComponent.h"
#include "components/OrbiterComponent.h"

#include "runtime/EngineRuntime.h"

PrefabLoader::PrefabLoader()
{
	FACTORY_INIT;
}

PrefabLoader::~PrefabLoader()
{
}

void PrefabLoader::Load(ResTag tag, const YAML::Node& desc)
{
	auto rndPos = [](float extendX, float extendY) {
		auto rndFloat = []() { return (float)rand() / RAND_MAX; };
		const float x = extendX * 2;
		const float y = extendY * 2;
		return Math::Vector3(rndFloat() * x - extendX, rndFloat() * y - extendY, 0.0f);
	};
	auto rndColor = []() {
		auto rndFloat = []() { return (float)rand() / RAND_MAX; };
		return Math::Color(rndFloat(), rndFloat(), rndFloat());
	};

	const std::string& prefabType = desc["class"].Scalar();
	const std::string& materialTag = desc["material"].IsDefined() ? desc["material"].Scalar() : "";
	std::shared_ptr<Material> material;
	if (!materialTag.empty()) {
		material = RenderResources::Instance().materials.Get(ToStrid(materialTag));
	}
	if (prefabType == "dir_light") {
		creators[tag] = [](const YAML::Node& desc) {
			CompositeComponent* dirLightSocket = new CompositeComponent(gTempGame.get());
			DirectionalLightComponent* dirLight = new DirectionalLightComponent(gTempGame.get(), dirLightSocket);
			dirLight->GetLightSource().SetColor(Math::Color(0.75, 0.75f, 0.75f));
			dirLightSocket->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(Math::Pi / 4.0f, -Math::Pi / 4.0f, 0.0f));
			dirLightSocket->Initialize();
			return dirLightSocket;
		};
	} else if (prefabType == "point_light") {
		creators[tag] = [material](const YAML::Node& desc) {
			Math::Color col = {1.0f, 1.0f, 1.0f};
			if (desc["r"]) {
				col.x = desc["r"].as<float>();
			}
			if (desc["g"]) {
				col.y = desc["g"].as<float>();
			}
			if (desc["b"]) {
				col.z = desc["b"].as<float>();
			}
			CompositeComponent* pointLightC = new CompositeComponent(gTempGame.get());
			MeshComponent* mesh = new MeshComponent(gTempGame.get(), pointLightC);
			if (material) {
				mesh->SetMaterial(material);
			}
			TEMP_OverrideMaterial(mesh, desc);
			mesh->SetGeometry(gRenderSys->GetRenderer()->GetUtils()->GetSphereGeom(gRenderSys->GetRenderer()));
			mesh->SetColor(col);
			PointLightComponent* pointLight = new PointLightComponent(gTempGame.get(), pointLightC);
			pointLight->GetLightSource().SetColor(col);
			pointLight->GetLightSource().SetIntensity(25.0f);
			OrbiterComponent* orbiter = new OrbiterComponent(gTempGame.get(), pointLightC, Math::Vector3{ 0.0f, 0.0f, 2.5f }, 10.0f, -1.0f);
			orbiter->SetMoveSpeed(1.0f);
			pointLightC->SetScale(Math::Vector3{ 0.2f });
			pointLightC->Initialize();
			return pointLightC;
		};
	} else if (prefabType == "player") {
		creators[tag] = [material](const YAML::Node& desc) {
			auto player = new PlayerBall(gTempGame.get());
			player->Initialize();
			if (material) {
				player->meshComp->SetMaterial(material);
			}
			TEMP_OverrideMaterial(player->meshComp, desc);
			return player;
		};
	} else if (prefabType == "flopa") {
		creators[tag] = [rndPos, material](const YAML::Node& desc) {
			CompositeComponent* flopa = new CompositeComponent(gTempGame.get());
			const Mesh::PTR& mesh = gRenderSys->GetRenderer()->GetMesh("assets/flop.fbx");
			MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
			if (rootMesh) {
				if (material) {
					rootMesh->SetMaterial(material);
				}
				TEMP_OverrideMaterial(rootMesh, desc);
				// TODO: i feel really bad about this
				flopa->boundingSphereRadius = rootMesh->boundingSphereRadius;
			}
			flopa->SetPosition(rndPos(20, 20));
			flopa->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(0.0f, Math::Pi / 2, 0.0f));
			flopa->SetScale(Math::Vector3{ 0.005f });
			flopa->Initialize();
			gTempGame->player->sceneObjects.push_back(flopa);
			return flopa;
		};
	} else if (prefabType == "cheese") {
		creators[tag] = [rndPos, material](const YAML::Node& desc) {
			CompositeComponent* flopa = new CompositeComponent(gTempGame.get());
			const Mesh::PTR& mesh = gRenderSys->GetRenderer()->GetMesh("assets/cheese.fbx");
			MeshComponent* rootMesh = MeshComponent::Build(mesh, flopa);
			if (rootMesh) {
				if (material) {
					rootMesh->SetMaterial(material);
				}
				TEMP_OverrideMaterial(rootMesh, desc);
				// TODO: i feel really bad about this
				flopa->boundingSphereRadius = rootMesh->boundingSphereRadius;
			}
			flopa->SetPosition(rndPos(20, 20));
			//flopa->SetRotation(Math::Quaternion::CreateFromYawPitchRoll(Math::Pi / 2, 0.0f, 0.0f));
			//flopa->SetScale(Math::Vector3{ 1.0f });
			flopa->Initialize();
			gTempGame->player->sceneObjects.push_back(flopa);
			return flopa;
		};
	}
}

Component* PrefabLoader::Create(ResTag tag, const YAML::Node& desc)
{
	return creators[tag](desc);
}

void PrefabLoader::TEMP_OverrideMaterial(MeshComponent* mesh, const YAML::Node& overrides)
{
	const YAML::Node& material = overrides["material"];
	if (!material.IsDefined()) {
		return;
	}
	const auto mat = RenderResources::Instance().materials.Get(ToStrid(material.Scalar()));
	mesh->SetMaterial(mat);
}
