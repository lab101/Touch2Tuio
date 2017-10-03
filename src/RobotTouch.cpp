#include "RobotTouch.h"
#include "cinder\Rand.h"
#include "cinder\app\App.h"
#include "cinder\gl\gl.h"
#include <chrono>  // chrono::system_clock


using namespace ci;
using namespace std;

void RobotTouch::setup(){
	
}


void RobotTouch::start(){

	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	mPerlin.setSeed(in_time_t);
	resetTimer();
	mIsRunning = true;
}

void RobotTouch::stop(){
	mIsRunning = false;

	// convert to touches
	std::vector<ci::app::TouchEvent::Touch> touchesUp;
	for (auto seq : mSequences){
		vec2 touchPos = seq->mAutomatedTouches[seq->position];
		ci::app::TouchEvent::Touch touch(touchPos, touchPos, seq->id, seq->position / 60.0, nullptr);
		touch.setPos(touchPos);
		touchesUp.push_back(touch);
	}
	
	ci::app::TouchEvent touchUpEvent(ci::app::getWindow(), touchesUp);
	onUp.emit(touchUpEvent);
	mSequences.clear();
}

bool RobotTouch::isRunning(){
	return mIsRunning;
}


void RobotTouch::update(){

	// check if we need to add new touches.
	if (mIsRunning && mSequences.size()  < 8 && (app::getElapsedSeconds() - mLastInsert) > mInsertDelay){
		resetTimer();

		int nrOfNewTouches = ci::randInt(3, 300);
		float speed = ci::randFloat(0.001, 0.04);

		TouchSeq* seq = new TouchSeq();
		seq->id = ++mLastTouchId;
		mSequences.push_back(seq);

		for (float i = 0; i < nrOfNewTouches; ++i){
			float x = mPerlin.noise(i * speed, ci::app::getElapsedSeconds());
			float y = mPerlin.noise(-i * speed, ci::app::getElapsedSeconds());
			vec2 p(x, y);
	
			p.x *= 0.5;
			p.y *= 0.5;
			p += vec2(0.5, 0.5);


			p.x *= app::getWindowWidth();
			p.y *= app::getWindowHeight();

			seq->mAutomatedTouches.push_back(p);
		}
	}

	// update current touches

	std::vector<ci::app::TouchEvent::Touch> touchesUp;
	std::vector<ci::app::TouchEvent::Touch> touchesDown;
	std::vector<ci::app::TouchEvent::Touch> touchesMoved;

	auto it = std::begin(mSequences);
	while (it != std::end(mSequences)) {
		TouchSeq* seq = (*it);
		if ((*it)->mAutomatedTouches.size() == seq->position){
			it = mSequences.erase(it);
		}
		else{
			++it;

			vec2 touchPos = seq->mAutomatedTouches[seq->position];
			ci::app::TouchEvent::Touch touch(touchPos, touchPos, seq->id, seq->position / 60.0, nullptr);
			touch.setPos(touchPos);


			if (seq->position == 0){
				touchesDown.push_back(touch);
			}
			else if (seq->position == seq->mAutomatedTouches.size() - 1){
				touchesUp.push_back(touch);
			}
			else{
				touchesMoved.push_back(touch);
			}

			if (touchesDown.size() > 0){
				ci::app::TouchEvent event(ci::app::getWindow(), touchesDown);
				onDown.emit(event);
			}
			if (touchesMoved.size() > 0){
				ci::app::TouchEvent event(ci::app::getWindow(), touchesMoved);
				onMove.emit(event);
			}
			if (touchesUp.size() > 0){
				ci::app::TouchEvent event(ci::app::getWindow(), touchesUp);
				onUp.emit(event);
			}


			seq->position++;
		}
	}



}


void RobotTouch::draw(){

	
}

void RobotTouch::resetTimer(){
	mInsertDelay =  randFloat(0.5, 3);
	mLastInsert = app::getElapsedSeconds();
}
