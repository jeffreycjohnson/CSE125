#pragma once
#include "Component.h"
#include "ThreadPool.h"
#include "Texture.h"

class LoadingScreen :
	public Component
{
private:
	enum LoadingState {
		BEFORE_LOADING,
		LOADING,
		FINISHED_LOADING
	};

	ThreadPool::Job* job;
	LoadingState state;
	std::unique_ptr<Texture> splashImage;

public:
	explicit LoadingScreen(const std::string& splashScreen, const std::string& title);
	~LoadingScreen();

	// This function is passed into a separate thread (what's actually doing the loading)
	virtual void load();

	// Called when loading is finished
	virtual void finished();

	virtual void create() override;
	virtual void fixedUpdate() override;
	virtual void drawUI() override;

};