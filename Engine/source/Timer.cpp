#include "Timer.h"
#include <iostream>
#include <glfw3.h>

using namespace std;

double Timer::startTime, Timer::oldTime, Timer::currentTime, Timer::fixedTimeChange;
double Timer::mDeltaTime, Timer::mTime, Timer::mTimeScale, Timer::mUnscaledDeltaTime, Timer::mUnscaledTime, Timer::cycleTime;
int Timer::frames;
float Timer::count;
float Timer::fixedTimestep;

void Timer::init(float count)
{
	startTime = oldTime = currentTime = glfwGetTime();
	mDeltaTime = mTime = mUnscaledDeltaTime = mUnscaledTime = cycleTime = 0.167;
	mTimeScale = 1;
	frames = 0;
    fixedTimestep = 0.02;
    fixedTimeChange = 0;
	Timer::count = count;
}

void Timer::update()
{
	oldTime = currentTime;
	currentTime = glfwGetTime();

	mDeltaTime = (float)(currentTime - oldTime) * mTimeScale;
	mTime += mDeltaTime;
	mUnscaledDeltaTime = (float)(currentTime - oldTime);
	mUnscaledTime += mUnscaledDeltaTime;

	cycleTime += mUnscaledDeltaTime;
	frames++;

    fixedTimeChange += mDeltaTime;

	if (cycleTime > count)
	{
		cout << cycleTime / frames * 1000 << " ms (" << frames / cycleTime << " fps)" << endl;
		cycleTime = 0;
		frames = 0;
	}
}

bool Timer::nextFixedStep()
{
    if(fixedTimeChange > fixedTimestep)
    {
        fixedTimeChange -= fixedTimestep;
        return true;
    }
    return false;
}

double Timer::deltaTime()
{
	return mDeltaTime;
}

double Timer::time()
{
	return mTime;
}

double Timer::unscaledDeltaTime()
{
	return mUnscaledDeltaTime;
}

double Timer::unscaledTime()
{
	return mUnscaledTime;
}

double Timer::timeScale()
{
	return mTimeScale;
}

void Timer::timeScale(double newScale)
{
	mTimeScale = newScale;
}