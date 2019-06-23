#pragma once

#include "DrawableGameComponent.h"
#include "VertexDeclarations.h"
#include <string>

using namespace Library;

namespace Rendering
{
	class ColoredTriangleDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ColoredTriangleDemo, DrawableGameComponent)

	public:
		ColoredTriangleDemo(Game& game, Camera& camera);
		~ColoredTriangleDemo();

		ColoredTriangleDemo() = delete;
		ColoredTriangleDemo(const ColoredTriangleDemo& rhs) = delete;
		ColoredTriangleDemo& operator=(const ColoredTriangleDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

		void load_vertices(VertexPositionColor*);
		void load_vertices(VertexPositionTexture*);
		//void load_vertices(VertexPositionTextureNormal*);
		void load_indices(unsigned int*);
		void load_texture2d_data(XMFLOAT4*, int, int);
		void load_mesh_from_file(std::string, std::vector<VertexPositionTextureNormal>&);

	private:
		ID3D11VertexShader* mVertexShader;		
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;
		ID3D11Buffer* mVertexBuffer;
		ID3D11Buffer* mIndexBuffer;
		ID3D11Buffer* mConstantBuffer;
		ID3D11Texture2D* mTextureBuffer2d;
		ID3D11SamplerState* textureSamplerStateObject;
		ID3D11ShaderResourceView* mTexture2dSRV;
	};
}
