#include "BasicStageObject.h"
#include "PolygonInstanceRegister.h"
#include "OBB.h"

void BasicStageObject::Setting(const BaseStageObject::OBJECT_ID& ObjectID, const BaseStageObject::COLLISION_ID& CollisionID, std::weak_ptr<PolygonMeshInstance> Instance)
{

	/*===== セッティング処理 =====*/

	BasicInit(ObjectID, CollisionID, Instance);

}

void BasicStageObject::Destroy()
{

	/*===== インスタンス破棄 =====*/

	PolygonInstanceRegister::Ins()->DestroyInstance(instance_);
	isActive_ = false;

}

void BasicStageObject::Update(int Timer, std::weak_ptr<CharacterMgr> Character)
{

	/*===== 更新処理 =====*/

	Timer;
	Character;

}

void BasicStageObject::Disable(int TimerToActivation, int CharaIndex)
{

	/*===== 無効化して再設定までの時間をセット =====*/

	TimerToActivation;
	CharaIndex;

}
