#include "BoostItem.h"

BoostItem::BoostItem()
{
	itemID_ = ItemID::NONE;
}

void BoostItem::Generate(std::weak_ptr<PolygonMeshInstance> CharaInstance)
{

	/*===== 生成処理 =====*/

	itemID_ = BaseItem::ItemID::BOOST;
	charaInstance = CharaInstance;

}

void BoostItem::Update()
{

	/*===== 更新処理 =====*/

	// 更新処理は何も行わない。

}

int BoostItem::Use(float CharaRotY, int ParamID)
{

	/*===== 使用処理 =====*/

	// きのこアイテム自体では使用されたときは何もしない。
	CharaRotY;
	ParamID;

	return 0;

}
