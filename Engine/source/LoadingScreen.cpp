#include "LoadingScreen.h"
#include "Renderer.h"
#include "Crosshair.h"

LoadingScreen::LoadingScreen(const std::string& splashScreen, const std::string& title) : state(BEFORE_LOAD)
{
	splashImage = std::make_unique<Texture>(splashScreen, false);
}

LoadingScreen::~LoadingScreen()
{
}

bool LoadingScreen::load()
{
	// Override me :D
	return false;
}

bool LoadingScreen::finished()
{
	// Override me :D
	return false;
}

void LoadingScreen::fixedUpdate()
{
	// Be wary of adding anything else here, as you may cause a data race
	if (state == AFTER_FINISH) {
		this->active = false;
	}
}

bool LoadingScreen::drawUI()
{
	// this is called on the main thread trolololool

	if (state != AFTER_FINISH) {
		Renderer::drawSplash(splashImage.get(), true);
		//Renderer::drawSpriteStretched(glm::vec2(0), glm::vec2(Renderer::getWindowWidth(), Renderer::getWindowHeight()), glm::vec4(0), splashImage.get()); // equivalent, just used for testing
	}

	if (state == BEFORE_LOAD) {
		state = LOADING;
		Renderer::crosshair->hide();
	}
	else if (state == LOADING) {
		bool validity = load();
		state = AFTER_LOAD;
		return validity; // Iterator may have been invalidated
	}
	else if (state == AFTER_LOAD) {
		bool validity = finished();
		state = AFTER_FINISH;
		return validity;
	}

	return false;

}
