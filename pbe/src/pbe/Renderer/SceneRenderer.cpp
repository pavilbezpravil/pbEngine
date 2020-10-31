#include "pch.h"
#include "SceneRenderer.h"

#include "GraphicsCommon.h"
#include "CommandContext.h"
#include "ColorBuffer.h"
#include "Renderer.h"

namespace pbe {

	void SceneRenderer::Init()
	{
		FVF fvf = FVF_POS | FVF_NORMAL;

		vs = Shader::Get(L"base", NULL, "mainVS", ShaderType::Vertex);
		ps = Shader::Get(L"base", NULL, "mainPS", ShaderType::Pixel);

		auto inputLayout = fvfGetInputLayout(fvf);

		InitBaseRootSignature();

		pso = Graphics::GraphicsPSODefault;

		pso.SetRootSignature(BaseRootSignature);
		pso.SetInputLayout((UINT)inputLayout.size(), inputLayout.data());
		pso.SetVertexShader(vs->GetByteCode());
		pso.SetPixelShader(ps->GetByteCode());
		pso.SetDepthStencilState(Graphics::DepthStateReadWrite);
		pso.SetRenderTargetFormat(Renderer::Get().GetFullScreenColor()->GetFormat(), Renderer::Get().GetFullScreenDepth()->GetFormat());
		pso.Finalize();
	}

	void SceneRenderer::BeginScene(const Ref<Scene>& scene, const CameraInfo& cameraInfo)
	{
		HZ_CORE_ASSERT(!_scene);
		_scene = scene;
		_cameraInfo = cameraInfo;
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

	void SceneRenderer::FlushDrawList()
	{
		GraphicsContext& context = GraphicsContext::Begin(L"Scene Renderer");

		ColorBuffer& rt = *Renderer::Get().GetFullScreenColor();
		DepthBuffer& depth = *Renderer::Get().GetFullScreenDepth();

		context.TransitionResource(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		context.ClearColor(rt);

		context.TransitionResource(depth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context.ClearDepthAndStencil(depth);
		
		context.SetRenderTarget(rt.GetRTV(), depth.GetDSV());
		context.SetViewportAndScissor(0, 0, rt.GetWidth(), rt.GetHeight());

		context.SetPipelineState(pso);
		context.SetRootSignature(BaseRootSignature);
		context.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		struct cbPass {
			Mat4 gMVP;
		};

		cbPass pass;
		pass.gMVP = _cameraInfo.viewProj;
		context.SetDynamicConstantBufferView(0, sizeof(cbPass), &pass);

		for (auto& drawCmd : _drawList) {
			// todo: transform
			auto& mesh = drawCmd.mesh;
			for (auto& submesh : mesh->GetSubmeshes()) {
				struct cbModel {
					Mat4 gTransform;
				};

				cbModel model;
				model.gTransform = submesh.Transform;
				context.SetDynamicConstantBufferView(1, sizeof(model), &model);

				context.SetVertexBuffer(0, mesh->GetVertexBuffer()->VertexBufferView(submesh.BaseVertex));
				context.SetIndexBuffer(mesh->GetIndexBuffer()->IndexBufferView(submesh.BaseIndex));
				context.DrawIndexed(submesh.IndexCount, 0, 0);
			}
		}

		context.Finish();

		_drawList = {};
	}

}
