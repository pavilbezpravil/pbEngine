#pragma once

#include "pbe/Core/Base.h"
#include "pbe/Core/Buffer.h"

#include "pbe/Renderer/RendererAPI.h"
#include "pbe/Renderer/ShaderUniform.h"

#include <string>
#include <glm/glm.hpp>

namespace pbe
{

	enum class ShaderType {
		Vertex,
		Pixel,
		Unknown,
	};

	class Shader : public RefCounted {
	public:
		explicit Shader(ShaderType type = ShaderType::Unknown)
			: type(type) {
		}

		static Ref<Shader> Get(const std::wstring& filename,
			const D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			ShaderType type);

		static void Recompile(bool force);

		static void Init();
		static void Deinit();

		operator bool() const { return blob; }

		D3D12_SHADER_BYTECODE GetByteCode();

	private:
		static Ref<Shader> Compile(const std::wstring& filename,
			const D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			ShaderType type);

		void CompileBlobInternal();

		static std::unordered_map<std::wstring, Ref<Shader>> _map;

		D3D12_SHADER_BYTECODE byteCode;
		Microsoft::WRL::ComPtr<ID3DBlob> blob;
		ShaderType type;
		std::wstring filename;
		std::vector<D3D_SHADER_MACRO> defines;
		std::string entrypoint;
		std::string target;
	};

}