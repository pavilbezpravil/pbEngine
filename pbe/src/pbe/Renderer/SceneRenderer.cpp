#include "pch.h"
#include "SceneRenderer.h"

#include "Renderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "ColorBuffer.h"
#include "Renderer2D.h"

namespace pbe {

	struct SceneRendererData
	{
		const Scene* ActiveScene = nullptr;
		struct SceneInfo
		{
			SceneRendererCamera SceneCamera;

			// Resources
			// Environment SceneEnvironment;
			Light ActiveLight;
		} SceneData;

		Ref<Shader> CompositeShader;

		struct DrawCommand
		{
			Ref<Mesh> Mesh;
			Ref<MaterialInstance> Material;
			glm::mat4 Transform;
		};
		std::vector<DrawCommand> DrawList;
		std::vector<DrawCommand> SelectedMeshDrawList;

		// Grid
		Ref<MaterialInstance> GridMaterial;
		Ref<MaterialInstance> OutlineMaterial;

		SceneRendererOptions Options;
	};

	static SceneRendererData s_Data;

	void SceneRenderer::Init()
	{
		// s_Data.CompositeShader = Shader::Create("assets/shaders/SceneComposite.glsl");

		// Grid
		// auto gridShader = Shader::Create("assets/shaders/Grid.glsl");
		// s_Data.GridMaterial = MaterialInstance::Create(Material::Create(gridShader));
		// float gridScale = 16.025f, gridSize = 0.025f;
		// s_Data.GridMaterial->Set("u_Scale", gridScale);
		// s_Data.GridMaterial->Set("u_Res", gridSize);
		//
		// // Outline
		// s_Data.OutlineMaterial->SetFlag(MaterialFlag::DepthTest, false);
	}

	void SceneRenderer::BeginScene(const Scene* scene, const SceneRendererCamera& camera)
	{
		HZ_CORE_ASSERT(!s_Data.ActiveScene, "");

		s_Data.ActiveScene = scene;

		s_Data.SceneData.SceneCamera = camera;
		s_Data.SceneData.ActiveLight = scene->m_Light;
	}

	void SceneRenderer::EndScene()
	{
		HZ_CORE_ASSERT(s_Data.ActiveScene, "");

		s_Data.ActiveScene = nullptr;

		FlushDrawList();
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform, Ref<MaterialInstance> overrideMaterial)
	{
		// TODO: Culling, sorting, etc.
		s_Data.DrawList.push_back({ mesh, overrideMaterial, transform });
	}

	void SceneRenderer::SubmitSelectedMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		s_Data.SelectedMeshDrawList.push_back({ mesh, nullptr, transform });
	}

	static Ref<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader;

	std::pair<Ref<TextureCube>, Ref<TextureCube>> SceneRenderer::CreateEnvironmentMap(const std::string& filepath)
	{
		const uint32_t cubemapSize = 2048;
		const uint32_t irradianceMapSize = 32;

		Ref<TextureCube> envUnfiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);
		// if (!equirectangularConversionShader)
			// equirectangularConversionShader = Shader::Create("assets/shaders/EquirectangularToCubeMap.glsl");
		Ref<Texture2D> envEquirect = Texture2D::Create(filepath);
		HZ_CORE_ASSERT(envEquirect->GetFormat() == TextureFormat::Float16, "Texture is not HDR!");

		// equirectangularConversionShader->Bind();
		envEquirect->Bind();
		Renderer::Submit([envUnfiltered, cubemapSize, envEquirect]()
		{
			// glBindImageTexture(0, envUnfiltered->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
			// glDispatchCompute(cubemapSize / 32, cubemapSize / 32, 6);
			// glGenerateTextureMipmap(envUnfiltered->GetRendererID());		
		});


		// if (!envFilteringShader)
			// envFilteringShader = Shader::Create("assets/shaders/EnvironmentMipFilter.glsl");

		Ref<TextureCube> envFiltered = TextureCube::Create(TextureFormat::Float16, cubemapSize, cubemapSize);

		Renderer::Submit([envUnfiltered, envFiltered]()
		{
			// glCopyImageSubData(envUnfiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			// 	envFiltered->GetRendererID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
			// 	envFiltered->GetWidth(), envFiltered->GetHeight(), 6);
		});

		// envFilteringShader->Bind();
		envUnfiltered->Bind();

		Renderer::Submit([envUnfiltered, envFiltered, cubemapSize]() {
			const float deltaRoughness = 1.0f / glm::max((float)(envFiltered->GetMipLevelCount() - 1.0f), 1.0f);
			for (int level = 1, size = cubemapSize / 2; level < envFiltered->GetMipLevelCount(); level++, size /= 2) // <= ?
			{
				// const GLuint numGroups = glm::max(1, size / 32);
				// glBindImageTexture(0, envFiltered->GetRendererID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				// glProgramUniform1f(envFilteringShader->GetRendererID(), 0, level * deltaRoughness);
				// glDispatchCompute(numGroups, numGroups, 6);
			}
		});

		// if (!envIrradianceShader)
			// envIrradianceShader = Shader::Create("assets/shaders/EnvironmentIrradiance.glsl");

		Ref<TextureCube> irradianceMap = TextureCube::Create(TextureFormat::Float16, irradianceMapSize, irradianceMapSize);
		// envIrradianceShader->Bind();
		envFiltered->Bind();
		Renderer::Submit([irradianceMap]()
		{
				// glBindImageTexture(0, irradianceMap->GetRendererID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				// glDispatchCompute(irradianceMap->GetWidth() / 32, irradianceMap->GetHeight() / 32, 6);
				// glGenerateTextureMipmap(irradianceMap->GetRendererID());
		});

		return { envFiltered, irradianceMap };
	}

	void SceneRenderer::GeometryPass()
	{

	}

	void SceneRenderer::CompositePass()
	{
		// dx12
	}

	void SceneRenderer::FlushDrawList()
	{
		HZ_CORE_ASSERT(!s_Data.ActiveScene, "");

		GeometryPass();
		CompositePass();

		s_Data.DrawList.clear();
		s_Data.SelectedMeshDrawList.clear();
		s_Data.SceneData = {};
	}

	Ref<Texture2D> SceneRenderer::GetFinalColorBuffer()
	{
		// return s_Data.CompositePass->GetSpecification().TargetFramebuffer;
		HZ_CORE_ASSERT(false, "Not implemented");
		return nullptr;
	}

	uint32_t SceneRenderer::GetFinalColorBufferRendererID()
	{
		return -1;
	}

	SceneRendererOptions& SceneRenderer::GetOptions()
	{
		return s_Data.Options;
	}

}