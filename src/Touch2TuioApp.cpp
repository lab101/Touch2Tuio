#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "TuioServer.h"
#include "TuioCursor.h"
#include "TcpSender.h"


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
	
	mFont = Font("Arial", 30);

	host = "127.0.0.1";
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

	//tuioServer->enableFullUpdate();

}


void Touch2TuioApp::touchesBegan(TouchEvent event){

	for (auto& t : event.getTouches()){
		int id = t.getId();
		CursorInfo info(t.getPos(),true);
		touches[id] = info;
	}
}

void Touch2TuioApp::touchesEnded(TouchEvent event){


	for (auto& t : event.getTouches()){

		int id = t.getId();
		if (touches.find(id) != touches.end()){

			touches[id].position = t.getPos();
			touches[id].isAlive = false;

		}
	}
}


void Touch2TuioApp::touchesMoved(TouchEvent event){

	for (auto& t : event.getTouches()){

		int id = t.getId();
		if (touches.find(id) != touches.end()){

			touches[id].position = t.getPos();
			touches[id].isAlive = true;
		}
	}
}


void Touch2TuioApp::mouseDrag(MouseEvent event){


}


void Touch2TuioApp::mouseUp(MouseEvent event)
{

	
}

void Touch2TuioApp::mouseDown( MouseEvent event )
{

}

void Touch2TuioApp::keyDown(KeyEvent event)
{

	if (event.getCode() == event.KEY_ESCAPE){
		quit();
	}
	else if (event.getCode() == event.KEY_f){
		setFullScreen(!isFullScreen());
	}
}





void Touch2TuioApp::update()
{
	frameTime = TuioTime::getSessionTime();
	tuioServer->initFrame(frameTime);


	std::map<int, CursorInfo>::iterator itr = touches.begin();
	while (itr != touches.end()) {

		if (itr->second.isAlive && itr->second.cursor == nullptr){
			itr->second.cursor = tuioServer->addTuioCursor(itr->second.position.x / getWindowWidth(), itr->second.position.y / getWindowHeight());
			++itr;
		}
		else if (itr->second.isAlive){
			tuioServer->updateTuioCursor(itr->second.cursor, itr->second.position.x / getWindowWidth(), itr->second.position.y / getWindowHeight());
			++itr;
		}
		else {
			itr->second.isAlive = false;
			tuioServer->removeTuioCursor(itr->second.cursor);
			itr = touches.erase(itr);
		}
	}

	tuioServer->stopUntouchedMovingCursors();
	tuioServer->commitFrame();
}

void Touch2TuioApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
	gl::drawString("TUIO Sender: " + host + ":" +  toString(port), vec2(20, 20), Color(1, 1, 1), mFont);
	gl::drawString("active touches: " + toString(touches.size()), vec2(20, 50), Color(1, 1, 1), mFont);

	for (auto& t : touches){
		ci::gl::drawSolidCircle(t.second.position, 20, 12);
	}
}

CINDER_APP(Touch2TuioApp, RendererGl, prepareSettings)
