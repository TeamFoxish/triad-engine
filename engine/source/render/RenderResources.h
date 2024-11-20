#pragma once

#include "resource/Res.h"

#include <unordered_map>
#include <memory>
#include <cassert>

struct RenderResources {
	friend bool InitRenderResources();
	friend void TermRenderResources();

	RenderResources() = default;
	RenderResources(const RenderResources&) = delete;
	RenderResources(RenderResources&&) = delete;
	~RenderResources() = default;

	static RenderResources& Instance() { return *instance; }

	template<typename K, typename T>
	class Storage {
	public:
		T Get(K key);

		void Add(K key, T res);
		void Remove(K key);

	private:
		std::unordered_map<K, T> storage;
	};

	Storage<ResTag, std::shared_ptr<class Material>> materials;
	Storage<ResTag, std::shared_ptr<struct Shader>> shaders;
	Storage<ResTag, class Texture*> textures;

private:
	static inline std::unique_ptr<RenderResources> instance = nullptr;
};

template<typename K, typename T>
inline T RenderResources::Storage<K, T>::Get(K key)
{
	auto iter = storage.find(key);
	assert(iter != storage.end());
	return iter->second;
}

template<typename K, typename T>
inline void RenderResources::Storage<K, T>::Add(K key, T res)
{
	assert(storage.find(key) == storage.end());
	storage[key] = std::move(res);
}

template<typename K, typename T>
inline void RenderResources::Storage<K, T>::Remove(K key)
{
	auto iter = storage.find(key);
	assert(iter != storage.end());
	storage.erase(iter);
}

bool InitRenderResources();
void TermRenderResources();
