#pragma once
#include <memory>
#include <vector>
#include <d3d12.h>
#include <string>
#include <wrl/client.h>
#include "Singleton.h"
#include <dxcapi.h>

#pragma comment(lib, "dxcompiler.lib")

using namespace std;
using namespace Microsoft::WRL;

// �V�F�[�_�[�f�[�^�N���X�̑O���錾
class ShaderData;

// �V�F�[�_�[�f�[�^�ۑ��N���X
class ShaderStorage : public Singleton<ShaderStorage> {

private:

	/*-- �����o�ϐ� --*/

	vector<shared_ptr<ShaderData>> shaderData;


public:

	/*-- �����o�֐� --*/

	// �V�F�[�_�[�����[�h����B
	ComPtr<ID3DBlob> LoadShader(const string& shaderPath, const string& shaderModel, const string& entryPoint);
	ComPtr<ID3DBlob> LoadShaderForDXC(const string& shaderPath, const string& shaderModel, const string& entryPoint);

	// �V�F�[�_�[�f�[�^��Ԃ��B
	ComPtr<ID3DBlob> GetShaderData(const string& shaderPath);
	ComPtr<IDxcBlob> GetShaderDataForDXC(const string& shaderPath);
	vector<char>& GetShaderBin(const string& shaderPath);

};

