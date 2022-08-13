#pragma once
#include "Vec.h"
#include "ConstBuffers.h"
#include <vector>
#include "PlayerModel.h"
#include <memory>

class BaseStage;
class OBB;
class PlayerTire;
class BaseItem;
class BaseOperationObject;
class BaseStage;

class Character {

private:

	/*===== �����o�ϐ� =====*/
public:

	Vec3 pos_;				// �Ԃ̍��W
	Vec3 prevPos_;			// �Ԃ̑O�t���[���̍��W
	Vec3 forwardVec_;		// �i�s�����x�N�g��
	Vec3 bottomVec;			// �������x�N�g��
	Vec3 upVec_;			// ������x�N�g��
	Vec3 size_;				// �T�C�Y
	float speed_;			// �ړ����x
	float gravity_;			// �d��
	float rotY_;			// �n���h������ɂ���ĕς��Y���̉�]��
	float shellHitRot_;		// �b���ɓ��������Ƃ��̉�]�B
	int returnDefPosTimer_;	// �f�t�H���g�̈ʒu�ɖ߂�܂ł̎��� �ޗ��ɗ��������p
	int canNotMoveTimer_;	// ����s�\�̃^�C�}�[
	const int CAN_NOT_MOVE_TIMER_SHELL_HIT = 60;
	const int RETURN_DEFPOS_TIMER = 600;
	bool isShotBehind_;		// ��둤�ɍb���𓊂��邩�̃t���O
	bool onGround_;			// �n��ɂ��邩 t=�n�� f=��
	bool onGrass_;			// ���̏�ɂ��邩 t=���̏� f=���̏ザ��Ȃ�

	std::shared_ptr<OBB> obb_;	// �����蔻��pOBB

	std::vector<std::shared_ptr<PlayerTire>> tires_;

	bool isGetItem_;	// �A�C�e�����擾�����t���[���̔���

	// ����I�u�W�F�N�g
	std::shared_ptr<BaseOperationObject> operationObject_;

	// �A�C�e���N���X
	std::shared_ptr<BaseItem> item_;

	bool IsTurningIndicatorRed_;// �E�C���J�[�̐F���Ԃ��ǂ����B
	int turningIndicatorTimer_;	// �E�C���J�[���`�J�`�J����^�C�}�[
	const int TURNING_INDICATOR_TIMER = 30;

	const float MAX_SPEED = 16.0f;		// �ړ����x�̍ő�l
	const float MAX_SPEED_ON_GRASS = 8.0f;// ���̏�ɂ���Ƃ��̍ő呬�x
	const float ADD_SPEED = 2.0f;		// �ړ����x�̉��Z��
	const float HANDLE_NORMAL = 0.03f;	// �ʏ펞�̃n���h�����O�̊p�x
	const float MAX_GRAV = 7.0f;		// �d�͂̍ő��
	const float ADD_GRAV = 0.05f;		// �d�͂̉��Z��
	const Vec3 PLAYER_DEF_POS = Vec3(0, 30, -30);


	/*-- ���f���̃f�[�^�Ɋւ���ϐ� --*/

	PlayerModel playerModel_;


	/*-- �h���t�g�Ɋւ���ϐ� --*/

	float boostSpeed_;					// �u�[�X�g����Ă���Ƃ��̈ړ����x
	int driftBoostTimer_;				// �h���t�g�Ńu�[�X�g����܂ł̃^�C�}�[
	int boostTimer_;					// �u�[�X�g����t���[����
	bool isDrift_;						// �h���t�g��Ԃ��ǂ����B
	bool isTireMask_;

public:

	struct TireUVSet {
		Vec2 uv_;
		Vec2 prevuv_;
	};

	// �^�C�����������ݗp
	struct TireMaskUV {
		TireUVSet forwardLeftUV_;
		TireUVSet forwardRightUV_;
		TireUVSet behindLeftUV_;
		TireUVSet behindRightUV_;
	};

private:

	TireMaskUV tireMaskUV_;				// �^�C�������o���ۂɎd�l

	const float HANDLE_DRIFT = 0.06f;	// �h���t�g���̃n���h�����O�̊p�x
	const float MAX_BOOST_SPEED = 20.0f;// �u�[�X�g�̈ړ��ʂ̍ő�l
	const float SUB_BOOST_SPEED = 0.2f;	// �u�[�X�g�̈ړ��ʂ̌��c��
	const int DRIFT_BOOST_TIMER = 30;	// �h���t�g�Ńu�[�X�g����܂ł̃^�C�}�[


public:

	enum class CHARA_ID {

		P1,	// �v���C���[1
		P1_WGHOST,	// �v���C���[1�S�[�X�g�������ݗL��
		AI1,	// AI1
		GHOST,	// �S�[�X�g

	};


private:

	CHARA_ID charaID_;


public:

	/*===== �����o�֐� =====*/

	// ����������
	Character(CHARA_ID CharaID);
	void Init();

	// �X�V����
	void Update(std::weak_ptr<BaseStage> StageData, RayConstBufferData& ConstBufferData, bool& IsPassedMiddlePoint, int& RapCount);

	// �`�揈��
	void Draw();

	// �^�C�������o
	bool CheckTireMask(std::weak_ptr<BaseStage> BaseStageData, TireMaskUV& TireMaskUVData);


	const Vec3& GetPos() { return pos_; }
	const Vec3& GetForwardVec() { return forwardVec_; }
	Vec3 GetUpVec() { return upVec_; };
	float GetNowSpeedPer();
	bool GetIsGetItem() { return isGetItem_; }

private:

	// ���͏���
	void Input(RayConstBufferData& ConstBufferData);

	// �ړ�����
	void Move();

	// �����蔻��
	void CheckHit(std::weak_ptr<BaseStage> StageData, bool& IsPassedMiddlePoint, int& RapCount);

	// �΂ߏ��̉�]
	void RotObliqueFloor(const Vec3& HitNormal);

};