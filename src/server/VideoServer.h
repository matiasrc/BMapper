#pragma once

#include "ofMain.h"
#include "FboSource.h"

#if defined(TARGET_OSX)
#include "ofxSyphon.h"

#elif defined(TARGET_WIN32)
#include "ofxSpout.h"

#else
    // Código para otros sistemas operativos
    ofLog() << "Running on another OS";
#endif


 class VideoServer : public ofx::piMapper::FboSource {
 	public:
        void setup();
 		void update();
 		void draw();

		void keyReleased(int key);

        #if defined(TARGET_OSX)
		bool selectServer(int index);
		std::vector<std::string> getServerLabels();
		int getSelectedServerIndex() const;
		void serverAnnounced(ofxSyphonServerDirectoryEventArgs &arg);
 		void serverUpdated(ofxSyphonServerDirectoryEventArgs &args);
 		void serverRetired(ofxSyphonServerDirectoryEventArgs &arg);

 		ofxSyphonServerDirectory dir;
 		ofxSyphonClient mClient;
 		int dirIdx;
     
        #elif defined(TARGET_WIN32)
        // La interfaz no accede al receptor directamente: estas funciones
        // mantienen a Spout encapsulado en la fuente de video multiplataforma.
        bool selectSpoutSender();
        std::string getSpoutSenderName() const;
        ofxSpout::Receiver receiver;
        ofTexture texture;
        #endif
 };
