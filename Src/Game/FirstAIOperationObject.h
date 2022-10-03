#pragma once
#include "BaseOperationObject.h"
#include <memory>

class FirstAIWayPointMgr;

class FirstAIOperationObject : public BaseOperationObject {

private:

	/*===== メンバ変数 =====*/

	std::shared_ptr<FirstAIWayPointMgr> waypointMgr_;
	int level_;


public:

	/*===== メンバ関数 =====*/

	FirstAIOperationObject(const int& WayPointOffset, const int& Level);

	BaseOperationObject::Operation Input(const BaseOperationObject::OperationInputData& InputData)override;

	void OnGoal()override;


};