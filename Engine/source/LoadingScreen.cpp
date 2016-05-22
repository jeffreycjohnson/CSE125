#include "LoadingScreen.h"
#include "Renderer.h"

LoadingScreen::LoadingScreen(const std::string& splashScreen, const std::string& title) : job(nullptr), state(BEFORE_LOADING)
{
	splashImage = std::make_unique<Texture>(splashScreen, false);
}

LoadingScreen::~LoadingScreen()
{
}

void LoadingScreen::load()
{
	// Override me :D
}

void LoadingScreen::finished()
{
	// Override me :D
}

void LoadingScreen::create()
{
	// Since load() is a virtual function, the std::bind here will bind to the overridden
	// versions of load in any and all subclasses of LoadingScreen.
	auto jobFuncPtr = std::bind(&LoadingScreen::load, this);
	job = workerPool->createJob(jobFuncPtr)->queue();
}

void LoadingScreen::fixedUpdate()
{
	if (job != nullptr) {
		bool complete = workerPool->completed(job);
	}

	switch (state) {
	case BEFORE_LOADING:
		{
			if (job == nullptr) return; // Loading hasn't started
			if (!workerPool->completed(job)) {
				state = LOADING;
			}
		}
	case LOADING:
		{
			if (workerPool->completed(job)) {
				state = FINISHED_LOADING;
				finished();
			}
		}
		break;
	case FINISHED_LOADING:
		{
			this->active = false; // TODO: Destroy the object
		}
		break;
	}
}

void LoadingScreen::drawUI()
{
	if (state != FINISHED_LOADING) {
		Renderer::drawSplash(splashImage.get(), true);
	}
}
