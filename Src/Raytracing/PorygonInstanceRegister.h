#pragma once
#include "Singleton.h"
#include "Vec.h"

#include <DirectXMath.h>
#include <vector>
#include <d3d12.h>
#include <memory>
#include <wrl.h>

class PorygonMeshInstance;

// ポリゴンインスタンスの参照を保存するクラス
class PorygonInstanceRegister : public Singleton<PorygonInstanceRegister> {

private:

	/*===== メンバ変数 =====*/

	std::vector<std::shared_ptr<PorygonMeshInstance>> instance;

	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDesc;

	using XMMATRIX = DirectX::XMMATRIX;


public:

	/*===== メンバ関数 =====*/

	// Insを生成する。
	int CreateInstance(const int& BlasIndex, const UINT& instanceID);

	// 移動(引数を加算)関数
	void AddTrans(const int& Index, const float& X, const float& Y, const float Z);
	void AddTrans(const int& Index, const Vec3& Pos);

	// 回転(ラジアン、引数を加算)関数
	void AddRotate(const int& Index, const float& X, const float& Y, const float Z);
	void AddRotate(const int& Index, const Vec3& Pos);

	// レジスターのDataを取得する関数。
	D3D12_RAYTRACING_INSTANCE_DESC* GetData() { return instanceDesc.data(); };

	// レジスターのサイズを取得する関数。
	inline const UINT& GetRegisterSize() { return UINT(instance.size()); }

};