#include "ColoredTriangleDemo.h"
#include "Game.h"
#include "GameException.h"
#include "Utility.h"
#include "VertexDeclarations.h"
#include "ColorHelper.h"
#include <iostream>
#include <vector>
#include <WICTextureLoader.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/algorithm/string.hpp>

namespace Rendering
{
	RTTI_DEFINITIONS(ColoredTriangleDemo)

	ColoredTriangleDemo::ColoredTriangleDemo(Game& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr)
	{
	}

	ColoredTriangleDemo::~ColoredTriangleDemo()
	{
		ReleaseObject(mVertexBuffer)
		ReleaseObject(mPixelShader)
		ReleaseObject(mInputLayout)
		ReleaseObject(mVertexShader)
		//ReleaseObject(mIndexBuffer)
		ReleaseObject(mConstantBuffer)
	}

	//Load mesh from file using obj wavefront object format
	//First read the file, then parse it, then build vertex and index buffers from it
	void ColoredTriangleDemo::load_mesh_from_file(std::string file_name, std::vector<VertexPositionTextureNormal>& geometry_info){

		//Define struct to hold per face info
		typedef struct {
			int v1, t1, n1,
				v2, t2, n2,
				v3, t3, n3;
		} t_face_info;

		std::string read_line;	//To store read line
		std::vector<std::string> file_lines;	
		std::vector<XMFLOAT3> vertex_coord;
		std::vector<XMFLOAT3> vertex_normal;
		std::vector<XMFLOAT2> tex_coord;
		std::vector<t_face_info> face_info;

		std::ifstream file_to_read(file_name);

		if (!file_to_read){
			std::cout << "Cannot open mesh file " << file_name << std::endl;
			exit;
		}

		//Read file line by line and push to vector
		while (file_to_read){
			std::getline(file_to_read, read_line);
			file_lines.push_back(read_line);
		}
		file_to_read.close();

		//Parse
		std::string word, param1, param2, param3, temp;
		for (std::vector<std::string>::iterator it = file_lines.begin(); it != file_lines.end(); ++it){
			std::istringstream sstream;
			sstream.str(*it);
			sstream >> word;

			if (!word.compare("v")){
				//Vertex coord
				sstream >> param1 >> param2 >> param3;
				vertex_coord.push_back(XMFLOAT3(std::stof(param1),
					std::stof(param2),
					std::stof(param3)));
				continue;
			}
			else if (!word.compare("vt")){
				//Vertex texture coordinate
				sstream >> param1 >> param2;
				tex_coord.push_back(XMFLOAT2(std::stof(param1), std::stof(param2)));
				continue;
			}
			else if (!word.compare("vn")){
				//Vertex normal coordinate
				sstream >> param1 >> param2 >> param3;
				vertex_normal.push_back(XMFLOAT3(std::stof(param1),
					std::stof(param2),
					std::stof(param3)));
				continue;
			}
			else if (!word.compare("f")){
				//Face info stored in format "v/t/n v/t/n v/t/n"
				sstream >> param1 >> param2 >> param3;
				//Break the params
				std::vector<std::string> face_info_string1, face_info_string2,face_info_string3;
				t_face_info f;
				boost::split(face_info_string1, param1, boost::is_any_of("/"));
				boost::split(face_info_string2, param2, boost::is_any_of("/"));
				boost::split(face_info_string3, param3, boost::is_any_of("/"));
				f = { std::stoi(face_info_string1[0]),
					std::stoi(face_info_string1[1]),
					std::stoi(face_info_string1[2]),
					std::stoi(face_info_string2[0]),
					std::stoi(face_info_string2[1]),
					std::stoi(face_info_string2[2]),
					std::stoi(face_info_string3[0]),
					std::stoi(face_info_string3[1]),
					std::stoi(face_info_string3[2]) };
				face_info.push_back(f);
			}
			else {
				//Useless info
				continue;
			}
		}//For loop

		//File parsed and stored. Build vtn pairs now from indices
		VertexPositionTextureNormal vptn;
		XMFLOAT4 position;
		XMFLOAT3 normal;
		XMFLOAT2 texture;
		for (std::vector<t_face_info>::iterator it = face_info.begin(); it != face_info.end(); ++it){
			
			//Vertex1
			position = XMFLOAT4(vertex_coord[it->v1 - 1].x, vertex_coord[it->v1 - 1].y, vertex_coord[it->v1 - 1].z, 1.f);
			normal = XMFLOAT3(vertex_normal[it->n1 - 1].x, vertex_normal[it->n1 - 1].y, vertex_normal[it->n1 - 1].z);
			texture = XMFLOAT2(tex_coord[it->t1 - 1].x, tex_coord[it->t1 - 1].y);
			vptn = { position, texture, normal };
			geometry_info.push_back(vptn);

			//Vertex2
			position = XMFLOAT4(vertex_coord[it->v2 - 1].x, vertex_coord[it->v2 - 1].y, vertex_coord[it->v2 - 1].z, 1.f);
			normal = XMFLOAT3(vertex_normal[it->n2 - 1].x, vertex_normal[it->n2 - 1].y, vertex_normal[it->n2 - 1].z);
			texture = XMFLOAT2(tex_coord[it->t2 - 1].x, tex_coord[it->t2 - 1].y);
			vptn = { position, texture, normal };
			geometry_info.push_back(vptn);

			//Vertex3
			position = XMFLOAT4(vertex_coord[it->v3 - 1].x, vertex_coord[it->v3 - 1].y, vertex_coord[it->v3 - 1].z, 1.f);
			normal = XMFLOAT3(vertex_normal[it->n3 - 1].x, vertex_normal[it->n3 - 1].y, vertex_normal[it->n3 - 1].z);
			texture = XMFLOAT2(tex_coord[it->t3 - 1].x, tex_coord[it->t3 - 1].y);
			vptn = { position, texture, normal };
			geometry_info.push_back(vptn);

		}//For - parsing complete
		
	} //End: load_mesh_from_file()

