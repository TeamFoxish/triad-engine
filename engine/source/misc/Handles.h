#pragma once

#undef min
#undef max
#include <thh-handle-vector/handle-vector.hpp>

template<typename T>
class HandleStorage {
public:
	using Storage = thh::handle_vector_t<T>;
	using Handle = thh::handle_t;
	using Index = int32_t;

	Index Length() const { return storage.size(); }

	// actual index in elements storage
	Handle GetHandleFromIndex(Index idx) const { return storage.handle_from_index(idx); }

	// the id part of handle
	Handle GetHandleFromId(Index id) const 
	{
		std::optional<Index> idx = storage.index_from_id(id);
		return idx ? GetHandleFromIndex(*idx) : Handle{};
	} 

	Handle Add(const T& item) { return storage.add(item); }
	Handle Add(T&& item) { return storage.add(std::move(item)); }
	void Remove(Handle handle) { storage.remove(handle); }

	void Clear() { storage.clear(); }

	T* Get(Handle handle) 
	{ 
		// not working because of copying
		// return storage.call_return(handle, [](const T& item) { return item; });
		T* res = nullptr;
		storage.call(handle, [&res](T& item) { res = &item; });
		return res;
	}

	const T* Get(Handle handle) const
	{
		const T* res = nullptr;
		storage.call(handle, [&res](const T& item) { res = &item; });
		return res;
	}

	T& operator[](Handle handle) 
	{
		T* item = Get(handle);
		assert(item);
		return *item;
	}

	const T& operator[](Handle handle) const
	{
		const T* item = Get(handle);
		assert(item);
		return *item;
	}

	T& operator[](Index idx) { return storage[idx]; }
	const T& operator[](Index idx) const { return storage[idx]; }

	Storage::iterator begin() { return storage.begin(); }
	Storage::const_iterator begin() const { return storage.begin(); }
	Storage::iterator end() { return storage.end(); }
	Storage::const_iterator end() const { return storage.end(); }

private:
	Storage storage;
};
