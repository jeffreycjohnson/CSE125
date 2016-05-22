#pragma once
#include "Component.h"
#include "Texture.h"
#include <atomic>

class LoadingScreen :
	public Component
{
private:
	enum LoadingState {
		BEFORE_LOAD,
		LOADING,
		AFTER_LOAD,
		BEFORE_FINISH,
		AFTER_FINISH
	};

	std::atomic<LoadingState> state;
	std::unique_ptr<Texture> splashImage;

public:
	explicit LoadingScreen(const std::string& splashScreen, const std::string& title);
	~LoadingScreen();

	// Called on the main thread from drawUI(), this function returns true if the gameObject's
	// component list has been modified (therefore invalidating the iterator)
	virtual bool load();

	// Called the frame after load() finishes executing, also on the main thread. Return value
	// semantics are the same as load() & drawUI().
	virtual bool finished();

	virtual void fixedUpdate() override;
	virtual bool drawUI() override;

};