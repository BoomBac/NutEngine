#include "Empty.h"
#include "Framework/Common/Log.h"

int Empty::Initialize()
{
	return 0;
}

void Empty::Finalize()
{
}

void Empty::Tick()
{
}

void Empty::OnUpKeyDown()
{
}

void Empty::OnUpKeyUp()
{
}

void Empty::OnUpKey()
{
}

void Empty::OnDownKeyDown()
{
}

void Empty::OnDownKeyUp()
{
}

void Empty::OnDownKey()
{
}

void Empty::OnLeftKeyDown()
{
	NE_LOG(Engine::ALL,Engine::kNormal,"Game Empty LeftKeyDown")
}

void Empty::OnLeftKeyUp()
{
}

void Empty::OnLeftKey()
{
}

void Empty::OnRightKeyDown()
{
}

void Empty::OnRightKeyUp()
{
}

void Empty::OnRightKey()
{
}
