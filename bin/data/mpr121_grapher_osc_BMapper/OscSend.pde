NetAddress direccionRemota;

int puerto =  3000;
String ip = "127.0.0.1";

OscBundle oscBundle;

boolean USAR_ACUMULADOR = true; //la toma del settings 
int acumularFrames = 0; //la toma del settings
int[] acumuladorXSensor = new int[numElectrodes];
boolean[] enviadoTouched = new boolean[numElectrodes];

void setupOscSend() {
  direccionRemota = new NetAddress(ip, puerto);
  oscBundle = new OscBundle();
}

void updateOscSend() {

  //Agregado Matias 09/2021
  OscMessage touch = new OscMessage("/touch");
  OscMessage tths = new OscMessage("/tths");
  OscMessage rths = new OscMessage("/rths");
  OscMessage fdat = new OscMessage("/fdat");
  OscMessage bval = new OscMessage("/bval");
  OscMessage diff = new OscMessage("/diff");

  for (int i = 0; i < numElectrodes; i++) {

    tths.add(touchThresholds[i]);
    rths.add(releaseThresholds[i]);
    fdat.add(filteredData[i]);
    bval.add(baselineVals[i]);
    diff.add(diffs[i]);

    //modificado por David para enviar luego de acumular:
    if ( USAR_ACUMULADOR ) {
      if ( status[i] != 0x00 && !enviadoTouched[i] ) {
        acumuladorXSensor[i]++;
        //println( acumuladorXSensor[i]);
        if ( acumuladorXSensor[i]>acumularFrames) {
          enviadoTouched[i] = true;
          sendTouched(i); //Agregado Matias 09/2021
        }
      }
      if (status[i] == 0x00) {
        acumuladorXSensor[i]=0; //David
        enviadoTouched[i]=false;//David
        sendReleased(i); //Agregado Matias 09/2021
      }

      if (  acumuladorXSensor[i]> acumularFrames) {
        touch.add(1);
      } else {
        touch.add(0);
      }
      
    } else { //end usar acumulador ////////////
    
      if (lastStatus[i] == 0 && status[i] != 0x00) {
        // touched
        sendTouched(i); //Agregado Matias 09/2021
      } else if (lastStatus[i] != 0x00 && status[i] == 0x00) {
        // released

        sendReleased(i); //Agregado Matias 09/2021
      }

      //tomo el estado, pero si no uso acumuldor
      if (status[i] == 0) {
        touch.add(0);//Agregado Matias 09/2021
      } else {
        touch.add(1);//Agregado Matias 09/2021
      }
    }
  }

  oscBundle.add(touch); 
  oscBundle.add(tths); 
  oscBundle.add(rths); 
  oscBundle.add(fdat); 
  oscBundle.add(bval); 
  oscP5.send(oscBundle, direccionRemota); //Se envia el mensaje
  oscBundle.clear();
  //Fin Agregado Matias 09/2021
}

void sendTouched(int sensor) {
  OscMessage mensaje = new OscMessage("/touched"); //crea una etiqueta para el mensaje
  mensaje.add(sensor); // se le agrega un dato

  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
  
  // ----Agregado Matias 24.07.2025
  mensaje = new OscMessage("/sensor_" + sensor); //crea una etiqueta para el mensaje
  mensaje.add("play"); // se le agrega un dato

  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
  println("-------TOUCHED: " + mensaje);
}

void sendReleased(int sensor) {
  OscMessage mensaje = new OscMessage("/released"); //crea una etiqueta para el mensaje
  mensaje.add(sensor); // se le agrega un dato

  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
  
  // ----Agregado Matias 24.07.2025
  mensaje = new OscMessage("/sensor_" + sensor); //crea una etiqueta para el mensaje
  mensaje.add("stop"); // se le agrega un dato
  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
  println("------RELEASED: " + mensaje);
}
