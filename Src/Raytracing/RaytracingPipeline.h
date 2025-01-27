#pragma once
#include <string>
#include <wtypes.h>
#include <vector>
#include <array>
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include "WindowsAPI.h"

class RayRootsignature;

struct RayPipelineShaderData {

	std::string shaderPath_;					// シェーダーパス
	std::vector<LPCWSTR> rayGenEnteryPoint_;	// エントリポイント
	std::vector<LPCWSTR> missEntryPoint_;		// エントリポイント
	std::vector<LPCWSTR> hitgroupEntryPoint_;	// エントリポイント
	RayPipelineShaderData() {};
	RayPipelineShaderData(const std::string& ShaderPath, const std::vector<LPCWSTR>& RGEntry, const std::vector<LPCWSTR>& MSEntry, const std::vector<LPCWSTR>& HGEntry)
		:shaderPath_(ShaderPath), rayGenEnteryPoint_(RGEntry), missEntryPoint_(MSEntry), hitgroupEntryPoint_(HGEntry) {};

};

// レイトレーシングで使用するパイプライン
class RaytracingPipeline {

protected:

	/*===== メンバ変数 =====*/

	std::vector<RayPipelineShaderData> shaderData_;			// 使用するシェーダーを纏めた構造体
	std::vector<D3D12_SHADER_BYTECODE> shaderCode_;			// 使用するシェーダーのバイトコード
	std::array<Microsoft::WRL::ComPtr<ID3D12StateObject>, 2> stateObject_;	// ステートオブジェクト
	std::shared_ptr<RayRootsignature> globalRootSig_;		// グローバルルートシグネチャ
	std::array<D3D12_DISPATCH_RAYS_DESC, 2> dispatchRayDesc_;				// レイ発射時の設定
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> shaderTable_;		// シェーダーテーブル
	std::array<void*, 2> shaderTalbeMapAddress_;							// シェーダーテーブルのデータ転送用Mapアドレス
	std::array<Microsoft::WRL::ComPtr<ID3D12StateObjectProperties>, 2> rtsoProps_;
	LPCWSTR hitGroupName_;


	// シェーダーテーブルの構築に必要な変数群
	UINT raygenRecordSize_;
	UINT missRecordSize_;
	UINT hitgroupRecordSize_;
	UINT hitgroupCount;
	UINT raygenSize_;
	UINT missSize_;
	UINT hitGroupSize_;
	UINT tableAlign_;
	UINT hitgroupRegion_;
	UINT tableSize_;
	UINT raygenRegion_;
	UINT missRegion_;


public:

	/*===== メンバ関数 =====*/

	// セッティング処理
	void Setting(const std::vector<RayPipelineShaderData>& InputData, int UseHitGroup, int SRVCount, int CBVCount, int UAVCount, int PayloadSize, int AttribSize, int ReflectionCount = 4);

	// シェーダーテーブルを構築
	void ConstructionShaderTable(int DispatchX = WINDOW_WIDTH, int DispatchY = WINDOW_HEIGHT);

	// HitGroupの情報を転送。
	void MapHitGroupInfo();

	// ゲッタ
	Microsoft::WRL::ComPtr<ID3D12StateObject> GetStateObject(int Index) { return stateObject_[Index]; }
	D3D12_DISPATCH_RAYS_DESC GetDispatchRayDesc(int Index) { return dispatchRayDesc_[Index]; }
	std::shared_ptr<RayRootsignature> GetGlobalRootSig() { return globalRootSig_; }

protected:

	// ヒットグループの中で最も使用データサイズが大きものを取得する。
	UINT GetLargestDataSizeInHitGroup();

	// アライメント
	UINT RoundUp(size_t Size, UINT Align) {
		return UINT(Size + Align - 1) & ~(Align - 1);
	}

	// シェーダーテーブルを書き込み、レイを設定する。
	void WriteShadetTalbeAndSettingRay(int Index, int DispatchX, int DispatchY);

	// シェーダー識別子を書き込む。
	UINT WriteShaderIdentifier(void* Dst, const void* ShaderId);

	// RayGenerationシェーダーの数を取得。
	int GetRayGenerationCount();

	// MissShaderの数を取得。
	int GetMissCount();

};