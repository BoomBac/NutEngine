#include "../Inc/QTApplication.h"




QTApplication::QTApplication(GfxConfiguration& cfg) : BaseApplication(cfg)
{
}

int QTApplication::Initialize()
{
	int ret = 0;
	pp_argv_ = new char*[1];
	pp_argv_[0] = "NutEngine";
	argc_ = 0;
	p_app_ = new QApplication(argc_, pp_argv_);
	p_editor_ = new Editor();
	p_editor_->show();
	p_editor_->setWindowTitle(config_.window_name_);
	handle_ = dynamic_cast<Editor*>(p_editor_)->GetViewportHandle();
	return 0;
}

void QTApplication::Finalize()
{

}

QTApplication::~QTApplication()
{
	delete[] pp_argv_;
	delete p_app_;
	delete p_editor_;
}

void QTApplication::Tick()
{
	p_app_->processEvents();
}

void* QTApplication::GetMainWindowHandler()
{
	return static_cast<void*>(handle_);
}

QMainWindow* QTApplication::GetMainWindow()
{
	return p_editor_;
}

QApplication* QTApplication::GetApp()
{
	return p_app_;
}
