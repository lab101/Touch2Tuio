#include "cinder\Vector.h"
#include <vector>
#include "cinder\Signals.h"
#include "cinder\Perlin.h"
#include "cinder\app\TouchEvent.h"


class TouchSeq{
public:
	uint32_t id = 0;
	int position = 0;
	std::vector<ci::vec2> mAutomatedTouches;

};

class RobotTouch{

	ci::Perlin		mPerlin;

	uint32_t		mLastTouchId = 0;
	std::vector<TouchSeq*>	mSequences;
	bool					mIsRunning = false;
	float					mLastInsert;
	float					mInsertDelay;

public:

	ci::signals::Signal<void(ci::app::TouchEvent)> onMove;
	ci::signals::Signal<void(ci::app::TouchEvent)> onDown;
	ci::signals::Signal<void(ci::app::TouchEvent)> onUp;

	bool isRunning();
	void resetTimer();
	void start();
	void stop();
	void setup();
	void update();
	void draw();
};