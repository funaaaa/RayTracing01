#include "BLASRegister.h"
#include "BLAS.h"

const int& BLASRegister::GenerateObj(const std::string& DirectryPath, const std::string& ModelName, const std::wstring& HitGroupName, const LPCWSTR& TexturePath)
{

	/*===== BLASを生成 =====*/

	blas.push_back(std::make_shared<BLAS>());
	blas.back()->GenerateBLASObj(DirectryPath, ModelName, HitGroupName, TexturePath);

	return blas.size() - 1;

}

const int& BLASRegister::GenerateFbx(const std::string& DirectryPath, const std::string& ModelName, const std::wstring& HitGroupName, const LPCWSTR& TextureHandle)
{

	/*===== BLASを生成 =====*/

	blas.push_back(std::make_shared<BLAS>());
	blas.back()->GenerateBLASFbx(DirectryPath, ModelName, HitGroupName, TextureHandle);

	return blas.size() - 1;

}

const int& BLASRegister::GenerateData(Object3DDeliveryData Data, const wstring& HitGroupName, const int& TextureID)
{

	/*===== BLASを生成 =====*/

	blas.push_back(std::make_shared<BLAS>());
	blas.back()->GenerateBLASData(Data, HitGroupName, TextureID);

	return blas.size() - 1;

}

void BLASRegister::Update(const int& Index)
{

	/*===== 指定のBLASを更新 =====*/

	blas[Index]->Update();

}

void BLASRegister::ComputeSkin(const int& Index)
{

	/*===== 指定のBLASのスキニングアニメーションを計算 =====*/

	blas[Index]->ComputeSkin();

}

void BLASRegister::InitAnimation(const int& Index)
{

	/*===== 指定のBLASのスキニングアニメーションを初期化 =====*/

	blas[Index]->InitAnimation();

}

void BLASRegister::PlayAnimation(const int& Index)
{

	/*===== 指定のBLASのスキニングアニメーションを開始 =====*/

	blas[Index]->PlayAnimation();

}

void BLASRegister::StopAnimation(const int& Index)
{

	/*===== 指定のBLASのスキニングアニメーションを停止 =====*/

	blas[Index]->StopAnimation();

}

uint8_t* BLASRegister::WriteShaderRecord(uint8_t* Dst, const int& Index, UINT RecordSize, Microsoft::WRL::ComPtr<ID3D12StateObject>& StateObject)
{

	/*===== シェーダーレコードを書き込む =====*/

	return blas[Index]->WriteShaderRecord(Dst, RecordSize, StateObject);

}

Microsoft::WRL::ComPtr<ID3D12Resource>& BLASRegister::GetBLASBuffer(const int& Index)
{
	return blas[Index]->GetBLASBuffer();
}
