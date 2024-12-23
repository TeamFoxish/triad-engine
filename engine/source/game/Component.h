#pragma once

#include "Compositer.h"

#include <string>

class Game;
class Component;
//template<typename, typename>
//class Compositer_T;
using Compositer = Compositer_T<Component>;

static int num = 0;

class Component
{
public:
	Component(Game* game, Compositer* parent = nullptr);
	virtual ~Component();

	virtual void Initialize(Compositer* parent = nullptr) {  }
	virtual void ProceedInput(class InputDevice* inpDevice) {  }
	virtual void Update(float deltaTime, Compositer* parent = nullptr) {  }

	Game* GetGame() const { return game; }
	std::string GetName() const { return name; }
	bool HasParent() const { return hasParent; }

	using Id_T = int32_t;
	Id_T GetId() const { return uid; }

private:
	void SetId();

#ifdef EDITOR
public:
	bool isComposite = false;
#endif // EDITOR
	Compositer* _parent = nullptr; // ToDo: remove


private:
	Game* game;
	bool hasParent = false;
	std::string name;
	Id_T uid;
};
