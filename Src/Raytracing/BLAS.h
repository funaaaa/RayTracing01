#pragma once
#include "Engine.h"
#include "RayDescriptor.h"
#include "ModelDataManager.h"
#include "FbxLoader.h"
#include "Vec.h"
#include "DynamicConstBuffer.h"
#include "DoubleResourceWrapper.h"
#include <DirectXMath.h>
#include <array>
#include <memory>

// 二重バッファ用クラス
class DoubleResourceWrapper;
class DoubleRayDescriptor;

// レイトレ用頂点構造体
struct RayVertex {

	Vec3 position_;
	Vec3 normal_;
	Vec2 uv_;
	Vec2 subUV_;
	Vec2 pad_;

};

// モデルの基本情報をまとめた構造体
struct ModelInfo {
	UINT vertexCount_;	// 頂点の数
	UINT indexCount_;	// 頂点インデックスの数
	UINT vertexStride_;	// 1頂点のデータサイズ
	UINT indexStride_;	// 1頂点インデックスのデータサイズ
	Vec3 vertexMax_;	// 頂点の各成分の最大値
	Vec3 vertexMin_;	// 頂点の各成分の最小値
	int modelIndex_;	// モデルのインデックス
	bool isOpaque_;		// 不透明フラグ
};

// 頂点情報
struct VertexInfo {
	std::vector<RayVertex> defVertex_;	// 生成した時点の頂点
	std::vector<RayVertex> vertex_;		// 現在の頂点 頂点を書き換える場合があるのでその時用
	std::vector<Vec3> vertexPos_;
	std::vector<Vec3> vertexNormal_;
	std::vector<Vec2> vertexUV_;
	std::vector<UINT> vertIndex_;
};

// BLASの情報
struct BLASInfo {
	std::shared_ptr<DoubleResourceWrapper> vertexBuffer_;			// 頂点バッファ
	std::shared_ptr<DoubleResourceWrapper> indexBuffer_;			// 頂点インデックスバッファ
	std::shared_ptr<DoubleResourceWrapper> materialBuffer_;			// 頂点インデックスバッファ
	std::shared_ptr<DoubleRayDescriptor> vertexDescriptor_;			// 頂点ディスクリプタ
	std::shared_ptr<DoubleRayDescriptor> indexDescriptor_;			// 頂点インデックスディスクリプタ
	std::shared_ptr<DoubleRayDescriptor> materialDescriptor_;		// マテリアル情報用ディスクリプタ
	ModelDataManager::GPUMaterial material_;						// マテリアル情報用定数バッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> blasBuffer_;		// BLAS用バッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> scratchBuffer_;	// スクラッチバッファ
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> updateBuffer_;	// 更新用バッファ
	int blasIndex_;			// BLASのインデックス。
	bool isGenerate_;
};

// HitGroupの情報
struct HitGroupInfo {
	std::wstring hitGroupName_;				// 使用するヒットグループの名前
	std::string modelPath_;					// 使用するモデルのパス
	std::vector<LPCWSTR> texturePath_;		// 使用するテクスチャのパス
	std::array<bool, 2> isChangeTexture_;	// テクスチャを書き換えたかフラグ
	std::array<bool, 2> isChangeVertex_;	// 頂点を書き換えたかフラグ
	int baseTextureHandle_;					// 使用するテクスチャのハンドル
	int mapTextureHandle_;
	std::vector<int> uavHandle_;			// 使用するUAVのハンドル
};

// ポリゴン形状を保存してあるBLASクラス
class BLAS {

private:

	/*===== メンバ変数 =====*/

	// モデルの基本情報
	ModelInfo modelInfo_;

	// 頂点情報用
	VertexInfo vertexInfo_;

	// BLASの基本的な情報
	BLASInfo blasInfo_;

	// HitGroupの情報
	HitGroupInfo hitGroupInfo_;


private:

	std::vector<FbxLoader::SkinComputeInput> skinComputeInput_;

public:

	enum class MAP_PARAM {
		NONE,
		NORMAL,
		SPECULAR,
		AO
	};


public:

	/*===== メンバ関数 =====*/

	BLAS();

	// 初期化処理
	void Init();

	// BLASの生成
	void GenerateBLASObj(const std::string& DirectryPath, const std::string& ModelName, const std::wstring& HitGroupName, int BlasIndex, bool IsOpaque = false);
	void GenerateBLASGLTF(const std::wstring& Path, const std::wstring& HitGroupName, int BlasIndex, bool IsOpaque = false);
	void GenerateBLASData(const ModelDataManager::ObjectData& ModelData, int BlasIndex, bool IsOpaque = false);

