#pragma once

#include "math/Math.h"
#include "misc/Strid.h"

#include <memory>
#include <variant>
#include <vector>

//struct Shader;
class Texture;

class Material {
	friend class MaterialLoader;

public:
	Material() = default;
	~Material();

	void Use(struct RenderContext& ctx);

	bool HasTextures() const { return !textures.empty(); }

protected:
	struct Batch {
		static constexpr int ALIGN = 4;
		uint32_t data[ALIGN];
	};
	std::vector<Batch> BuildBuffer() const;

protected:
	struct Prop {
		struct Visitor
		{
			std::vector<Batch>& dest;
			int& size;

			template<typename T>
			void operator()(const T& val) const {
				constexpr int VAL_SIZE = sizeof(T);
				static_assert(VAL_SIZE % 4 == 0, "not sure if this code will work for smaller types");
				const int sizeLeft = (int)(dest.size() * sizeof(Batch) - size);
				if (sizeLeft >= VAL_SIZE) {
					assert(sizeLeft < sizeof(Batch));
					void* target = reinterpret_cast<char*>(dest.data()) + size;
					memcpy(target, &val, VAL_SIZE);
					size += VAL_SIZE;
					return;
				}
				const int alignedSize = (int)(dest.size() * sizeof(Batch));
				size = alignedSize + VAL_SIZE;
				const int sizeNeeded = (int)std::ceil(VAL_SIZE / (float)sizeof(Batch));
				dest.resize(dest.size() + sizeNeeded);
				{
					void* target = reinterpret_cast<char*>(dest.data()) + alignedSize;
					memcpy(target, &val, VAL_SIZE);
				}
			}
		};

		enum Type {
			Invalid = 0,
			Integer,
			Float,
			Vector4,
		};
		static Type ParseType(Strid typeStr);

		void AppendTo(std::vector<Batch>& dest, int& size) const;

		Strid key;
		Type type = Invalid;
		using ValT = std::variant<int, float, Math::Vector4>;
		ValT val;
	};

	struct TextureProp {
		Strid key;
		uint32_t slot;
		Texture* tex;
	};

protected:
	// unable to add shader reference
	// requires too many creation parameters
	// shaders are defined in specific renderers
	// std::shared_ptr<Shader> shader;

	struct ID3D11Buffer* buffer = nullptr;
	uint32_t bufferSize = 0;
	
	using PropList = std::vector<Prop>;
	PropList properties;

	using TexturePropList = std::vector<TextureProp>;
	TexturePropList textures;

	static constexpr uint32_t MATERIAL_BIND_SLOT = 1;
};
