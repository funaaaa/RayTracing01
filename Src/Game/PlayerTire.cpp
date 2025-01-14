#include "PlayerTire.h"
#include "PolygonInstanceRegister.h"
#include "PolygonInstance.h"
#include "FHelper.h"

PlayerTire::PlayerTire(std::weak_ptr<PolygonMeshInstance> TireInstance, bool IsBehindTire)
{

	/*===== コンストラクタ =====*/

	tireInstance_ = TireInstance;
	isBehindTire_ = IsBehindTire;
	pos_ = Vec3();
	prevPos_ = Vec3();
	rot_ = Vec3();

}

void PlayerTire::Update()
{

	/*===== 更新処理 =====*/

	// Y軸回転を0に近づける。
	rot_.y_ -= rot_.y_ / 10.0f;

	// 今フレームの座標を求める。
	pos_ = tireInstance_.lock()->GetWorldPos();

	// 前フレームとの距離を求める。
	float distance = (pos_ - prevPos_).Length();

	// 距離から回転させる量を決める。
	rot_.x_ -= distance / 10.0f;

	// 回転を一旦初期化。
	tireInstance_.lock()->ChangeRotate(Vec3(0, 0, 0));

	// 前輪だったら。
	if (!isBehindTire_) {

		tireInstance_.lock()->ChangeRotate(Vec3(0, rot_.y_, 0));

	}

	// 移動に伴う回転をさせる。
	Vec3 rotVec = FHelper::MulRotationMatNormal(Vec3(1, 0, 0), tireInstance_.lock()->GetRotate());

	DirectX::XMVECTOR quaternion = DirectX::XMQuaternionRotationNormal(rotVec.ConvertXMVECTOR(), rot_.x_);

	// クォータニオンを行列に治す。
	DirectX::XMMATRIX quaternionMat = DirectX::XMMatrixRotationQuaternion(quaternion);
	tireInstance_.lock()->AddRotate(quaternionMat);

	// 座標を保存。
	prevPos_ = pos_;

}

void PlayerTire::Rot(bool IsDrift, float Rate)
{

	/*===== 回転処理 =====*/

	const float DRIFT_ROT = 0.5f;
	const float NORMAL_ROT = 0.3f;

	// ドリフトだったら
	if (IsDrift) {

		rot_.y_ = DRIFT_ROT * Rate;

	}
	else {

		rot_.y_ = NORMAL_ROT * Rate;

	}

}
