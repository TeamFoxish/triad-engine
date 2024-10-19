#pragma once

#ifdef EDITOR

#include <memory>

class EditorRuntime;

class EditorConfigurator {
public:
	bool CreateRuntime(int argc, char* argv[], std::unique_ptr<EditorRuntime>& runtime);
};

#endif // EDITOR