	//Load vertices with texture
	void ColoredTriangleDemo::load_vertices(VertexPositionTexture* v_vertices){
		
		//1. Populate the vector with vertices
		v_vertices[0] = (VertexPositionTexture(XMFLOAT4(-0.5f, -0.5f, 0.f, 1.f), XMFLOAT2(0.25f,0.75f)));
		v_vertices[1] = (VertexPositionTexture(XMFLOAT4(0.5f, -0.5f, 0.f, 1.f), XMFLOAT2(0.75f, 0.75f)));
		v_vertices[2] = (VertexPositionTexture(XMFLOAT4(0.5f, 0.5f, 0.f, 1.f), XMFLOAT2(0.75f, 0.25f)));
		v_vertices[3] = (VertexPositionTexture(XMFLOAT4(-0.5f, 0.5f, 0.f, 1.f), XMFLOAT2(0.25f, 0.25f)));
		v_vertices[4] = (VertexPositionTexture(XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.f), XMFLOAT2(0.25f, 1.f)));
		v_vertices[5] = (VertexPositionTexture(XMFLOAT4(0.5f, -0.5f, 0.5f, 1.f), XMFLOAT2(0.75f, 1.f)));
		v_vertices[6] = (VertexPositionTexture(XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.f), XMFLOAT2(0.25f, 0.f)));
		v_vertices[7] = (VertexPositionTexture(XMFLOAT4(0.5f, 0.5f, 0.5f, 1.f), XMFLOAT2(0.75f, 0.f)));
	}

	//Load vertices without texture
	void ColoredTriangleDemo::load_vertices(VertexPositionColor* v_vertices){
	}

	//Load indices
	void ColoredTriangleDemo::load_indices(unsigned int* indices){
		indices[0] = 0;
		indices[1] = 3;
		indices[2] = 2;
		indices[3] = 0;
		indices[4] = 2;
		indices[5] = 1;
		indices[6] = 1;
		indices[7] = 2;
		indices[8] = 7;
		indices[9] = 1;
		indices[10] = 7;
		indices[11] = 5;
		indices[12] = 2;
		indices[13] = 3;
		indices[14] = 7;
		indices[15] = 3;
		indices[16] = 6;
		indices[17] = 7;
		indices[18] = 0;
		indices[19] = 4;
		indices[20] = 5;
		indices[21] = 0;
		indices[22] = 5;
		indices[23] = 1;
		indices[24] = 0;
		indices[25] = 3;
		indices[26] = 6;
		indices[27] = 0;
		indices[28] = 6;
		indices[29] = 4;
		indices[30] = 4;
		indices[31] = 6;
		indices[32] = 7;
		indices[33] = 4;
		indices[34] = 7;
		indices[35] = 5;

	}
	
	void ColoredTriangleDemo::Initialize()
	{
		SetCurrentDirectory(Utility::ExecutableDirectory().c_str());

		// Load a compiled vertex shader
		std::vector<char> compiledVertexShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\VertexShader.cso", compiledVertexShader);		
		ThrowIfFailed(mGame->Direct3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, &mVertexShader), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::vector<char> compiledPixelShader;
		Utility::LoadBinaryFile(L"Content\\Effects\\PixelShader.cso", compiledPixelShader);
		ThrowIfFailed(mGame->Direct3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, &mPixelShader), "ID3D11Device::CreatedPixelShader() failed.");

		//Load mesh from file
		std::vector<VertexPositionTextureNormal> geometry_info;
		load_mesh_from_file("E:\\DX\\Essential_lessons\\source\\Lesson3.1\\cup._obj", geometry_info);

		// Create an input layout for Postion Texture Normal
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		ThrowIfFailed(mGame->Direct3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), &mInputLayout), "ID3D11Device::CreateInputLayout() failed.");

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTextureNormal) * geometry_info.size();
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &geometry_info[0];
		ThrowIfFailed(mGame->Direct3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, &mVertexBuffer), "ID3D11Device::CreateBuffer() failed.");

		//Indexed drawcall no supported if mesh loaded from file in obj format
		/*
		//Create an index buffer
		//1. Populate index buffer data
		unsigned int indices[36];
		load_indices(indices);
		
		//2. Define Desc
		D3D11_BUFFER_DESC indexBufferDesc;
		indexBufferDesc.ByteWidth = sizeof(indices);
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.MiscFlags = 0;
		indexBufferDesc.CPUAccessFlags = 0;

		//Create subresource data
		D3D11_SUBRESOURCE_DATA indexSubResourceData;
		ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
		indexSubResourceData.pSysMem = indices;

		//3.Create Buffer
		HRESULT ib;
		ib = mGame->Direct3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, &mIndexBuffer);
		if (FAILED(ib)){
			std::cout << "Index Buffer Creation failed. " << ib << std::endl;
			return;
		}
		*/

		//Load texture
		HRESULT srv = CreateWICTextureFromFile(mGame->Direct3DDevice(), NULL, L"E:\\DX\\Essential_lessons\\source\\Lesson3.1\\cup.jpg", NULL, &mTexture2dSRV);
		if (FAILED(srv)){
			std::cout << "Texture Loading failed from file " << srv << std::endl;
			return;
		}

		//Create SSO
		D3D11_SAMPLER_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		textureDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		textureDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		textureDesc.MinLOD = 0;
		textureDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT sso = mGame->Direct3DDevice()->CreateSamplerState(&textureDesc, &textureSamplerStateObject);
		if (FAILED(sso)){
			std::cout << "Sampler State Creation Failed " << sso << std::endl;
			return;
		}	
		//Create World Matrix
		XMMATRIX world = XMMatrixIdentity();
		//Create View matrices
		XMVECTOR eyepos = XMVectorSet(-2.f, 2.f, -3.f, 0.f);
		XMVECTOR lookat = XMVectorSet(0.f, 0.f, 0.f,0.f);
		XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f,0.f);
		XMMATRIX view = XMMatrixLookAtLH(eyepos, lookat, up);

		//Create Projection matrix
		float fovDegrees = 90.f;
		float fovRadians = (float)(fovDegrees / 360.f) * 3.14f;
		float aspectRatio = (float)(1024.f / 768.f);
		float nearz = 0.1f;
		float farz = 1000.f;
		XMMATRIX projection = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearz, farz);

		//Create View Projection Matrix
		XMMATRIX viewProjections =world * view * projection;
		viewProjections = XMMatrixTranspose(viewProjections);

		//Create Constant buffer desc
		D3D11_BUFFER_DESC constantDesc;
		ZeroMemory(&constantDesc, sizeof(constantDesc));
		constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantDesc.ByteWidth = sizeof(viewProjections);
		constantDesc.Usage = D3D11_USAGE_DYNAMIC;
		constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		//Create subresource data for CB
		D3D11_SUBRESOURCE_DATA cbData;
		ZeroMemory(&cbData, sizeof(cbData));
		cbData.pSysMem = &viewProjections;

		HRESULT cb = mGame->Direct3DDevice()->CreateBuffer(&constantDesc, &cbData, &mConstantBuffer);
		if (FAILED(cb)){
			std::cout << "Constant Buffer Creation Failed " << cb << std::endl;
			return;
		}
	}

	void ColoredTriangleDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->Direct3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout);

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
		//Indexed drawcall not supported if mesh loaded from file in obj format
		//direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		direct3DDeviceContext->PSSetShaderResources(0, 1, &mTexture2dSRV);
		direct3DDeviceContext->PSSetSamplers(0, 1, &textureSamplerStateObject);
		direct3DDeviceContext->VSSetShader(mVertexShader, nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader, nullptr, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

		direct3DDeviceContext->Draw(5643, 0);
		//direct3DDeviceContext->DrawIndexed(36,0,0);
	}
}