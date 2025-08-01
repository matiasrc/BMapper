 #include "VideoServer.h"

 void VideoServer::setup(){
 	// Give our source a decent name
 	name = "Video server";

 	// Allocate our FBO source, decide how big it should be
 	allocate(500, 500);

    #if defined(TARGET_OSX)
 	//setup our directory
 	dir.setup();
 	//setup our client
 	mClient.setup();

 	//register for our directory's callbacks
 	ofAddListener(dir.events.serverAnnounced, this, &VideoServer::serverAnnounced);
 	// not yet implemented
 	//ofAddListener(dir.events.serverUpdated, this, &ofApp::serverUpdated);
 	ofAddListener(dir.events.serverRetired, this, &VideoServer::serverRetired);

 	dirIdx = -1;
     
    #elif defined(TARGET_WIN32)
    receiver.init();
    #endif
 }

#if defined(TARGET_OSX)
 //these are our directory's callbacks
 void VideoServer::serverAnnounced(ofxSyphonServerDirectoryEventArgs &arg)
 {
    for( auto& dir : arg.servers ){
 			ofLogNotice("ofxSyphonServerDirectory Server Announced")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
 	}
 	dirIdx = 0;
 }

 void VideoServer::serverUpdated(ofxSyphonServerDirectoryEventArgs &arg)
 {
 	for( auto& dir : arg.servers ){
 			ofLogNotice("ofxSyphonServerDirectory Server Updated")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
 	}
 	dirIdx = 0;
 }

 void VideoServer::serverRetired(ofxSyphonServerDirectoryEventArgs &arg)
 {
    for( auto& dir : arg.servers ){
 			ofLogNotice("ofxSyphonServerDirectory Server Retired")<<" Server Name: "<<dir.serverName <<" | App Name: "<<dir.appName;
 	}
 	dirIdx = 0;
 }

#endif

 // Don't do any drawing here
void VideoServer::update(){
#if defined(TARGET_OSX)
    //esto deberia andar anda'
    
     if (mClient.isSetup()) {
         if ((getWidth() != mClient.getWidth() || getHeight() != mClient.getHeight()) && mClient.getWidth() > 0 && mClient.getHeight() > 0) {
             allocate(mClient.getWidth(), mClient.getHeight());
         }
     }
    
    
#elif defined(TARGET_WIN32)
     if (texture.isAllocated()) {
         if (getWidth() != texture.getWidth() || getHeight() != texture.getHeight()) {
             allocate(texture.getWidth(), texture.getHeight());
         }
     }
    receiver.receive(texture);
 #endif
 }

 // No need to take care of fbo.begin() and fbo.end() here.
 // All within draw() is being rendered into fbo;
 void VideoServer::draw(){
    #if defined(TARGET_OSX)
 	if(dir.isValidIndex(dirIdx))
        mClient.draw(0, 0);
     
    #elif defined(TARGET_WIN32)
    texture.draw(0, 0);
    #endif
 }

 //--------------------------------------------------------------
 void VideoServer::keyReleased(int key){
    #if defined(TARGET_OSX)
     //press any key to move through all available Syphon servers
     if (dir.size() > 0)
     {
         dirIdx++;
         if(dirIdx > dir.size() - 1)
             dirIdx = 0;

         mClient.set(dir.getDescription(dirIdx));
         string serverName = mClient.getServerName();
         string appName = mClient.getApplicationName();

         if(serverName == ""){
             serverName = "null";
         }
         if(appName == ""){
             appName = "null";
         }
         ofSetWindowTitle(serverName + ":" + appName);
     }
     else
     {
         ofSetWindowTitle("No Server");
     }
    #endif
 }
