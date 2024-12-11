#include "Component.h"

#include "game/Component.h"

#include "Game.h"
#include "scene/Scene.h"

Component::Component(Game* _game, Compositer* parent)
	: game(_game)
{
	SetId();
	name = std::format("Component {}", num++);

	if (parent) {
		parent->AddChild({this});
		hasParent = true;
		game->scenes[0]->GetStorage().TEMP_AddChildComponentIdEntry(this);
		_parent = parent;
		return;
	}
	// TODO: make Game a compositer as well??? (may require additional member pointer)
	game->AddComponent(this);
}

Component::~Component()
{
	if (!hasParent) {
		game->RemoveComponent(this);
	}
}

void Component::SetId()
{
	// 0 may be utilized as invalid id
	static Id_T _id = 1;
	uid = _id++;
}
