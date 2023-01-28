#pragma once
class BaseManager {
public:
	virtual bool Init() = 0;
	virtual void Shutdown() = 0;
};