#include "Player.h"
#include "BLASRegister.h"
#include "PorygonInstanceRegister.h"
#include "HitGroupMgr.h"
#include "Input.h"
#include "FHelper.h"

void Player::Init()
{

	/*===== 初期化処理 =====*/

	int carBlasIndex = BLASRegister::Ins()->GenerateObj("Resource/", "car.obj", HitGroupMgr::Ins()->hitGroupNames[HitGroupMgr::DENOISE_AO_HIT_GROUP], { L"Resource/white.png" });
	carInstanceIndex = PorygonInstanceRegister::Ins()->CreateInstance(carBlasIndex, 2);
	PorygonInstanceRegister::Ins()->AddScale(carInstanceIndex, Vec3(10, 10, 10));

	pos = Vec3();
	forwardVec = Vec3(0, 0, 1);
	speed = 0;

}

void Player::Update()
{

	/*===== 更新処理 =====*/

	// 入力処理
	Input();

	// 移動処理
	Move();

	// 座標を更新。
	PorygonInstanceRegister::Ins()->ChangeTrans(carInstanceIndex, pos);

}

void Player::Draw()
{

	/*===== 描画処理 =====*/

}

Vec3 Player::GetUpVec()
{

	/*===== 上ベクトルを取得 =====*/

	// 回転行列を取得。
	DirectX::XMMATRIX matRot = PorygonInstanceRegister::Ins()->GetRotate(carInstanceIndex);

	// デフォルトの上ベクトル。
	Vec3 defUpVec = Vec3(0, 1, 0);

	// デフォルトの上ベクトルに回転行列をかけた値を返す。
	return FHelper::MulRotationMatNormal(defUpVec, matRot);

}

void Player::Input()
{

	/*===== 入力処理 =====*/

	// RTが引かれていたら加速。
	const float INPUT_DEADLINE_TRI = 0.5f;
	float inputRightTriValue = Input::Ins()->isPadTri(XINPUT_TRIGGER_RIGHT);
	if (INPUT_DEADLINE_TRI < inputRightTriValue) {

		speed += inputRightTriValue * ADD_SPEED;

	}
	else {

		// 移動していなかったら移動量を0に近づける。
		speed -= speed / 10.0f;

	}

	// 右スティックの横の傾き量でキャラを回転させる。
	float inputLeftStickHori = Input::Ins()->isPadThumb(XINPUT_THUMB_LEFTSIDE);
	if (0.2f < std::fabs(inputLeftStickHori)) {

		// 回転を加算する。
		PorygonInstanceRegister::Ins()->AddRotate(carInstanceIndex, Vec3(0, HANDLE_NORMAL * inputLeftStickHori, 0));

		// 正面ベクトルを車の回転行列分回転させる。
		forwardVec = FHelper::MulRotationMatNormal(Vec3(0, 0, 1), PorygonInstanceRegister::Ins()->GetRotate(carInstanceIndex));

	}

}

void Player::Move()
{

	/*===== 移動処理 =====*/

	// 移動速度が限界を超えないようにする。
	if (MAX_SPEED < speed) {

		speed = MAX_SPEED;

	}

	// 座標移動させる。
	pos += forwardVec * speed;

}
