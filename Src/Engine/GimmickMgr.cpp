#include "GimmickMgr.h"
#include "BoostGimmick.h"
#include "BLASRegister.h"

int GimmickMgr::AddGimmick(const BaseGimmick::ID& GimmickID, const std::string& DirectryPath, const std::string& ModelName, std::vector<LPCWSTR> TexturePath, const std::wstring& HitGroupName, const UINT& ShaderID)
{

	/*===== ギミックを追加 =====*/

	// IDがブーストだったら
	if (GimmickID == BaseGimmick::ID::BOOST) {
		gimmicks_.emplace_back(std::make_shared<BoostGimmick>());
	}

	// Blasをロード
	int blasIndex = BLASRegister::Ins()->GenerateObj(DirectryPath, ModelName, HitGroupName, TexturePath);
	// instanceを生成。
	int instanceIndex = PolygonInstanceRegister::Ins()->CreateInstance(blasIndex, ShaderID);

	gimmicks_.back()->Setting(GimmickID, instanceIndex);

	return static_cast<int>(gimmicks_.size()) - 1;

}

void GimmickMgr::AddTrans(const int& GimmickIndex, const Vec3& Trans)
{

	/*===== 移動を加算 =====*/

	gimmicks_[GimmickIndex]->AddTrans(Trans);

}

void GimmickMgr::ChangeTrans(const int& GimmickIndex, const Vec3& Trans)
{

	/*===== 移動を代入 =====*/

	gimmicks_[GimmickIndex]->ChangeTrans(Trans);

}

void GimmickMgr::AddScale(const int& GimmickIndex, const Vec3& Scale)
{

	/*===== スケールを加算 =====*/

	gimmicks_[GimmickIndex]->AddScale(Scale);

}

void GimmickMgr::ChangeScale(const int& GimmickIndex, const Vec3& Scale)
{

	/*===== スケールを代入 =====*/

	gimmicks_[GimmickIndex]->ChangeScale(Scale);

}

void GimmickMgr::AddRotate(const int& GimmickIndex, const Vec3& Rotate)
{

	/*===== 回転を加算 =====*/

	gimmicks_[GimmickIndex]->AddRotate(Rotate);

}

void GimmickMgr::ChangeRotate(const int& GimmickIndex, const Vec3& Rotate)
{

	/*===== 回転を代入 =====*/

	gimmicks_[GimmickIndex]->ChangeRotate(Rotate);

}