	// BLASの更新
	void Update();

	// アニメーションの有効化
	void InitAnimation();	// 初期化
	void PlayAnimation();	// 再生
	void StopAnimation();	// 停止

	// マテリアルの参照を取得。
	ModelDataManager::GPUMaterial& GetMaterial() { return blasInfo_.material_; }

	// マテリアルを書き換えた判定。
	void IsChangeMaterial();

	// テクスチャを変えたフラグ。
	void ChangeTextureFlag() {
		hitGroupInfo_.isChangeTexture_[0] = true;
		hitGroupInfo_.isChangeTexture_[1] = true;
	}

	// テクスチャを追加。
	void ChangeBaseTexture(int Index);
	void ChangeMapTexture(int Index, MAP_PARAM MapParam);
	void AddUAVTex(int Index) { hitGroupInfo_.uavHandle_.emplace_back(Index); }

	// シェーダーレコードを書き込む。
	uint8_t* WriteShaderRecord(uint8_t* Dst, UINT recordSize, Microsoft::WRL::ComPtr<ID3D12StateObject>& StateObject, LPCWSTR HitGroupName, int Index);

	// 各成分の長さの最大を返す。
	Vec3 GetVertexLengthMax();

	// 全ての頂点にVec3情報をかける。 重い処理なので動的には呼ばない。
	void MulVec3Vertex(Vec3 Vec);

	// 指定のUVをSUBUVに代入する。
	void AssignUV(const std::vector<RayVertex>& UV);

	// 頂点情報の座標成分のみを変更。
	void ChangeVertexPosition(int Index, const Vec3& Pos);

	// アクセッサ
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetBLASBuffer(int Index) { return blasInfo_.blasBuffer_[Index]; }
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetVertexBuffer(int Index) { return blasInfo_.vertexBuffer_->GetBuffer(Index); }
	Microsoft::WRL::ComPtr<ID3D12Resource>& GetIndexBuffer(int Index) { return blasInfo_.indexBuffer_->GetBuffer(Index); }
	std::wstring& GetHitGroupName() { return hitGroupInfo_.hitGroupName_; }
	const std::string& GetModelPath() { return hitGroupInfo_.modelPath_; }
	const std::vector<LPCWSTR>& GetTexturePath() { return hitGroupInfo_.texturePath_; }
	const Vec3& GetVertexMin() { return modelInfo_.vertexMin_; }
	const Vec3& GetVertexMax() { return modelInfo_.vertexMax_; }
	bool GetIsGenerate() { return blasInfo_.isGenerate_; }
	int GetBlasIndex() { return blasInfo_.blasIndex_; }
	int GetBaseTexture() { return hitGroupInfo_.baseTextureHandle_; }

	std::vector<RayVertex> GetVertex() { return vertexInfo_.vertex_; }
	const std::vector<Vec3>& GetVertexPos() { return vertexInfo_.vertexPos_; }
	const std::vector<Vec3>& GetVertexNormal() { return vertexInfo_.vertexNormal_; }
	const std::vector<Vec2>& GetVertexUV() { return vertexInfo_.vertexUV_; };
	const std::vector<UINT>& GetVertexIndex() { return vertexInfo_.vertIndex_; }

private:

	// BLAS生成時に設定を取得する関数
	D3D12_RAYTRACING_GEOMETRY_DESC GetGeometryDesc(bool IsOpaque, int Index);

	// 加速構造体の設定用関数
	void SettingAccelerationStructure(const D3D12_RAYTRACING_GEOMETRY_DESC& geomDesc, int Index);

	// 加速構造体の構築用関数
	void CreateAccelerationStructure(int Index);

	// マテリアルを設定
	void CreateMaterialBuffer();

	// 頂点バッファを設定
	void CreateVertexBuffer(const ModelDataManager::ObjectData& DataBuff);

	// GPUディスクリプタを書き込む。
	inline UINT WriteGPUDescriptor(void* Dst, const D3D12_GPU_DESCRIPTOR_HANDLE* Descriptor)
	{
		memcpy(Dst, Descriptor, sizeof(Descriptor));
		return static_cast<UINT>((sizeof(Descriptor)));
	}
	inline UINT WriteGPUDescriptor(void* Dst, const D3D12_GPU_VIRTUAL_ADDRESS* Descriptor)
	{
		memcpy(Dst, Descriptor, sizeof(Descriptor));
		return static_cast<UINT>((sizeof(Descriptor)));
	}
};