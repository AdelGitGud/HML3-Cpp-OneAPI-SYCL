#pragma once
#include "BaseManager.h"
class RenderManager :public BaseManager {
public:
	RenderManager();
	~RenderManager() override;
	bool Init() override;
	void Shutdown() override;
};

