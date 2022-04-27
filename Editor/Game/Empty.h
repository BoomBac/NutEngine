#ifndef __EMPTY_H__
#define __EMPTY_H__

#include "Framework/Common/GameLogic.h"

class Empty : public Engine::GameLogic
{
public:
	int Initialize() override;
	void Finalize() override;
	void Tick() override;

	virtual void OnUpKeyDown();
	virtual void OnUpKeyUp();
	virtual void OnUpKey();

	virtual void OnDownKeyDown();
	virtual void OnDownKeyUp();
	virtual void OnDownKey();

	virtual void OnLeftKeyDown();
	virtual void OnLeftKeyUp();
	virtual void OnLeftKey();

	virtual void OnRightKeyDown();
	virtual void OnRightKeyUp();
	virtual void OnRightKey();
};

#endif // !__EMPTY_H__

