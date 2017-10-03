#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "TuioServer.h"
#include "TuioCursor.h"
#include "TcpSender.h"

#include "RobotTouch.h"

#include "cinder/Utilities.h"

#include "osc/OscTypes.h"
#include <list>
#include <math.h>
#include <map>

using namespace ci;
using namespace ci::app;
using namespace std;

using namespace TUIO;


class CursorInfo {

public:
	vec2 position;
	bool isAlive;	
	bool isRemoved = false;
	int shadowCount = 0;	
	int frameCount = 0;

	TuioCursor* cursor = nullptr;

	CursorInfo(vec2 newPosition, bool newAliveState) {
		isAlive = newAliveState;
		position = newPosition;
	}

	CursorInfo() {
		isAlive = false;
	}
};

class Touch2TuioApp : public App {

 TuioServer *tuioServer;
 TuioTime frameTime;
 std::string host;
 int port;


 RobotTouch robot;


 std::map<int, CursorInfo> touches;
 ci::Font mFont;

  public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void mouseUp(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;

	void keyDown(KeyEvent event) override;


	void touchesBegan(TouchEvent event) override;
	void touchesMoved(TouchEvent event) override;
	void touchesEnded(TouchEvent event) override;

	void update() override;
	void draw() override;
};



void prepareSettings(Touch2TuioApp::Settings *settings)
{
	settings->setMultiTouchEnabled(true);
}

void Touch2TuioApp::setup()
{
	TuioTime::initSession();
	frameTime = TuioTime::getSessionTime();
	
	mFont = Font("Consolas", 30);

	host = "172.18.39.87";
	port = 3333;
	
	auto args = getCommandLineArgs();
	if (args.size() > 1){
		host = args[1];
	}
	if (args.size() > 2){
		port = stoi(args[2]);
	}

	if (args.size() > 3){
		setFullScreen(true);
	}
	console() << "connecting to " << host << ":" << port << std::endl;
	
	try{

		tuioServer = new TuioServer(host.c_str(), port);
		OscSender *tcp_sender;

		tcp_sender = new TcpSender(host.c_str(), port);
		tuioServer->addOscSender(tcp_sender);

		tuioServer->setSourceName("touchSender");
		tuioServer->enableObjectProfile(false);
		tuioServer->enableBlobProfile(false);

	}
	catch (Exception ex){
		console() << "error occored at setup " << ex.what() << std::endl;
	}


	// fake random touches for stresstesting.
	robot.setup();
	robot.onDown.connect(std::bind(&Touch2TuioApp::touchesBegan, this, std::placeholders::_1));
	robot.onMove.connect(std::bind(&Touch2TuioApp::touchesMoved, this, std::placeholders::_1));
	robot.onUp.connect(std::bind(&Touch2TuioApp::touchesEnded, this, std::placeholders::_1));

	//tuioServer->enableFullUpdate();

}


void Touch2TuioApp::touchesBegan(TouchEvent event){

	for (auto& t : event.getTouches()){
		int id = t.getId();
		CursorInfo info(t.getPos(),true);
		touches[id] = info;
		touches[id].frameCount=1;

	}
}

void Touch2TuioApp::touchesEnded(TouchEvent event){

	for (auto& t : event.getTouches()){

		int id = t.getId();
		if (touches.find(id) != touches.end()){

			touches[id].position = t.getPos();
			touches[id].isAlive = false;
			touches[id].shadowCount = 40;
			touches[id].frameCount++;
		}
	}
}


void Touch2TuioApp::touchesMoved(TouchEvent event){

	for (auto& t : event.getTouches()){

		int id = t.getId();
		if (touches.find(id) != touches.end()){
			touches[id].position = t.getPos();
			touches[id].frameCount++;
		}
	}
}


void Touch2TuioApp::mouseDrag(MouseEvent event){
	if (touches.find(-1) != touches.end()){
		touches[-1].position = event.getPos();
		touches[-1].frameCount++;

	}
}


void Touch2TuioApp::mouseUp(MouseEvent event)
{
	if (touches.find(-1) != touches.end()){

		touches[-1].position = event.getPos();
		touches[-1].isAlive = false;
		touches[-1].shadowCount = 40;
	}
}

void Touch2TuioApp::mouseDown( MouseEvent event )
{
	CursorInfo info(event.getPos(), true);
	touches[-1] = info;
	touches[-1].frameCount = 1;
}



void Touch2TuioApp::keyDown(KeyEvent event)
{

	if (event.getCode() == event.KEY_ESCAPE){
		quit();
	}
	else if (event.getCode() == event.KEY_f){
		setFullScreen(!isFullScreen());
	}
	// toggle automated random touches for testing
	else if (event.getCode() == event.KEY_r){
		if (robot.isRunning()){
			robot.stop();
		}
		else{
			robot.start();
		}
	}
}





void Touch2TuioApp::update()
{
	robot.update();

	frameTime = TuioTime::getSessionTime();
	tuioServer->initFrame(frameTime);

	std::map<int, CursorInfo>::iterator itr = touches.begin();
	while (itr != touches.end()) {


		if (itr->second.isAlive){
			// send add tuio cursor
			if (itr->second.cursor == nullptr){
				itr->second.cursor = tuioServer->addTuioCursor(itr->second.position.x / getWindowWidth(), itr->second.position.y / getWindowHeight());
				//++itr;
			}
			else{
				// send tuio moving
				tuioServer->updateTuioCursor(itr->second.cursor, itr->second.position.x / getWindowWidth(), itr->second.position.y / getWindowHeight());

			}

			++itr;
		}
		else{

			// send tuio remove
			if (!itr->second.isRemoved){
				tuioServer->removeTuioCursor(itr->second.cursor);
				itr->second.isRemoved = true;
			}

			// keep the the touch item when shadowing is enabled
			if (itr->second.shadowCount > 0){
				itr->second.shadowCount--;
				++itr;
			}
			else{
				// when no shadow count erase it completely
				itr = touches.erase(itr);
			}


		}
	}

	tuioServer->stopUntouchedMovingCursors();
	tuioServer->commitFrame();
}

void Touch2TuioApp::draw()
{
	gl::clear( Color( 0, 0.3, 0.6 ) ); 


	gl::lineWidth(2);
	gl::color(0., 0.7, 0.7,0.3);
	for (float x = -100; x < getWindowWidth(); x += 200){
		gl::drawLine(vec2(x, 0), vec2(x, getWindowHeight()));
	}

	for (int y = -100; y < getWindowHeight(); y += 200){
		gl::drawLine(vec2(0.0, y), vec2(getWindowWidth(), y));

	}

	gl::lineWidth(0.1);
	gl::color(0.5, 1, 1,0.1);
	for (float x = 0; x < getWindowWidth(); x += 20){
		gl::drawLine(vec2(x, 100.0), vec2(x, getWindowHeight()));
	}

	for (int y = 0; y < getWindowHeight(); y += 20){
		gl::drawLine(vec2(0.0, y), vec2(getWindowWidth(), y));
	}


	for (auto& t : touches){
		if (t.second.isAlive){

			if (t.second.frameCount < 1){
				gl::color(0, 1, 0);
				ci::gl::drawSolidCircle(t.second.position, 50, 60);
			}
			else{
				gl::color(1, 1, 1);
				ci::gl::drawSolidCircle(t.second.position, 40, 60);

			}

		}
		else{
			//DEAD
			gl::color(1, 0, 0,0.7);
			ci::gl::drawSolidCircle(t.second.position, 30, 60);


		}
	}

	gl::color(1, 1, 1);

	gl::drawString("TUIO Sender: " + host + ":" + toString(port), vec2(20, 20), Color(1, 1, 1), mFont);
	gl::drawString("active touches: " + toString(touches.size()), vec2(20, 50), Color(1, 1, 1), mFont);
	std::string robotState = (robot.isRunning() ? "ON" : "OFF");
	gl::drawString("robot touch: " + robotState , vec2(400, 20), Color(1, 1, 1), mFont);

	robot.draw();
}

CINDER_APP(Touch2TuioApp, RendererGl, prepareSettings)
