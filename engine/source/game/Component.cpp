#include "Component.h"

#include "game/Component.h"

#include "Game.h"

Component::Component(Game* _game, Compositer* parent)
	: game(_game)
{
	name = std::format("Component {}", num++);

	if (parent) {
		parent->AddChild({this});
		hasParent = true;
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
