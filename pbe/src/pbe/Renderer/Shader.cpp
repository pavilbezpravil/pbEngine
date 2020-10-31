#include "pch.h"
#include "Shader.h"

#include <d3dcompiler.h>
#include <d3d12shader.h>


#include "pbe/Core/Utility.h"
#include "pbe/Renderer/Renderer.h"

using Microsoft::WRL::ComPtr;

namespace pbe {

	// void ShaderReflection() {
	// 	ComPtr<ID3D12ShaderReflection> reflection;
	// 	HRESULT hr = D3DReflect(shader.code->GetBufferPointer(), shader.code->GetBufferSize(), IID_ID3D12ShaderReflection, reinterpret_cast<void**>(reflection.GetAddressOf()));
	// 	ASSERT_SUCCEEDED(hr);
	//
	// 	D3D12_SHADER_DESC shaderDesc;
	// 	reflection->GetDesc(&shaderDesc);
	// 	
	// }


#define ThrowIfFailed(x) x

	namespace
	{
		ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename) {
			std::ifstream fin(filename, std::ios::binary);

			fin.seekg(0, std::ios_base::end);
			std::ifstream::pos_type size = (int)fin.tellg();
			fin.seekg(0, std::ios_base::beg);

			ComPtr<ID3DBlob> blob;
			ThrowIfFailed(D3DCreateBlob(size, blob.GetAddressOf()));

			fin.read((char*)blob->GetBufferPointer(), size);
			fin.close();

			return blob;
		}

		ComPtr<ID3DBlob> CompileBlob(
			const std::wstring& filename,
			const D3D_SHADER_MACRO* defines,
			const std::string& entrypoint,
			const std::string& target) {
			UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
			compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			HRESULT hr = S_OK;

			ComPtr<ID3DBlob> byteCode = nullptr;
			ComPtr<ID3DBlob> errors;
			hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

			if (errors != nullptr)
				HZ_CORE_WARN((char*)errors->GetBufferPointer());
				// OutputDebugStringA((char*)errors->GetBufferPointer());

			ThrowIfFailed(hr);

			return byteCode;
		}

		std::string GetTarget(ShaderType type) {
			std::string res;

			switch (type) {
			case ShaderType::Vertex: res = "vs_5_0"; break;
			case ShaderType::Pixel: res = "ps_5_0"; break;
			default: assert(false);
			}
			return res;
		}

		std::vector<D3D_SHADER_MACRO> D3DMacroToVector(const D3D_SHADER_MACRO* defines) {
			std::vector<D3D_SHADER_MACRO> ret;
			while (defines && defines->Name) {
				ret.emplace_back(*defines);
				++defines;
			}
			return ret;
		}
	}

	std::unordered_map<std::wstring, Ref<Shader>> Shader::_map;

	Ref<Shader> Shader::Get(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,
		ShaderType type) {
		Ref<Shader> shader;

		std::wstring shaderIdentifier = filename + MakeWStr(entrypoint);
		if (!_map.count(shaderIdentifier)) {
			shader = Shader::Compile(filename, defines, entrypoint, type);
			_map[shaderIdentifier] = shader;
		}
		return _map[shaderIdentifier];
	}

	void Shader::Recompile(bool force) {
		ASSERT(force);
		HZ_CORE_INFO("Recompile shader(force = {})", force);

		for (auto& item : _map) {
			auto& shader = item.second;
			shader->CompileBlobInternal();
		}
	}

	void Shader::Init() {
	}

	void Shader::Deinit() {
		_map.clear();
	}

	D3D12_SHADER_BYTECODE Shader::GetByteCode() const {
		return byteCode;
	}

	Ref<Shader> Shader::Compile(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint,
		ShaderType type) {
		Ref<Shader> shader = Ref<Shader>(new Shader(type));
		shader->filename = filename;
		shader->defines = D3DMacroToVector(defines);
		shader->entrypoint = entrypoint;
		shader->target = GetTarget(type);
		shader->type = type;

		shader->CompileBlobInternal();

		return shader;
	}

	void Shader::CompileBlobInternal() {
		// todo: tmp
		std::wstring fname = L"assets/shaders/";
		fname += filename;
		fname += L".hlsl";

		HZ_CORE_INFO("Compile shader {}", MakeStr(fname));
		
		blob = CompileBlob(fname, defines.data(), entrypoint, target);
		if (blob) {
			byteCode = { blob->GetBufferPointer(), blob->GetBufferSize() };
		}
	}

}
