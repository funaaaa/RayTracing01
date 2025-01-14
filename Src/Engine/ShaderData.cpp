#include "ShaderData.h"
#include <d3dcompiler.h>
#include <system_error>
#include <fstream>
#include <iostream>
#include <sstream>
#include <array>
#include "FString.h"

#pragma comment(lib, "dxcompiler.lib")

ShaderData::ShaderData(const std::string& shaderPath_, const std::string& entryPoint_, const std::string& shaderModel_, bool isDXC)
{

	/*-- コンストラクタ --*/

	// 引数を保存。
	this->shaderPath_ = shaderPath_;
	this->entryPoint_ = entryPoint_;
	this->shaderModel_ = shaderModel_;
	this->shaderBlob_ = nullptr;

	if (isDXC) {

		//シェーダーをロード。
		LoadShaderDXC();

	}
	else {

		//シェーダーをロード。
		LoadShader();

	}

}

void ShaderData::LoadShader()
{

	/*-- シェーダーをロードする処理 --*/

	std::array<wchar_t, 128> shaderPathBuff;
	FString::ConvertStringToWchar_t(shaderPath_, shaderPathBuff.data(), 128);

	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(
		shaderPathBuff.data(),								//シェーダファイル名
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,					//インクルード可能にする
		entryPoint_.c_str(), shaderModel_.c_str(),			//エントリーポイント名、シェーダーモデル指定
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	//デバッグ用設定
		0,
		&shaderBlob_, &errorBlob);

	// シェーダーのエラーチェック
	if (FAILED(result)) {

		// hresultからエラーメッセージを取得
		std::string errorMsg = std::system_category().message(result);

		// errorBlobからエラー内容をstring型にコピー
		std::string errstr;
		errstr.resize(errorBlob->GetBufferSize());

		std::copy_n((char*)errorBlob->GetBufferPointer(),
			errorBlob->GetBufferSize(),
			errstr.begin());
		errstr += "\n";
		// エラー内容を出力ウィンドウに表示
		OutputDebugStringA(errstr.c_str());
		exit(1);
	}

}

void ShaderData::LoadShaderDXC()
{
	HRESULT hr;
	std::ifstream infile(shaderPath_, std::ifstream::binary);
	if (!infile) {
		throw std::runtime_error("failed shader compile.");
	}

	// 型変換。
	std::wstring fileName = FString::StringToWString(shaderPath_);
	std::stringstream strstream;

	strstream << infile.rdbuf();

	std::string shaderCode_ = strstream.str();
	Microsoft::WRL::ComPtr<IDxcLibrary> library;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	Microsoft::WRL::ComPtr<IDxcCompiler> compiler;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> source_;

	library->CreateBlobWithEncodingFromPinned(
		(LPBYTE)shaderCode_.c_str(), static_cast<UINT32>(shaderCode_.size()), CP_UTF8, &source_);
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

	// インクルードを使う場合には適切に設定すること
	library->CreateIncludeHandler(&includeHandler);
	// コンパイルオプションの指定
	std::vector<LPCWSTR> arguments;

	arguments.emplace_back(L"/enable-16bit-types");		// 16bit変数を使用可能にする。

	// シェーダーモデルは一旦これで固定する。
	const auto target_ = L"lib_6_4";

	Microsoft::WRL::ComPtr<IDxcOperationResult> dxcResult;
	hr = compiler->Compile(source_.Get(), fileName.c_str(),
		L"", target_, arguments.data(), static_cast<UINT>(arguments.size()),
		nullptr, 0, includeHandler.Get(), &dxcResult);

	if (FAILED(hr)) {
		throw std::runtime_error("failed shader compile.");
	}

	dxcResult->GetStatus(&hr);
	if (FAILED(hr)) {
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> errBlob;
		dxcResult->GetErrorBuffer(&errBlob);
		throw std::runtime_error("failed shader compile");
	}

	Microsoft::WRL::ComPtr<IDxcBlob> blob;
	dxcResult->GetResult(&blob);

	shaderBlobDxc_ = blob;

	std::vector<char> result;
	auto size = blob->GetBufferSize();
	result.resize(size);
	memcpy(result.data(), blob->GetBufferPointer(), size);
	shaderBin_ = result;

}