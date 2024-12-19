#pragma once

#include "Lights.h"

#include "misc/Handles.h"
#include "shared/TransformStorage.h"

struct LightsStorage {
	friend bool InitLightsStorage();
	friend void TermLightsStorage();

	template<typename LightT>
    class StorageImpl {
	public:
		struct LightSource;
		using Storage = HandleStorage<LightSource>;
		using Handle = Storage::Handle;

		struct LightSource {
			std::unique_ptr<LightT> light; // TODO: no need to hold ptr to light since each type is held by different storages. replace with value
			TransformStorage::Handle transform;
		};

		StorageImpl() 
		{
			instance = this;
		}
		StorageImpl(const StorageImpl&) = delete;
		StorageImpl(StorageImpl&&) = delete;
		~StorageImpl() 
		{
			instance = nullptr;
		}

		LightT& Get(Handle handle) { return *storage[handle].light; }
		LightSource& GetLightSource(Handle handle) { return storage[handle];  }

		const Storage& GetStorage() const { return storage; }

		Handle Add(LightSource&& light) { return storage.Add(std::move(light)); }
		void Remove(Handle handle) { storage.Remove(handle); }

		static StorageImpl& Instance() { return *instance; }

	private:
		Storage storage;

		static inline StorageImpl* instance;
    };

	static LightsStorage& Instance() { return *instance; }

	StorageImpl<DirectionalLight> dirLights;
	StorageImpl<PointLight> pointLights;

private:
	static inline std::unique_ptr<LightsStorage> instance;
};

bool InitLightsStorage();
void TermLightsStorage();
