#ifndef __GAME_LOGIC_H__
#define __GAME_LOGIC_H__
#include "Framework/Interface/IRuntimeModule.h"

namespace Engine
{
	class GameLogic : public IRuntimeModule
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
	extern GameLogic* g_pGameLogic;
}
#endif // !__GAME_LOGIC_H__

