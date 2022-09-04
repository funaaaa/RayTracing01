#pragma once
#include "Singleton.h"

class GameSceneMode : public Singleton<GameSceneMode> {

	/*===== メンバ変数 =====*/

public:

	enum class MODE_ID {

		DEF,
		AI,
		WRITE_GHOST,
		GHOST,

	};

	MODE_ID id_;
	int ghostLevel_;

	GameSceneMode() {
		id_ = MODE_ID::GHOST;
		ghostLevel_;
	}

};