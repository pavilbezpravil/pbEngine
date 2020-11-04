#include "pch.h"
#include "SceneRenderer.h"

#include "GraphicsCommon.h"
#include "CommandContext.h"
#include "SamplerManager.h"
#include "ColorBuffer.h"
#include "Renderer.h"

namespace pbe {

	namespace
	{
		enum class BaseDescriptor : uint
		{
			cbPass = 0,
			cbModel = 1,
			texDepth = 2,
			gLights = 3,
		};
		
		struct cbPass {
			Mat4 gVP;
			Mat4 gWorldToShadowMap;
			Vec3 gCamPos;
			int gNumLights;
		};

		struct cbModel {
			Mat4 gTransform;
			Mat4 gNormalTransform;
		};
	}

	void SceneRenderer::InitBaseRootSignature()
	{
		BaseRootSignature = Ref<RootSignature>::Create();
		(*BaseRootSignature).Reset(4, 1);
		(*BaseRootSignature).InitStaticSampler(0, Graphics::SamplerPointBorderDesc);
		(*BaseRootSignature)[(int)BaseDescriptor::cbPass].InitAsConstantBuffer(0);
		(*BaseRootSignature)[(int)BaseDescriptor::cbModel].InitAsConstantBuffer(1);
		(*BaseRootSignature)[(int)BaseDescriptor::texDepth].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1);
		(*BaseRootSignature)[(int)BaseDescriptor::gLights].InitAsBufferSRV(1);
		(*BaseRootSignature).Finalize(L"Base", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	void SceneRenderer::Init()
	{
		_shadowBuffer = Ref<DepthBuffer>::Create(1.f);
		_shadowBuffer->Create(L"Shadow buffer", 512, 512, DXGI_FORMAT_D16_UNORM);

		{
			std::vector<D3D_SHADER_MACRO> defines;
			defines.push_back({ "COLOR_PASS", "" });
			defines.push_back({ 0, 0 });

			vs = Shader::Get(L"base", defines.data(), "mainVS", ShaderType::Vertex);
			ps = Shader::Get(L"base", defines.data(), "mainPS", ShaderType::Pixel);
		}

		{
			std::vector<D3D_SHADER_MACRO> defines;
			defines.push_back({ "SHADOW_PASS", "" });
			defines.push_back({ 0, 0 });
			
			vs_shadow = Shader::Get(L"base", defines.data(), "mainVS", ShaderType::Vertex);
			ps_shadow = Shader::Get(L"base", defines.data(), "mainPS", ShaderType::Pixel);
		}

		InitBaseRootSignature();
	}

	void SceneRenderer::Shutdown()
	{
		
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo,
	                               const Environment& environment)
	{
		HZ_CORE_ASSERT(!_scene);
		_scene = scene;
		_cameraInfo = cameraInfo;
		_environment = environment;

		if (environment.lights.size() > _lightsGPUBuffer.GetElementCount()) {
			_lightsGPUBuffer.Create(L"Scene Lights", environment.lights.size(), sizeof(Light));
		}
	}

	void SceneRenderer::EndScene()
	{
		HZ_CORE_ASSERT(_scene);
		FlushDrawList();
		_scene = nullptr;
	}

	void SceneRenderer::SubmitMesh(Ref<Mesh> mesh, const glm::mat4& transform)
	{
		DrawCommand cmd = {mesh, transform};
		_drawList.push_back(cmd);
	}

	Mat4 SceneRenderer::GetShadowViewProj()
	{
		int directLightIdx = -1;

		for (int i = 0; i < _environment.lights.size(); ++i) {
			if (_environment.lights[i].type == Light::Direction) {
				directLightIdx = i;
				break;
			}
		}
		
		if (directLightIdx == -1) {
			return Mat4(1.f);
		}

		Light directLight = _environment.lights[directLightIdx];
		
		const float shadowHalfBounds = 20;
		const float shadowDepth = 40;
		auto shadowProj = glm::orthoRH_ZO(-shadowHalfBounds, shadowHalfBounds,
			-shadowHalfBounds, shadowHalfBounds,
			0.f, shadowDepth);

		Vec3 center = _cameraInfo.position - directLight.positionOrDirection * shadowDepth / 2.f;
		auto shadowView = glm::lookAtRH(center, center + directLight.positionOrDirection, directLight.up);

		return shadowProj * shadowView;
	}

	void SceneRenderer::DrawAllMesh(GraphicsContext& context)
	{
		// todo:
		context.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& drawCmd : _drawList) {
			auto& mesh = drawCmd.mesh;
			for (auto& submesh : mesh->GetSubmeshes()) {
				cbModel model;
				model.gTransform = drawCmd.transform * submesh.Transform;
				model.gNormalTransform = glm::transpose(glm::inverse(model.gTransform));
				context.SetDynamicConstantBufferView((uint)BaseDescriptor::cbModel, sizeof(model), &model);

				auto& inputLayout = mesh->GetVertexBuffer()->GetInputLayout();
				context.SetInputLayout((UINT)inputLayout.size(), inputLayout.data());

				context.SetVertexBuffer(0, mesh->GetVertexBuffer()->VertexBufferView(submesh.BaseVertex));
				context.SetIndexBuffer(mesh->GetIndexBuffer()->IndexBufferView(submesh.BaseIndex));
				context.DrawIndexed(submesh.IndexCount, 0, 0);
			}
		}
	}

