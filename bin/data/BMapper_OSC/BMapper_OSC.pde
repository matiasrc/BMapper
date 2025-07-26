import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress destino;

String[] etiquetas = {"play", "stop", "pause", "resume"};
String[] superficies = {"superficie_1", "superficie_2", "superficie_3", "superficie_4"};
int puerto = 3334;
int buttonWidth = 100;
int buttonHeight = 40;
int spacing = 20;

void setup() {
  size(500, 300);
  oscP5 = new OscP5(this, puerto);
  destino = new NetAddress("127.0.0.1", puerto);
}

void draw() {
  background(255);
  drawButtons();
}

void drawButtons() {
  for (int g = 0; g < superficies.length; g++) {
    for (int i = 0; i < etiquetas.length; i++) {
      int x = spacing + (buttonWidth + spacing) * g;
      int y = spacing + (buttonHeight + spacing) * i;
      fill(200);
      rect(x, y, buttonWidth, buttonHeight);
      fill(0);
      textAlign(CENTER, CENTER);
      text(etiquetas[i], x + buttonWidth / 2, y + buttonHeight / 2);
    }
  }
}

void mousePressed() {
  for (int g = 0; g < superficies.length; g++) {
    for (int i = 0; i < etiquetas.length; i++) {
      int x = spacing + (buttonWidth + spacing) * g;
      int y = spacing + (buttonHeight + spacing) * i;
      if (mouseX > x && mouseX < x + buttonWidth && mouseY > y && mouseY < y + buttonHeight) {
        sendOscMessage(superficies[g], etiquetas[i]);
      }
    }
  }
}

void sendOscMessage(String superficie, String etiqueta) {
  OscMessage message = new OscMessage("/" + superficie);
  message.add(etiqueta);
  oscP5.send(message, destino);
  println("Enviando mensaje OSC: " + superficie + " " + etiqueta);
}



// --- Mensajes OSC----
/*

/superficie_1 play
/etiqueta stop
/etiqueta resume
/etiqueta pause



*/
