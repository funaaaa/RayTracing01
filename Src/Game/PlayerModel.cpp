#include "PlayerModel.h"
#include "BLASRegister.h"
#include "PolygonInstanceRegister.h"
#include "Vec.h"
#include "HitGroupMgr.h"

void PlayerModel::Load(COLOR ColorID, bool IsGhost)
{

	/*===== モデルをロード =====*/

	LPCWSTR texPath = L"Resource/Game/red.png";

	switch (ColorID)
	{
	case PlayerModel::COLOR::RED:

		texPath = L"Resource/Game/red.png";

		break;
	case PlayerModel::COLOR::BLUE:

		texPath = L"Resource/Game/blue.png";

		break;
	default:

		break;
	}

	if (IsGhost) {

		// 車のボディをロード
		carBodyBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBody.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, true, true, true);
		carBodyInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBodyBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->AddScale(carBodyInsIndex_, Vec3(25, 25, 25));

		// 車のバンパーをロード
		carBumperBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBumper.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carBumperInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBumperBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBumperInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carLeftLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carLeftLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftLightInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carRightLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carRightLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightLightInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carFrontLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carFrontLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carFrontLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carFrontLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carFrontLightInsIndex_, carBodyInsIndex_);

		// 車の後ろタイヤをロード
		carBehindTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBehindTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carBehindTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBehindTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carBehindTireInsIndex_, Vec3(-0.089f, -0.882f, 1.101f));

		// 車の後ろタイヤのフレームをロード
		carBehindTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBehindTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carBehindTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBehindTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carBehindTireFrameInsIndex_, Vec3(-0.089f, -0.882f, 1.101f));

		// 右前タイヤをロード
		carRightTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carRightTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carRightTireInsIndex_, Vec3(0.84f, -0.87f, -1.55f));

		// 右前タイヤのフレームをロード
		carRightTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carRightTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carRightTireFrameInsIndex_, Vec3(0.85f, -0.86f, -1.55f));

		// 左前タイヤをロード
		carLeftTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carLeftTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carLeftTireInsIndex_, Vec3(-0.94f, -0.86f, -1.57f));

		// 左前タイヤのフレームをロード
		carLeftTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carLeftTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carLeftTireFrameInsIndex_, Vec3(-0.95f, -0.85f, -1.55f));

		// 車の鏡をロード
		carMirrorBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carMirror.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carMirrorInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carMirrorInsIndex_, carBodyInsIndex_);

		// 車の鏡のカバーをロード
		carMirrorCoverBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carMirrorCover.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carMirrorCoverInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carMirrorCoverInsIndex_, carBodyInsIndex_);

		// 車のウィンドウをロード
		carWindowBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carWindow.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/windowColor.png" }, false, true, true);
		carWindowInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carWindowBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carWindowInsIndex_, carBodyInsIndex_);

	}
	else {

		// 車のボディをロード
		carBodyBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBody.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, true, true, true);
		carBodyInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBodyBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFLECTION);
		PolygonInstanceRegister::Ins()->AddScale(carBodyInsIndex_, Vec3(25, 25, 25));

		// 車のバンパーをロード
		carBumperBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBumper.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carBumperInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBumperBlasIndex_, PolygonInstanceRegister::SHADER_ID::REFLECTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBumperInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carLeftLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carLeftLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftLightInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carRightLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carRightLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightLightInsIndex_, carBodyInsIndex_);

		// 車のライトをロード
		carFrontLightBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carFrontLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/white.png" }, false, true, true);
		carFrontLightInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carFrontLightBlasIndex_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		PolygonInstanceRegister::Ins()->SetParentInstance(carFrontLightInsIndex_, carBodyInsIndex_);

		// 車の後ろタイヤをロード
		carBehindTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBehindTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carBehindTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBehindTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carBehindTireInsIndex_, Vec3(-0.089f, -0.882f, 1.101f));

		// 車の後ろタイヤのフレームをロード
		carBehindTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carBehindTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carBehindTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carBehindTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carBehindTireFrameInsIndex_, Vec3(-0.089f, -0.882f, 1.101f));

		// 右前タイヤをロード
		carRightTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carRightTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carRightTireInsIndex_, Vec3(0.84f, -0.87f, -1.55f));

		// 右前タイヤのフレームをロード
		carRightTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carRightTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carRightTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carRightTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carRightTireFrameInsIndex_, Vec3(0.85f, -0.86f, -1.55f));

		// 左前タイヤをロード
		carLeftTireBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carLeftTireInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftTireInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carLeftTireInsIndex_, Vec3(-0.94f, -0.86f, -1.57f));

		// 左前タイヤのフレームをロード
		carLeftTireFrameBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carLeftTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { texPath }, false, true, true);
		carLeftTireFrameInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireFrameBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carLeftTireFrameInsIndex_, carBodyInsIndex_);
		PolygonInstanceRegister::Ins()->AddTrans(carLeftTireFrameInsIndex_, Vec3(-0.95f, -0.85f, -1.55f));

		// 車の鏡をロード
		carMirrorBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carMirror.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carMirrorInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlasIndex_, PolygonInstanceRegister::SHADER_ID::COMPLETE_REFLECTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carMirrorInsIndex_, carBodyInsIndex_);

		// 車の鏡のカバーをロード
		carMirrorCoverBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carMirrorCover.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/tireColor.png" }, true, true, true);
		carMirrorCoverInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlasIndex_, PolygonInstanceRegister::SHADER_ID::COMPLETE_REFLECTION);
		PolygonInstanceRegister::Ins()->SetParentInstance(carMirrorCoverInsIndex_, carBodyInsIndex_);

		// 車のウィンドウをロード
		carWindowBlasIndex_ = BLASRegister::Ins()->GenerateObj("Resource/Game/car/", "carWindow.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], { L"Resource/Game/car/windowColor.png" }, false, true, true);
		carWindowInsIndex_ = PolygonInstanceRegister::Ins()->CreateInstance(carWindowBlasIndex_, PolygonInstanceRegister::SHADER_ID::DEF);
		PolygonInstanceRegister::Ins()->SetParentInstance(carWindowInsIndex_, carBodyInsIndex_);

	}

	bodyInsIndex_.clear();
	tireInsIndex_.clear();
	carBlasIndex_.clear();


	// 車体のインデックスを保存。
	bodyInsIndex_.emplace_back(carBumperInsIndex_);
	bodyInsIndex_.emplace_back(carLeftLightInsIndex_);
	bodyInsIndex_.emplace_back(carRightLightInsIndex_);
	bodyInsIndex_.emplace_back(carFrontLightInsIndex_);
	bodyInsIndex_.emplace_back(carMirrorInsIndex_);
	bodyInsIndex_.emplace_back(carMirrorCoverInsIndex_);
	bodyInsIndex_.emplace_back(carWindowInsIndex_);
	bodyInsIndex_.emplace_back(carBodyInsIndex_);

	// タイヤのインデックスを保存。
	tireInsIndex_.emplace_back(carBehindTireInsIndex_);
	tireInsIndex_.emplace_back(carBehindTireFrameInsIndex_);
	tireInsIndex_.emplace_back(carRightTireInsIndex_);
	tireInsIndex_.emplace_back(carRightTireFrameInsIndex_);
	tireInsIndex_.emplace_back(carLeftTireInsIndex_);
	tireInsIndex_.emplace_back(carLeftTireFrameInsIndex_);

	// 車体のインデックスを保存。
	carBlasIndex_.emplace_back(carBumperBlasIndex_);
	carBlasIndex_.emplace_back(carLeftLightBlasIndex_);
	carBlasIndex_.emplace_back(carRightLightBlasIndex_);
	carBlasIndex_.emplace_back(carFrontLightBlasIndex_);
	carBlasIndex_.emplace_back(carMirrorBlasIndex_);
	carBlasIndex_.emplace_back(carMirrorCoverBlasIndex_);
	carBlasIndex_.emplace_back(carWindowBlasIndex_);
	carBlasIndex_.emplace_back(carBodyBlasIndex_);
	carBlasIndex_.emplace_back(carBehindTireBlasIndex_);
	carBlasIndex_.emplace_back(carBehindTireFrameBlasIndex_);
	carBlasIndex_.emplace_back(carRightTireBlasIndex_);
	carBlasIndex_.emplace_back(carRightTireFrameBlasIndex_);
	carBlasIndex_.emplace_back(carLeftTireBlasIndex_);
	carBlasIndex_.emplace_back(carLeftTireFrameBlasIndex_);
}

void PlayerModel::Delete()
{

	/*===== 車のインスタンスを削除 =====*/

	for (auto& index : tireInsIndex_) {

		PolygonInstanceRegister::Ins()->DestroyInstance(index);

	}
	for (auto& index : bodyInsIndex_) {

		PolygonInstanceRegister::Ins()->DestroyInstance(index);

	}

	for (auto& index : carBlasIndex_) {

		BLASRegister::Ins()->DeleteIndex(index);

	}

}
