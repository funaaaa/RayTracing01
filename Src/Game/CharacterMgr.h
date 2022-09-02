#pragma once
#include <memory>
#include <vector>
#include "Character.h"
#include "ConstBuffers.h"

class Character;
class BaseStage;

class CharacterMgr {

private:

	/*===== メンバ変数 =====*/

	std::vector<std::shared_ptr<Character>> character_;
	int playerIndex_;


public:

	/*===== メンバ関数 =====*/

	CharacterMgr();
	void Init();
	void Update(std::weak_ptr<BaseStage> Stage, RayConstBufferData& ConstBufferData, int& RapCount, bool& IsPassedMiddlePoint, const bool& IsBeforeStart, const bool& IsGameFinish);
	void Draw();

	void AddChara(const int& CharaID, const bool& IsPlayer);
	bool CheckTireMask(std::weak_ptr<BaseStage> BaseStageData, std::vector<Character::TireMaskUV>& TireMaskUVData);
	inline std::weak_ptr<Character> GetPlayerIns() { return character_[playerIndex_]; }

};