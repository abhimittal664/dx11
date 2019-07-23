#pragma once

#include "DrawableGameComponent.h"
#include "VertexDeclarations.h"
#include <iostream>
#include <vector>
#include <objLoader/OBJ_Loader.h>
#include <Mouse.h>
#include <Keyboard.h>

using namespace Library;

namespace Rendering
{
	class ColoredTriangleDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ColoredTriangleDemo, DrawableGameComponent)

	public:
		ColoredTriangleDemo(Game& game, Camera& camera);
		~ColoredTriangleDemo();

		//New types
		typedef struct{
			XMFLOAT4 position;
			XMFLOAT2 texcoord;
			XMFLOAT3 normal;
		} VertexPositionTextureNormalMaterialId;

		typedef struct {
			XMFLOAT4 kAmbient;
			XMFLOAT4 kDiffuse;
			XMFLOAT4 kSpecular;
			XMFLOAT4 ExpSpecular;
			int uses_Ka;
			int uses_Kd;
			int uses_Ks;
			int dummy;	//Padded to make constant buffer size x16
		} t_objectMaterial;

		typedef struct {
			ID3D11ShaderResourceView* mTexture2dSRV[3];
		} t_texture2dSRV;

		ColoredTriangleDemo() = delete;
		ColoredTriangleDemo(const ColoredTriangleDemo& rhs) = delete;
		ColoredTriangleDemo& operator=(const ColoredTriangleDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

		objl::Loader						load_mesh_from_file(std::string);
		std::vector<ID3D11Buffer*>			load_vertex_buffer_from_file(objl::Loader);
		std::vector<ID3D11Buffer*>			load_index_buffer_from_file(objl::Loader);
		ID3D11Buffer*						create_light_cbuffer();
		ID3D11Buffer*						create_object_mat_cbuffer(std::string);
		ID3D11Buffer*						create_mvp_cbuffer();
		std::vector<t_objectMaterial>		load_materials(objl::Loader);
		std::vector<t_texture2dSRV>			load_textures_from_file(objl::Loader);
		std::vector<ID3D11Buffer*>			load_material_cbuffer(std::vector<ColoredTriangleDemo::t_objectMaterial>);

		void								update_camera_loc();
		ID3D11SamplerState*					create_shader_sampler();

	private:
		ID3D11VertexShader*					mVertexShader;		
		ID3D11PixelShader*					mPixelShader;
		ID3D11InputLayout*					mInputLayout;
		ID3D11RasterizerState*				rasState;

		std::vector<ID3D11Buffer*>			v_vertexBuffer;
		std::vector<ID3D11Buffer*>			v_indexBuffer;
		ID3D11Buffer*						mConstantBuffer;	//To pass MVP transform matrices
		ID3D11Buffer*						mLightCBuffer;	//To pass lighting info
		std::vector<ID3D11Buffer*>			v_objMaterialCBuffer; //To pass object material info

		ID3D11SamplerState*					textureSamplerStateObject;
		std::vector<t_texture2dSRV>			v_texture2dSRV;
		std::vector<t_objectMaterial>		v_materials;

		//Obj model loader object
		objl::Loader						mobjMeshLoader;

		//Mouse
		Mouse*								pMouse;
		XMFLOAT3							mouse_loc;

		//Keyboard
		Keyboard*							pKeyboard;
	};

} // Namespace
