#ifndef INCLUDE_TIME_H
#define INCLUDE_TIME_H

class Timer
{
private:
	static double startTime, oldTime, currentTime, fixedTimeChange;
	static double mDeltaTime, mTime, mTimeScale, mUnscaledDeltaTime, mUnscaledTime, cycleTime;
	static int frames;
	static float count;

public:
    static float fixedTimestep;

	static void init(float);
	static void update();

    static bool nextFixedStep();
	static double deltaTime();
	static double time();
	static double unscaledDeltaTime();
	static double unscaledTime();
	static double timeScale();
	static void timeScale(double);
};

#endif