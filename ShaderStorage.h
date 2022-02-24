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

// シェーダーデータクラスの前方宣言
class ShaderData;

// シェーダーデータ保存クラス
class ShaderStorage : public Singleton<ShaderStorage> {

private:

	/*-- メンバ変数 --*/

	vector<shared_ptr<ShaderData>> shaderData;


public:

	/*-- メンバ関数 --*/

	// シェーダーをロードする。
	ComPtr<ID3DBlob> LoadShader(const string& shaderPath, const string& shaderModel, const string& entryPoint);
	ComPtr<ID3DBlob> LoadShaderForDXC(const string& shaderPath, const string& shaderModel, const string& entryPoint);

	// シェーダーデータを返す。
	ComPtr<ID3DBlob> GetShaderData(const string& shaderPath);
	ComPtr<IDxcBlob> GetShaderDataForDXC(const string& shaderPath);
	vector<char>& GetShaderBin(const string& shaderPath);

};


