#pragma once
#include <array>
#include <memory>
#include "BaseStageObject.h"
#include "Singleton.h"
#include "BaseStage.h"
#include "FHelper.h"

class StageObjectMgr {

private:

	/*===== メンバ変数 =====*/

	std::array<std::pair<std::shared_ptr<BaseStageObject>, bool>, 256> objects_;



public:

	/*===== メンバ変数 =====*/

	// セッティング
	void Setting();

	// 追加する。
	int AddObject(const BaseStageObject::OBJECT_ID& ObjectID, const BaseStageObject::COLLISION_ID& CollisionID, const std::string& DirectryPath, const std::string& ModelName, const std::wstring& HitGroupName, UINT ShaderID, bool IsOpaque = true);
	int AddObject(const BaseStageObject::OBJECT_ID& ObjectID, const BaseStageObject::COLLISION_ID& CollisionID, const std::wstring& ModelPath, const std::wstring& HitGroupName, UINT ShaderID, bool IsOpaque = true);

	// 更新処理
	void Update(int Timer);

	// 指定のインデックスの法線ベクトルを変更。
	void ChangeNormalTexture(int Index, int NormalTexture);

	// 当たり判定
	BaseStage::ColliderOutput Collider(BaseStage::ColliderInput Input);

	// ギミック集を追加。
	std::array<std::pair<std::shared_ptr<BaseStageObject>, bool>, 256> GetGimmickData() { return objects_; }

	// 移動関係
	void AddTrans(int Index, const Vec3& Trans);
	void ChangeTrans(int Index, const Vec3& Trans);

	// サイズ関係
	void AddScale(int Index, const Vec3& Scale);
	void ChangeScale(int Index, const Vec3& Scale);

	// 回転関係
	void AddRotate(int Index, const Vec3& Rotate);
	void ChangeRotate(int Index, const Vec3& Rotate);

	// Instanseを非表示、表示関数
	void NonDisplay(int Index);
	void Display(int Index);

	// ゲッタ
	int GetBlasIndex(int Index) { return objects_[Index].first->GetBLASIndex(); }
	int GetInstanceIndex(int Index) { return objects_[Index].first->GetInstanceIndex(); }

	// 削除。
	void DeleteIndex(int Index);
	void ChangeInstanceShaderID(std::weak_ptr<PolygonMeshInstance> Instance, UINT ShaderID);
	void ChangeInstanceShaderID(int Index, UINT ShaderID);


private:

	// ステージ、草との当たり判定
	BaseStage::ColliderOutput StageMeshCollider(BaseStage::ColliderInput& Input, FHelper::RayToModelCollisionData InputRayData, BaseStage::ColliderOutput Output, BaseStageObject::OBJECT_ID ObjectID);

	// 装飾オブジェクトとの当たり判定
	BaseStage::ColliderOutput OrnamentMeshCollider(BaseStage::ColliderInput& Input, FHelper::RayToModelCollisionData InputRayData, BaseStage::ColliderOutput Output);

	// 4方向に当たり判定
	BaseStage::ColliderOutput Decision4Way(BaseStage::ColliderInput& Input, FHelper::RayToModelCollisionData InputRayData, BaseStage::ColliderOutput Output, BaseStageObject::OBJECT_ID ObjectID);

	// 斜め床の回転
	void RotObliqueFloor(BaseStage::ColliderInput Input, const Vec3& HitNormal, BaseStage::ColliderOutput& Output);

};