	void SceneRenderer::ShadowPass()
	{
		GraphicsContext& context = GraphicsContext::Begin(L"Shadow Pass");

		context.ClearDepth(_shadowBuffer);

		context.SetDepthStencilTarget(_shadowBuffer);
		context.SetViewportAndScissor(0, 0, _shadowBuffer->GetWidth(), _shadowBuffer->GetHeight());

		context.SetRootSignature(BaseRootSignature);
		context.SetDepthStencilState(Graphics::DepthStateReadWrite);

		context.SetVertexShader(vs_shadow);
		context.SetPixelShader(ps_shadow);

		cbPass pass;
		pass.gVP = GetShadowViewProj();
		context.SetDynamicConstantBufferView((uint)BaseDescriptor::cbPass, sizeof(cbPass), &pass);

		DrawAllMesh(context);

		context.Finish();
	}

	void SceneRenderer::DepthPass()
	{
	}

	void SceneRenderer::ColorPass()
	{
		GraphicsContext& context = GraphicsContext::Begin(L"Color Pass");

		auto& rt = Renderer::Get().GetFullScreenColor();
		auto& depth = Renderer::Get().GetFullScreenDepth();

		context.ClearColor(rt);
		context.ClearDepth(depth);

		context.SetRenderTarget(rt, depth);
		context.SetViewportAndScissor(0, 0, rt->GetWidth(), rt->GetHeight());

		context.SetRootSignature(BaseRootSignature);
		context.SetDepthStencilState(Graphics::DepthStateReadWrite);

		context.SetVertexShader(vs);
		context.SetPixelShader(ps);

		cbPass pass;
		pass.gVP = _cameraInfo.viewProj;
		pass.gWorldToShadowMap = glm::translate(Mat4(1.f), {0.5f, 0.5f, 0.f})
								* glm::scale(Mat4(1.f), { 0.5f, -0.5f, 1.f })
								* GetShadowViewProj();
		pass.gCamPos = _cameraInfo.position;
		pass.gNumLights = _environment.lights.size();

		context.SetDynamicConstantBufferView((uint)BaseDescriptor::cbPass, sizeof(cbPass), &pass);

		context.SetDynamicDescriptor((uint)BaseDescriptor::texDepth, 0, _shadowBuffer->GetDepthSRV());

		context.WriteBuffer(_lightsGPUBuffer, 0, _environment.lights.data(), _environment.lights.size() * sizeof(Light));
		context.TransitionResource(_lightsGPUBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context.SetBufferSRV((uint)BaseDescriptor::gLights, _lightsGPUBuffer);
		// context.SetDynamicSRV((uint)BaseDescriptor::gLights, _environment.lights.size() * sizeof(Light), _environment.lights.data());

		DrawAllMesh(context);

		context.Finish();
	}

	void SceneRenderer::FlushDrawList()
	{
		ShadowPass();
		DepthPass();
		ColorPass();

		_drawList = {};
	}
}
