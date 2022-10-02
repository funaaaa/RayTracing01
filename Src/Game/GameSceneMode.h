#pragma once
#include "Singleton.h"

class GameSceneMode : public Singleton<GameSceneMode> {

	/*===== メンバ変数 =====*/

public:

	enum class MODE {

		DEF,
		AI,
		WRITE_GHOST,
		GHOST,

	};

	MODE mode_;
	int level_;

	GameSceneMode() {
		mode_ = MODE::AI;
		level_;
	}

};