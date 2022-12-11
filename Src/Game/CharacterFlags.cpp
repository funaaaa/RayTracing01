#include "CharacterFlags.h"

void CharacterFlags::Init()
{

	/*===== 初期化処理 =====*/

	isShotBehind_ = false;
	onGroundPrev_ = false;
	onGround_ = false;
	onGrass_ = false;
	isConcentrationLine_ = false;
	isGetItem_ = false;
	isUseItem_ = false;
	isHitJumpActionGimmick_ = false;
	isJumpActionTrigger_ = false;
	isJumpAction_ = false;
	isAccel_ = false;
	isBeforeStartPrev_ = false;

}
