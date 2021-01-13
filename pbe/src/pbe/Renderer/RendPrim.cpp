#include "pch.h"
#include "RendPrim.h"

#include "GraphicsCommon.h"
#include "RootSignature.h"
#include "Shader.h"
#include "pbe/Geom/GeomBuffer.h"
#include "pbe/Geom/GeomUtils.h"
#include "pbe/Core/Math/AABB.h"


namespace pbe {

	enum class BaseDescriptor : uint
	{
		cbPass = 0,
	};

	struct cbPass {
		Mat4 gVP;
	};

	struct RendPrim_Impl
	{
		RendPrim_Impl()
		{
			lineBuffer.Create(FVF_POS | FVF_COLOR);
			Init();
		}
		
		void InitRendPrimRootSignature()
		{
			RendPrimRootSignature = Ref<RootSignature>::Create();
			(*RendPrimRootSignature).Reset(1);
			(*RendPrimRootSignature)[(int)BaseDescriptor::cbPass].InitAsConstantBuffer(0);
			(*RendPrimRootSignature).Finalize(L"RendPrim", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		}

		void Init()
		{
			{
				vs = Shader::Get(L"rend_prim", NULL, "mainVS", ShaderType::Vertex);
				ps = Shader::Get(L"rend_prim", NULL, "mainPS", ShaderType::Pixel);
			}

			InitRendPrimRootSignature();
		}

		~RendPrim_Impl()
		{
			
		}

		void DrawLine(const Vec3& from, const Vec3& to, const Color& color)
		{
			uint nVert = lineBuffer.NumVertex();
			uint nIdxs = lineBuffer.NumIndexes();
			lineBuffer.AddVertexes(2);
			lineBuffer.AddIndexes(2);

			lineBuffer.PosMut(nVert) = from;
			lineBuffer.ColorMut(nVert) = color;
			lineBuffer.PosMut(nVert + 1) = to;
			lineBuffer.ColorMut(nVert + 1) = color;

			lineBuffer.IndexMut(nIdxs) = nVert;
			lineBuffer.IndexMut(nIdxs + 1) = nVert + 1;
		}

		void DrawAABB(const AABB& aabb, const Color& color)
		{
			Vec3 size = aabb.Size();

			Vec3 p0 = aabb.Min;
			Vec3 p1 = p0;
			p1.x += size.x;
			Vec3 p2 = p1;
			p2.y += size.y;
			Vec3 p3 = p2;
			p3.x -= size.x;

			DrawLine(p0, p1, color);
			DrawLine(p1, p2, color);
			DrawLine(p2, p3, color);
			DrawLine(p3, p0, color);

			Vec3 p4 = p0;
			Vec3 p5 = p1;
			Vec3 p6 = p2;
			Vec3 p7 = p3;

			p4.z += size.z;
			p5.z += size.z;
			p6.z += size.z;
			p7.z += size.z;

			DrawLine(p0, p4, color);
			DrawLine(p1, p5, color);
			DrawLine(p2, p6, color);
			DrawLine(p3, p7, color);

			DrawLine(p4, p5, color);
			DrawLine(p5, p6, color);
			DrawLine(p6, p7, color);
			DrawLine(p7, p4, color);
		}

		void DrawCircle(const Vec3& center, const Vec3& normal, float radius, uint nSegments, const Color& color)
		{
			nSegments = std::max(nSegments, 3u);

			Vec3 basisX;
			basisX = glm::cross(normal, Vec3_X);
			if (glm::length(basisX) < 0.1) {
				basisX = glm::cross(normal, Vec3_Y);
			}
			basisX = glm::normalize(basisX);

			float angleDelta = M_2PI / nSegments;
			Quat q = glm::angleAxis(angleDelta, normal);

			Vec3 pPrevL = basisX * radius;
			for (int i = 0; i < nSegments; ++i) {
				Vec3 pNextL = q * pPrevL;
				DrawLine(center + pPrevL, center + pNextL, color);
				pPrevL = pNextL;
			}
		}

		void DrawSphere(const Vec3& center, float radius, uint nSegments, const Color& color)
		{
			DrawCircle(center, Vec3_X, radius, nSegments, color);
			DrawCircle(center, Vec3_Y, radius, nSegments, color);
			DrawCircle(center, Vec3_Z, radius, nSegments, color);
		}

		void DrawCone(const Vec3& center, const Vec3& forward, float angle, float radius, uint nSegments, const Color& color)
		{
			nSegments = std::max(nSegments, 3u);

			Vec3 basisX;
			basisX = glm::cross(forward, Vec3_X);
			if (glm::length(basisX) < 0.1) {
				basisX = glm::cross(forward, Vec3_Y);
			}
			basisX = glm::normalize(basisX);

			float angleDelta = M_2PI / nSegments;
			Quat q = glm::angleAxis(angleDelta, forward);

			Quat q1 = glm::angleAxis(angle / 2.f, basisX);
			Vec3 pPrevL = q1 * forward * radius;
			for (int i = 0; i < nSegments; ++i) {
				Vec3 pNextL = q * pPrevL;
				DrawLine(center, center + pPrevL, color);
				DrawLine(center + pPrevL, center + pNextL, color);
				pPrevL = pNextL;
			}
		}

		void RenderPrimitives(GraphicsContext& context, const Mat4& viewProj)
		{
			if (lineBuffer.NumIndexes() == 0) {
				return;
			}

			uint nVertex = lineBuffer.NumVertex();
			if (lineVB2.GetElementCount() < nVertex) {
				lineVB2.Create(L"RendPrim_lineBufferVB", nVertex, lineBuffer.GetStride());
			}

			uint nIndexes = lineBuffer.NumIndexes();
			if (lineIB2.GetElementCount() < nIndexes) {
				lineIB2.Create(L"RendPrim_lineBufferIB", nIndexes, SIZEOF_INDEX);
			}
			context.WriteBuffer(lineVB2, 0, lineBuffer.GetRawVertexData(), nVertex * lineBuffer.GetStride());
			context.WriteBuffer(lineIB2, 0, lineBuffer.GetRawIndexesData(), nIndexes * SIZEOF_INDEX);
			context.TransitionResource(lineVB2, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			context.TransitionResource(lineIB2, D3D12_RESOURCE_STATE_INDEX_BUFFER, true);

			cbPass pass;
			pass.gVP = viewProj;
			context.SetDynamicConstantBufferView((uint)BaseDescriptor::cbPass, sizeof(cbPass), &pass);

			context.SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);

			auto inputLayout = fvfGetInputLayout(lineBuffer.GetFVF());
			context.SetInputLayout((UINT)inputLayout.size(), inputLayout.data());

			context.SetVertexShader(vs);
			context.SetPixelShader(ps);

			context.SetVertexBuffer(0, lineVB2.VertexBufferView());
			context.SetIndexBuffer(lineIB2.IndexBufferView());
			context.DrawIndexed(lineBuffer.NumIndexes(), 0, 0);
		}

		void NextFrame()
		{
			lineBuffer.Clear();
		}

		Ref<RootSignature> RendPrimRootSignature = nullptr;

		Ref<Shader> vs;
		Ref<Shader> ps;

		GeomBuffer lineBuffer;

		ByteAddressBuffer lineVB2;
		ByteAddressBuffer lineIB2;
	};
	static RendPrim_Impl* primRnd = NULL;


	namespace RendPrim
	{

		void Init() {
			primRnd = new RendPrim_Impl();
		}

		void Term() {
			delete primRnd;
			primRnd = NULL;
		}

		void RenderPrimitives(GraphicsContext& context, const Mat4& viewProj)
		{
			primRnd->RenderPrimitives(context, viewProj);
			primRnd->NextFrame();
		}

		void DrawLine(const Vec3& from, const Vec3& to, const Color& color)
		{
			primRnd->DrawLine(from, to, color);
		}

		void DrawAABB(const AABB& aabb, const Color& color)
		{
			primRnd->DrawAABB(aabb, color);
		}

		void DrawCircle(const Vec3& center, const Vec3& normal, float radius, uint nSegments, const Color& color)
		{
			primRnd->DrawCircle(center, normal, radius, nSegments, color);
		}

		void DrawSphere(const Vec3& center, float radius, uint nSegments, const Color& color)
		{
			primRnd->DrawSphere(center, radius, nSegments, color);
		}

		void DrawCone(const Vec3& center, const Vec3& forward, float angle, float radius, uint nSegments, const Color& color)
		{
			primRnd->DrawCone(center, forward, angle, radius, nSegments, color);
		}
	}

}
