#include "PlayerModel.h"
#include "BLASRegister.h"
#include "BLAS.h"
#include "PolygonInstanceRegister.h"
#include "PolygonInstance.h"
#include "Vec.h"
#include "HitGroupMgr.h"

void PlayerModel::Load(COLOR ColorID, bool IsGhost)
{

	/*===== モデルをロード =====*/

	// 使用するディレクトリ
	std::string folder = "Red/";
	if (IsGhost) {

		folder = "Blue/";

	}

	if (IsGhost) {

		// 車のボディをロード
		carBodyBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "body.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBodyInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBodyBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carBodyInstance.lock()->AddScale(Vec3(25, 25, 25));

		// 車のバンパーをロード
		carBumperBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "bumper.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBumperInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBumperBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carBumperInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carLeftLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftLightBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carLeftLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carRightLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightLightBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carRightLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carFrontLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "frontLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carFrontLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carFrontLightBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carFrontLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車の後ろタイヤをロード
		carBehindTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "behindTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBehindTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carBehindTireInstance.lock()->SetParentInstance(carBodyInstance);
		carBehindTireInstance.lock()->AddTrans(Vec3(-0.089f, -0.882f, 1.101f));

		// 車の後ろタイヤのフレームをロード
		carBehindTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "behindTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBehindTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carBehindTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carBehindTireFrameInstance.lock()->AddTrans(Vec3(-0.089f, -0.882f, 1.101f));

		// 右前タイヤをロード
		carRightTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carRightTireInstance.lock()->SetParentInstance(carBodyInstance);
		carRightTireInstance.lock()->AddTrans(Vec3(0.84f, -0.87f, -1.55f));

		// 右前タイヤのフレームをロード
		carRightTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carRightTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carRightTireFrameInstance.lock()->AddTrans(Vec3(0.85f, -0.86f, -1.55f));

		// 左前タイヤをロード
		carLeftTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carLeftTireInstance.lock()->SetParentInstance(carBodyInstance);
		carLeftTireInstance.lock()->AddTrans(Vec3(-0.94f, -0.86f, -1.57f));

		// 左前タイヤのフレームをロード
		carLeftTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carLeftTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carLeftTireFrameInstance.lock()->AddTrans(Vec3(-0.95f, -0.85f, -1.55f));

		// 車の鏡をロード
		carMirrorBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "mirror.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carMirrorInstance = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carMirrorInstance.lock()->SetParentInstance(carBodyInstance);

		// 車の鏡のカバーをロード
		carMirrorCoverBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "mirrorCover.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carMirrorCoverInstance = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carMirrorCoverInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のウィンドウをロード
		carWindowBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "window.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carWindowInsInstance = PolygonInstanceRegister::Ins()->CreateInstance(carWindowBlas_, PolygonInstanceRegister::SHADER_ID::REFRACTION);
		carWindowInsInstance.lock()->SetParentInstance(carBodyInstance);

	}
	else {

		// 車のボディをロード
		carBodyBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "body.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBodyInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBodyBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carBodyInstance.lock()->AddScale(Vec3(25, 25, 25));

		// 車のバンパーをロード
		carBumperBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "bumper.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBumperInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBumperBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carBumperInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carLeftLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftLightBlas_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		carLeftLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carRightLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightLightBlas_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		carRightLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のライトをロード
		carFrontLightBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "frontLight.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carFrontLightInstance = PolygonInstanceRegister::Ins()->CreateInstance(carFrontLightBlas_, PolygonInstanceRegister::SHADER_ID::TEXCOLOR);
		carFrontLightInstance.lock()->SetParentInstance(carBodyInstance);

		// 車の後ろタイヤをロード
		carBehindTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "behindTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBehindTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carBehindTireInstance.lock()->SetParentInstance(carBodyInstance);
		carBehindTireInstance.lock()->AddTrans(Vec3(-0.089f, -0.882f, 1.101f));

		// 車の後ろタイヤのフレームをロード
		carBehindTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "behindTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carBehindTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carBehindTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carBehindTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carBehindTireFrameInstance.lock()->AddTrans(Vec3(-0.089f, -0.882f, 1.101f));

		// 右前タイヤをロード
		carRightTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carRightTireInstance.lock()->SetParentInstance(carBodyInstance);
		carRightTireInstance.lock()->AddTrans(Vec3(0.84f, -0.87f, -1.55f));

		// 右前タイヤのフレームをロード
		carRightTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "rightTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carRightTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carRightTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carRightTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carRightTireFrameInstance.lock()->AddTrans(Vec3(0.85f, -0.86f, -1.55f));

		// 左前タイヤをロード
		carLeftTireBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftTire.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftTireInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carLeftTireInstance.lock()->SetParentInstance(carBodyInstance);
		carLeftTireInstance.lock()->AddTrans(Vec3(-0.94f, -0.86f, -1.57f));

		// 左前タイヤのフレームをロード
		carLeftTireFrameBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "leftTireFrame.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carLeftTireFrameInstance = PolygonInstanceRegister::Ins()->CreateInstance(carLeftTireFrameBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carLeftTireFrameInstance.lock()->SetParentInstance(carBodyInstance);
		carLeftTireFrameInstance.lock()->AddTrans(Vec3(-0.95f, -0.85f, -1.55f));

		// 車の鏡をロード
		carMirrorBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "mirror.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carMirrorInstance = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carMirrorInstance.lock()->SetParentInstance(carBodyInstance);

		// 車の鏡のカバーをロード
		carMirrorCoverBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "mirrorCover.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carMirrorCoverInstance = PolygonInstanceRegister::Ins()->CreateInstance(carMirrorBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carMirrorCoverInstance.lock()->SetParentInstance(carBodyInstance);

		// 車のウィンドウをロード
		carWindowBlas_ = BLASRegister::Ins()->GenerateObj("Resource/Game/CAR/OBJ/" + folder, "window.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DEF], true, true);
		carWindowInsInstance = PolygonInstanceRegister::Ins()->CreateInstance(carWindowBlas_, PolygonInstanceRegister::SHADER_ID::DEF);
		carWindowInsInstance.lock()->SetParentInstance(carBodyInstance);

	}

	bodyInstance.clear();
	tireInstance.clear();
	carBlasIndex_.clear();


	// 車体のインデックスを保存。
	bodyInstance.emplace_back(carBumperInstance);
	bodyInstance.emplace_back(carLeftLightInstance);
	bodyInstance.emplace_back(carRightLightInstance);
	bodyInstance.emplace_back(carFrontLightInstance);
	bodyInstance.emplace_back(carMirrorInstance);
	bodyInstance.emplace_back(carMirrorCoverInstance);
	bodyInstance.emplace_back(carWindowInsInstance);
	bodyInstance.emplace_back(carBodyInstance);

	// タイヤのインデックスを保存。
	tireInstance.emplace_back(carBehindTireInstance);
	tireInstance.emplace_back(carBehindTireFrameInstance);
	tireInstance.emplace_back(carRightTireInstance);
	tireInstance.emplace_back(carRightTireFrameInstance);
	tireInstance.emplace_back(carLeftTireInstance);
	tireInstance.emplace_back(carLeftTireFrameInstance);

	// 車体のインデックスを保存。
	carBlasIndex_.emplace_back(carBumperBlas_);
	carBlasIndex_.emplace_back(carLeftLightBlas_);
	carBlasIndex_.emplace_back(carRightLightBlas_);
	carBlasIndex_.emplace_back(carFrontLightBlas_);
	carBlasIndex_.emplace_back(carMirrorBlas_);
	carBlasIndex_.emplace_back(carMirrorCoverBlas_);
	carBlasIndex_.emplace_back(carWindowBlas_);
	carBlasIndex_.emplace_back(carBodyBlas_);
	carBlasIndex_.emplace_back(carBehindTireBlas_);
	carBlasIndex_.emplace_back(carBehindTireFrameBlas_);
	carBlasIndex_.emplace_back(carRightTireBlas_);
	carBlasIndex_.emplace_back(carRightTireFrameBlas_);
	carBlasIndex_.emplace_back(carLeftTireBlas_);
	carBlasIndex_.emplace_back(carLeftTireFrameBlas_);
}

void PlayerModel::Delete()
{

	/*===== 車のインスタンスを削除 =====*/

	for (auto& index : tireInstance) {

		PolygonInstanceRegister::Ins()->DestroyInstance(index);

	}
	for (auto& index : bodyInstance) {

		PolygonInstanceRegister::Ins()->DestroyInstance(index);

	}

	for (auto& index : carBlasIndex_) {

		index.lock()->Init();

	}

}
