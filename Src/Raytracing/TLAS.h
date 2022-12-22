#pragma once
#include "Engine.h"
#include <array>

// TLASクラス
class TLAS {

private:

	/*===== メンバ変数 =====*/

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> tlasBuffer_;			// TLAS用バッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> scratchBuffer_;		// スクラッチバッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> instanceDescBuffer_;	// インスタンスバッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> tlasUpdateBuffer_;	// TLAS更新用バッファ

	int descriptorHeapIndex_;


public:

	/*===== メンバ関数 =====*/

	// TLASの生成
	void GenerateTLAS();

	// アクセッサ
	Microsoft::WRL::ComPtr<ID3D12Resource> GetTLASBuffer(int Index) { return tlasBuffer_[Index]; }

	// ディスクリプタヒープのインデックスのアクセッサ
	inline int GetDescriptorHeapIndex() { return descriptorHeapIndex_; }

	// インスタンスを更新
	void Update();


private:

	// アドレスに情報を書き込む処理
	void WriteToMemory(Microsoft::WRL::ComPtr<ID3D12Resource>& Resource, const void* PData, size_t DataSize);

	// 加速構造体の設定用関数
	void SettingAccelerationStructure(int Index);

	// 加速構造体の構築用関数
	void CreateAccelerationStructure(int Index);

};
