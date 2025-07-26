import processing.core.*; 
import processing.data.*; 
import processing.event.*; 
import processing.opengl.*; 

import processing.serial.*; 
import controlP5.*; 
import oscP5.*; 
import netP5.*; 
import processing.awt.PSurfaceAWT.SmoothCanvas; 
import javax.swing.JFrame; 
import java.awt.Dimension; 
import java.awt.Point; 

import java.util.HashMap; 
import java.util.ArrayList; 
import java.io.File; 
import java.io.BufferedReader; 
import java.io.PrintWriter; 
import java.io.InputStream; 
import java.io.OutputStream; 
import java.io.IOException; 

public class mpr121_grapher_osc extends PApplet {

/*******************************************************************************
 
 Bare Conductive MPR121 output grapher / debug plotter for TouchBoard and Pi Cap
 -------------------------------------------------------------------------------
 
 mpr121_grapher.pde - processing grapher for raw data from TouchBoard and Pi Cap
 
 Requires Processing 3.0+
 
 Requires controlp5 (version 2.2.5+) to be in your processing libraries folder:
 http://www.sojamo.de/libraries/controlP5/
 
 Requires osc5 (version 0.9.8+) to be in your processing libraries folder:
 http://www.sojamo.de/libraries/oscP5/
 
 If connecting via Serial Data requires datastream on the Touch Board:
 https://github.com/BareConductive/mpr121/tree/public/Examples/DataStream
 
 If connecting via OSC requires picap-datastream-osc on the Pi Cap
 
 Bare Conductive code written by Stefan Dzisiewski-Smith and Szymon Kaliski.
 
 This work is licensed under a MIT license https://opensource.org/licenses/MIT
 
 Copyright (c) 2016, Bare Conductive
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 *******************************************************************************/











JFrame displayFrame;
SmoothCanvas smoothCanvas;

//por David Bedoian 9/2021. Para seleccionar placa por defecto
//trata de seleccionar del settings.json
//la primera vez va a preguntar, hasta poder guardarla para la próxima:
String serialPortNameSelected;


public class Line {
  public float left;
  public float right;
  public float top;
  public float bottom;
}


final int baudRate = 57600;

final int numElectrodes  = 12;
final int numGraphPoints = 300;
final int tenBits        = 1024;

final int graphsLeft           = 20;
final int graphsTop            = 50;
final int graphsWidth          = 984;
final int graphsHeight         = 510;
final int numVerticalDivisions = 8;

final int filteredColour   = color(255, 0, 0, 200);
final int baselineColour   = color(0, 0, 255, 200);
final int touchedColour    = color(255, 128, 0, 200);
final int releasedColour   = color(0, 128, 128, 200);
final int textColour       = color(60);
final int touchColourBar   = color(255, 255, 255, 200);
final int touchColourGraph = color(255, 0, 255, 200);
final int releaseColour    = color(255, 255, 255, 200);

final int graphFooterLeft = 20;
final int graphFooterTop  = graphsTop + graphsHeight + 20;

final int numFooterLabels = 6;

boolean serialSelected = false;
boolean oscSelected    = false;
boolean firstRead      = true;
boolean secondRead     = false;
boolean paused         = false;
boolean helpVisible    = false;
boolean cursorVisible  = false;
boolean isResetting    = false;

int resetStartTime;
final int RESET_TIMEOUT = 5000;

String mode = "BARS"; // "GRAPHS", "DARK", "BARS"

ControlP5 cp5;
ScrollableList electrodeSelector, serialSelector;
Textlabel labels[], startPrompt, instructions, pausedIndicator, cursorLabel;
Button oscButton, helpButton, resetButton;

OscP5 oscP5;

Serial inPort;        // the serial port
String[] validSerialList;
String inString;      // input string from serial port
String[] splitString; // input string array after splitting
int lf = 10;          // ASCII linefeed

int[] filteredData, baselineVals, diffs, touchThresholds, releaseThresholds, fakeTouchThresholds, fakeReleaseThresholds, lastFakeTouchThresholds, status, lastStatus;
int[][] filteredGraph, baselineGraph, touchGraph, releaseGraph, statusGraph;
Line[] filteredDataLines, touchLines, releaseLines;

int globalGraphPtr     = 0;
int electrodeNumber    = 0;
int serialNumber       = 4;
int lastMillis         = 0;
int mouseOverElectrode = -1;
int lastClickY         = 0;

int DEFAULT_WIDTH  = 1024;
int DEFAULT_HEIGHT = 600;
int prevWidth      = 0;
int prevHeight     = 0;

public void setup() {
  
  frameRate(60);
  

  smoothCanvas = (SmoothCanvas) getSurface().getNative();
  displayFrame = (JFrame) smoothCanvas.getFrame();
  displayFrame.setMinimumSize(new Dimension(1024, 600));
  displayFrame.setResizable(true);

  // init cp5
  cp5 = new ControlP5(this);

  setupOscSend();
  // setup OSC receiver on port 3000
  oscP5 = new OscP5(this, 3001);

  // init serial
  int validPortCount = 0;
  String[] serialList = Serial.list();

  for (int i = 0; i < serialList.length; i++) {
    if (!(serialList[i].toLowerCase().contains("/dev/tty.") || serialList[i].toLowerCase().contains("bluetooth"))) {
      validPortCount++;
    }
  }

  validSerialList = new String[validPortCount];
  validPortCount = 0;

  for (int i = 0; i < serialList.length; i++) {
    if (!(serialList[i].toLowerCase().contains("/dev/tty.") || serialList[i].toLowerCase().contains("bluetooth"))) {
      validSerialList[validPortCount++] = serialList[i];
    }
  }



  setupGraphs();
  setupStartPrompt();
  setupRunGUI();
  setupLabels();
  readSettings();
}

public void fullReset() {
  serialSelected = false;
  oscSelected    = false;
  firstRead      = true;
  secondRead     = false;
  paused         = false;
  helpVisible    = false;
  cursorVisible  = false;
  isResetting    = false;
  prevWidth      = 0;
  prevHeight     = 0;
  mode           = "GRAPHS";

  disableRunGUI();
  hideGraphLabels();
  electrodeSelector.hide();
  enableStartPrompt();
}

public void draw() {
  surface.setTitle("fps:"+(int)frameRate);
  if (width != prevWidth || height != prevHeight) {
    updatePositions();
    prevWidth = width;
    prevHeight = height;
  }

  if (mode == "DARK") {
    background(0);
  } else {
    background(200);
  }

  stroke(255);

  if (!serialSelected && !oscSelected) {
    return;
  }

  if (mode == "BARS") {
    drawGrid();
    drawBars();
    drawThresholds();
  }

  if (mode == "GRAPHS") {
    drawGrid();
    drawGraphs(filteredGraph, electrodeNumber, filteredColour);
    drawGraphs(baselineGraph, electrodeNumber, baselineColour);
    drawGraphs(touchGraph, electrodeNumber, touchedColour);
    drawGraphs(releaseGraph, electrodeNumber, releasedColour);
    drawStatus(electrodeNumber);
  }

  if (mode == "DARK") {
    drawDarkModeGraphs(filteredGraph, electrodeNumber);
  }

  if ((mode == "GRAPHS" || mode == "BARS") && cursorVisible) {
    updateCursorLabel();
    drawCursor();
  }

  if (helpVisible) {
    fill(200);
    stroke(60);
    strokeWeight(1);
    rectMode(CORNER);

    rect(
      (int)rescaleWidth(graphsLeft + 50) - 10, 
      (int)rescaleHeight(44) - 10, 
      250, 
      92
      );
  }

  if ((millis() > lastMillis + 500) && paused) {
    lastMillis = millis();
    pausedIndicator.setVisible(!pausedIndicator.isVisible());
  }

  if (isResetting) {
    if (millis() - resetStartTime > RESET_TIMEOUT) {
      fullReset();
    } else {
      boolean connected = false;

      try {
        setupSerial();
        inPort.read();
        connected = true;
      } 
      catch (RuntimeException e) {
      }

      if (connected) {
        firstRead = true;
        secondRead = false;
        isResetting = false;
      }
    }
  }

  // Agregado Matias 09.2021
  updateOscSend();

  //david:
  pushStyle();
  textAlign(RIGHT);
  textSize(18);
  fill(20);
  text(serialPortNameSelected, width-20, 22);
  fill(20);
  text("acumularFrames: "+ acumularFrames, width-20, 42);





  popStyle();
}

public @Override void exit() {
  saveSettings();
  super.exit();
}

public String JSONGetStrOr(JSONObject o, String key, String notFound) {

  String value = notFound;

  try {
    value = o.getString(key);
  } 
  catch (RuntimeException e) {
  }

  return value;
}


public int JSONGetIntOr(JSONObject o, String key, int notFound) {
  int value = notFound;

  try {
    value = o.getInt(key);
  } 
  catch (RuntimeException e) {
  }

  return value;
}


public boolean JSONGetBooleanOr(JSONObject o, String key, boolean notFound) {

  boolean value = notFound;

  try {
    value = o.getBoolean(key);
  } 
  catch (RuntimeException e) {
  }

  return value;
}



public void readSettings() {
  File settingsFile = new File(dataPath("settings.json"));

  if (!settingsFile.exists()) {
    return;
  }

  JSONObject settings = loadJSONObject(dataPath("settings.json"));

  int windowWidth = JSONGetIntOr(settings, "windowWidth", 1024);
  int windowHeight = JSONGetIntOr(settings, "windowHeight", 600);

  int windowX = JSONGetIntOr(settings, "windowX", -1);
  int windowY = JSONGetIntOr(settings, "windowY", -1);

  displayFrame.setSize(windowWidth, windowHeight);

  if (windowX >= 0 && windowY >= 0) {
    displayFrame.setLocation(new Point(windowX, windowY));
  }

  /*** agregador por David 9/2021 para seleccionar puerto automaticamente ***/

  //busco del settings si ya hay una placa seleccionada
  serialPortNameSelected = JSONGetStrOr ( settings, "serialPortNameSelected", "" );
  //println( "puerto seleccionado:" + serialPortNameSelected );
  if ( serialPortNameSelected!="") {
    for (int i = 0; i < validSerialList.length; i++) {
      if (validSerialList[i].toLowerCase().equals(serialPortNameSelected)) {
        serialNumber = i;
        setupSerial();

        serialSelected = true;
        oscSelected    = false;

        electrodeSelector.hide();
        disableStartPrompt();
        enableRunGUI();
        showGraphLabels();
        electrodeSelector.show();
        updatePositions();
      }
    }
  }

  //y tambien le agrego opcion de acumulador
  //(para eliminar ruido):
  USAR_ACUMULADOR = JSONGetBooleanOr ( settings, "USAR_ACUMULADOR", true );
  acumularFrames = JSONGetIntOr ( settings, "acumularFrames", 20 );


  /**** end agregado por David ********/
}

public void saveSettings() {

  JSONObject settings = new JSONObject();

  settings.setInt("windowWidth", prevWidth);
  settings.setInt("windowHeight", prevHeight);

  settings.setInt("windowX", displayFrame.getX());
  settings.setInt("windowY", displayFrame.getY());

  settings.setString("serialPortNameSelected", serialPortNameSelected );
  settings.setInt("acumularFrames", acumularFrames );
  settings.setBoolean("USAR_ACUMULADOR", USAR_ACUMULADOR );



  saveJSONObject(settings, dataPath("settings.json"));
}

public void oscEvent(OscMessage oscMessage) {
  if (paused || !oscSelected) {
    return;
  }

  if (firstRead && oscMessage.checkAddrPattern("/diff")) {
    firstRead = false;
  } else {
    if (oscMessage.checkAddrPattern("/touch")) {
      updateArrayOSC(status, oscMessage.arguments());
    } else if (oscMessage.checkAddrPattern("/tths")) {
      updateArrayOSC(touchThresholds, oscMessage.arguments());
    } else if (oscMessage.checkAddrPattern("/rths")) {
      updateArrayOSC(releaseThresholds, oscMessage.arguments());
    } else if (oscMessage.checkAddrPattern("/fdat")) {
      updateArrayOSC(filteredData, oscMessage.arguments());
    } else if (oscMessage.checkAddrPattern("/bval")) {
      updateArrayOSC(baselineVals, oscMessage.arguments());
    } else if (oscMessage.checkAddrPattern("/diff")) {
      updateArrayOSC(diffs, oscMessage.arguments());
      updateGraphs(); // update graphs when we get a DIFF line as this is the last of our dataset
    }
  }
}

public void serialEvent(Serial p) {
  if (paused || !serialSelected) {
    return;
  }

  inString = p.readString();
  splitString = splitTokens(inString, ": ");

  if (firstRead && splitString[0].equals("DIFF")) {
    firstRead = false;
    secondRead = true;
  } else {
    if (splitString[0].equals("TOUCH")) {
      updateArraySerial(status);
    } else if (splitString[0].equals("TTHS")) {
      updateArraySerial(touchThresholds);
    } else if (splitString[0].equals("RTHS")) {
      updateArraySerial(releaseThresholds);
    } else if (splitString[0].equals("FDAT")) {
      updateArraySerial(filteredData);
    } else if (splitString[0].equals("BVAL")) {
      updateArraySerial(baselineVals);
    } else if (splitString[0].equals("DIFF")) {
      updateArraySerial(diffs);

      if (secondRead) {
        for (int i = 0; i < numElectrodes; i++) {
          fakeTouchThresholds[i] = touchThresholds[i];
          fakeReleaseThresholds[i] = releaseThresholds[i];
          lastFakeTouchThresholds[i] = touchThresholds[i];
        }
      }
      secondRead = false;

      updateGraphs(); // update graphs when we get a DIFF line as this is the last of our dataset
    }
  }
}

public void setupSerial() {
  inPort = new Serial(this, validSerialList[serialNumber], baudRate);
  inPort.bufferUntil(lf);
}

public void controlEvent(ControlEvent controlEvent) {
  if (controlEvent.isFrom(cp5.getController("electrodeSel"))) {
    electrodeNumber = (int)controlEvent.getController().getValue();
  } else if (controlEvent.isFrom(cp5.getController("serialSel"))) {
    serialNumber = (int)controlEvent.getController().getValue();

    //por David, para grabar en el jSon al salir:
    serialPortNameSelected = validSerialList[serialNumber]; 


    setupSerial();

    serialSelected = true;
    oscSelected    = false;

    disableStartPrompt();
    enableRunGUI();
    showGraphLabels();
    electrodeSelector.show();
    updatePositions();
  } else if (controlEvent.isFrom(cp5.getController("oscButton"))) {
    serialSelected = false;
    oscSelected    = true;

    disableStartPrompt();
    enableRunGUI();
    showGraphLabels();
    electrodeSelector.show();
    updatePositions();
  } else if (controlEvent.isFrom(cp5.getController("helpButton"))) {
    helpVisible = !helpVisible;

    if (helpVisible) {
      instructions.show();
    } else {
      instructions.hide();
    }
  } else if (controlEvent.isFrom(cp5.getController("resetButton"))) {
    inPort.write("RESET\n");
    inPort.clear();
    inPort.stop();

    delay(100);

    resetStartTime = millis();
    isResetting = true;
  }
}

//agregue el escroll del mouse para ajuste fino

public void mouseWheel( MouseEvent e ) {


  for (int i = 0; i < numElectrodes; i++) {
    if (mouseX >= rescaleWidth(filteredDataLines[i].left) &&
      mouseX <= rescaleWidth(filteredDataLines[i].right) &&
      mouseY >= rescaleHeight(filteredDataLines[i].top) &&
      mouseY <= rescaleHeight(filteredDataLines[i].bottom)) {
      mouseOverElectrode = i;


      int inc = e.getCount()>0 ? -1 : 1;


      fakeTouchThresholds[mouseOverElectrode]+= inc;
      fakeReleaseThresholds[mouseOverElectrode] = fakeTouchThresholds[mouseOverElectrode] / 2;

      // touch threshold
      inPort.write("STTH:" + mouseOverElectrode + ":" + fakeTouchThresholds[mouseOverElectrode] + "\n");

      // release threshold
      inPort.write("SRTH:" + mouseOverElectrode + ":" + fakeReleaseThresholds[mouseOverElectrode] + "\n");

      return;
    }
  }
}
public void mousePressed() {
  if (mode != "BARS") {
    return;
  }

  for (int i = 0; i < numElectrodes; i++) {
    if (mouseX >= rescaleWidth(filteredDataLines[i].left) &&
      mouseX <= rescaleWidth(filteredDataLines[i].right) &&
      mouseY >= rescaleHeight(filteredDataLines[i].top) &&
      mouseY <= rescaleHeight(filteredDataLines[i].bottom)) {
      mouseOverElectrode = i;
      lastClickY = mouseY;
      return;
    }
  }

  mouseOverElectrode = -1; // invalid
}

public void mouseDragged() {
  if (mode != "BARS") {
    return;
  }

  if (mouseOverElectrode >= 0 && mouseOverElectrode < numElectrodes) {
    int threshDiff = (mouseY - lastClickY) * 2;

    fakeTouchThresholds[mouseOverElectrode] = lastFakeTouchThresholds[mouseOverElectrode] + threshDiff;
    if (fakeTouchThresholds[mouseOverElectrode] < 2) {
      fakeTouchThresholds[mouseOverElectrode] = 2;
    } else if (fakeTouchThresholds[mouseOverElectrode] > 255) {
      fakeTouchThresholds[mouseOverElectrode] = 255;
    }

    if (baselineVals[mouseOverElectrode] - fakeTouchThresholds[mouseOverElectrode] <= 10) {
      fakeTouchThresholds[mouseOverElectrode] = baselineVals[mouseOverElectrode] - 10;
    }

    fakeReleaseThresholds[mouseOverElectrode] = fakeTouchThresholds[mouseOverElectrode] / 2;
  }
}

public void mouseReleased() {
  if (mode != "BARS") {
    return;
  }

  if (mouseOverElectrode >= 0 && mouseOverElectrode < numElectrodes) {
    lastFakeTouchThresholds[mouseOverElectrode] = fakeTouchThresholds[mouseOverElectrode];

    // touch threshold
    inPort.write("STTH:" + mouseOverElectrode + ":" + fakeTouchThresholds[mouseOverElectrode] + "\n");

    // release threshold
    inPort.write("SRTH:" + mouseOverElectrode + ":" + fakeReleaseThresholds[mouseOverElectrode] + "\n");
  }
}

public void keyPressed() {
  if (!(serialSelected || oscSelected)) {
    return;
  }

  if (key == CODED) {
    if (keyCode == LEFT) {
      if (electrodeSelector.getValue() > 0) {
        electrodeSelector.setValue((int)electrodeSelector.getValue() - 1);
      }
    } else if (keyCode == RIGHT) {
      if (electrodeSelector.getValue() < numElectrodes - 1) {
        electrodeSelector.setValue((int)electrodeSelector.getValue() + 1);
      }
    }
  } else if (key == 'p' || key == 'P') {
    paused = !paused;
    lastMillis = millis();

    if (paused) {
      pausedIndicator.setVisible(true);
    } else {
      pausedIndicator.setVisible(false);
    }
  } else if (key == 's' || key == 'S') {
    mode = "DARK";

    for (int i = 1; i < numFooterLabels; i++) {
      labels[i + numVerticalDivisions + 1].setVisible(false);
    }

    cursorVisible = false;
    cursorLabel.hide();
    electrodeSelector.hide();
    disableRunGUI();
    hideGraphLabels();
  } else if (key == 'g' || key == 'G') {
    mode = "GRAPHS";
    electrodeSelector.show();
    enableRunGUI();
    showGraphLabels();
  } else if (key == 'b' || key == 'B') {
    mode = "BARS";
    electrodeSelector.hide();
    enableRunGUI();
    hideGraphLabels();
  } else if (key == 'd' || key == 'D') {
    csvDump();
  } else if (key == 'h' || key == 'H') {
    helpVisible = !helpVisible;

    if (helpVisible) {
      instructions.show();
    } else {
      instructions.hide();
    }
  } else if (key == 'c' || key == 'C') {
    cursorVisible = !cursorVisible;

    if (cursorVisible) {
      cursorLabel.show();
    } else {
      cursorLabel.hide();
    }
  } else if (key == '+') {
    acumularFrames++;
  } else if (key=='-') {
    acumularFrames--;
    acumularFrames = acumularFrames<0 ? 0 : acumularFrames;
  }
}

public void csvDump() {
  String outFileName;
  PrintWriter outFile;
  int i;
  int j;

  outFileName = "CSV dumps/CSV dump " + nf(year(), 4) + "-" + nf(month(), 2) + "-" + nf(day(), 2) + " at " + nf(hour(), 2) + "." + nf(minute(), 2) + "." + nf(second(), 2) + ".csv";
  outFile = createWriter(outFileName);

  // columns: E0 filtered data, E0 baseline data, E0 touch threshold, E0 release threshold, E1 filtered data...
  for (i = 0; i < numElectrodes; i++) {
    outFile.print("E" + str(i) + " filtered data," + "E" + str(i) + " baseline data," + "E" + str(i) + " touch threshold," + "E" + str(i) + " release threshold");

    if (i == numElectrodes - 1) {
      outFile.println(); // end of line doesn't need any extra commas
    } else {
      outFile.print(","); // add a comma to separate next batch of headers
    }
  }

  int localGraphPtr = globalGraphPtr;
  int numPointsWritten = 0;

  while (numPointsWritten < numGraphPoints) {
    for (i = 0; i < numElectrodes; i++) {
      outFile.print(
        str(filteredGraph[i][localGraphPtr]) + "," +
        str(baselineGraph[i][localGraphPtr]) + "," +
        str(touchGraph[i][localGraphPtr]) + "," +
        str(releaseGraph[i][localGraphPtr])
        );

      if (i == numElectrodes - 1) {
        outFile.println(); // end of line doesn't need any extra commas
      } else {
        outFile.print(","); // add a comma to separate next batch of headers
      }
    }

    if (++localGraphPtr >= numGraphPoints) {
      localGraphPtr = 0;
    }

    numPointsWritten++;
  }

  // flush the changes and close the file
  outFile.flush();
  outFile.close();

  println("CSV snapshot dumped to " + sketchPath(outFileName));
}
/*******************************************************************************

 Bare Conductive MPR121 output grapher / debug plotter for TouchBoard and Pi Cap
 -------------------------------------------------------------------------------

 GUIhelpers.pde - helper functions for mpr121_grapher.pde

 Bare Conductive code written by Stefan Dzisiewski-Smith and Szymon Kaliski.

 This work is licensed under a MIT license https://opensource.org/licenses/MIT

 Copyright (c) 2016, Bare Conductive

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.

*******************************************************************************/

public void customiseSL(ScrollableList sl) {
  // a convenience function to customize a DropdownList
  sl.setBackgroundColor(color(190));
  sl.setItemHeight(20);
  sl.setBarHeight(20);
  sl.getCaptionLabel().set("dropdown");
  sl.setColorBackground(color(60));
  sl.setColorActive(color(255, 128));
  sl.setSize(210, 100);
}

public void setupLabels() {
  labels = new Textlabel[numFooterLabels + numVerticalDivisions + 1];

  String footerLabels[] = { "FILTERED DATA", "BASELINE DATA", "TOUCHED LEVEL", "RELEASED LEVEL", "TOUCH EVENT", "RELEASE EVENT" };
  int footerColours[]   = { filteredColour, baselineColour, touchedColour, releasedColour, touchColourGraph, releaseColour };

  for (int i = 0; i < numVerticalDivisions + 1; i++) {
    labels[i] = cp5
                .addTextlabel(String.valueOf(tenBits - (i * tenBits / numVerticalDivisions)))
                .setText(String.valueOf(tenBits - (i * tenBits / numVerticalDivisions)))
                .setColorValue(textColour);

    labels[i].hide();
  }

  for (int i = 0; i < numFooterLabels; i++) {
    labels[i + numVerticalDivisions + 1] = cp5
                                           .addTextlabel(footerLabels[i])
                                           .setText(footerLabels[i])
                                           .setColorValue(footerColours[i]);

    labels[i + numVerticalDivisions + 1].hide();
  }

  pausedIndicator = cp5
                    .addTextlabel("pausedIndicator")
                    .setText("PAUSED")
                    .setColorValue(color(255, 0, 0, 200))
                    .setVisible(false);

  cursorLabel = cp5
                .addTextlabel("")
                .setColorValue(textColour)
                .setVisible(false);
}

public void setupRunGUI() {
  electrodeSelector = cp5.addScrollableList("electrodeSel");
  electrodeSelector.hide();
  customiseSL(electrodeSelector);
  electrodeSelector.getCaptionLabel().set("electrode number");
  for (int i = 0; i < numElectrodes; i++) {
    electrodeSelector.addItem("electrode " + i, i);
  }
  electrodeSelector.setValue(electrodeNumber);

  instructions = cp5
                 .addTextlabel("pauseInstructions")
                 .setText("PRESS H TO TOGGLE HELP\nPRESS P TO TOGGLE PAUSE\nPRESS C TO TOGGLE CURSOR\nRPRESS D TO DUMP DATA\n\nPRESS S TO SEE JUST FILTERED DATA (SOLO MODE)\nPRESS B TO SEE BAR GRAPH\nPRESS G TO GET BACK TO GRAPHS")
                 .setColorValue(textColour);

  instructions.hide();

  helpButton = cp5
               .addButton("helpButton")
               .setCaptionLabel("HELP")
               .setColorBackground(color(60))
               .setSize(100, 20);

  helpButton.hide();

  resetButton = cp5
                .addButton("resetButton")
                .setCaptionLabel("RESET")
                .setColorBackground(color(60))
                .setSize(100, 20);

  resetButton.hide();
}

public void setupStartPrompt() {
  startPrompt = cp5
                .addTextlabel("startPromptLabel")
                .setText("SELECT THE SERIAL PORT THAT YOUR BARE CONDUCTIVE TOUCH BOARD IS CONNECTED TO, OR CHOOSE OSC SO WE CAN BEGIN:")
                .setColorValue(textColour) ;

  serialSelector = cp5.addScrollableList("serialSel");
  customiseSL(serialSelector);
  serialSelector.getCaptionLabel().set("serial port");

  for (int i = 0; i < validSerialList.length; i++) {
    serialSelector.addItem(validSerialList[i], i);
  }

  serialSelector.close();

  oscButton = cp5
              .addButton("oscButton")
              .setCaptionLabel("OSC")
              .setColorBackground(color(60))
              .setSize(120, 20);
}

public void updateCursorLabel() {
  float value = tenBits - (((mouseY - rescaleHeight(graphsTop)) / rescaleHeight(graphsHeight)) * tenBits);
  value = constrain(value, 0, 1024);

  float posY = constrain(mouseY, rescaleHeight(graphsTop), rescaleHeight(graphsTop + graphsHeight));

  cursorLabel
  .setText(String.valueOf(round(value)))
  .setPosition(
    (int)rescaleWidth(graphsLeft + graphsWidth) - 28,
    posY - 5
  );
}

public void enableStartPrompt() {
  startPrompt.show();
  serialSelector.show();
  oscButton.show();
}

public void disableStartPrompt() {
  startPrompt.hide();
  serialSelector.hide();
  oscButton.hide();
}

public void enableRunGUI() {
  helpButton.show();

  if (serialSelected) {
    resetButton.show();
  }

  for (int i = 0; i < numVerticalDivisions + 1; i++) {
    labels[i].show();
  }
}

public void disableRunGUI() {
  helpButton.hide();
  resetButton.hide();

  for (int i = 0; i < numVerticalDivisions + 1; i++) {
    labels[i].hide();
  }

}

public void showGraphLabels() {
  for (int i = 0; i < numFooterLabels; i++) {
    labels[i + numVerticalDivisions + 1].show();
  }
}

public void hideGraphLabels() {
  for (int i = 0; i < numFooterLabels; i++) {
    labels[i + numVerticalDivisions + 1].hide();
  }
}

public void updatePositions() {
  for (int i = 0; i < numVerticalDivisions + 1; i++) {
    labels[i].setPosition(
      (int)rescaleWidth(graphsLeft),
      (int)rescaleHeight(graphsTop + i * (graphsHeight / numVerticalDivisions) - 10));
  }

  for (int i = 0; i < numFooterLabels; i++) {
    labels[i + numVerticalDivisions + 1].setPosition(
      (int)rescaleWidth(graphFooterLeft + 200 + 100 * i),
      (int)rescaleHeight(graphFooterTop));
  }

  electrodeSelector.setPosition((int)rescaleWidth(graphsLeft) + (serialSelected ? 240 : 120), (int)rescaleHeight(10));
  helpButton.setPosition((int)rescaleWidth(graphsLeft), (int)rescaleHeight(10));
  instructions.setPosition((int)rescaleWidth(graphsLeft + 50), (int)rescaleHeight(44));
  oscButton.setPosition((int)rescaleWidth(530), (int)rescaleHeight(150));
  pausedIndicator.setPosition((int)rescaleWidth(965), (int)rescaleHeight(graphFooterTop));
  resetButton.setPosition((int)rescaleWidth(graphsLeft) + 120, (int)rescaleHeight(10));
  serialSelector.setPosition((int)rescaleWidth(103), (int)rescaleHeight(150));
  startPrompt.setPosition((int)rescaleWidth(100), (int)rescaleHeight(100));

  // update cp5 graphics binding so event handlers can realign their picking coordinates
  cp5.setGraphics(this, 0, 0);
}
NetAddress direccionRemota;

int puerto =  3000;
String ip = "127.0.0.1";

OscBundle oscBundle;

boolean USAR_ACUMULADOR = true; //la toma del settings 
int acumularFrames = 0; //la toma del settings
int[] acumuladorXSensor = new int[numElectrodes];
boolean[] enviadoTouched = new boolean[numElectrodes];

public void setupOscSend() {
  direccionRemota = new NetAddress(ip, puerto);
  oscBundle = new OscBundle();
}

public void updateOscSend() {

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

public void sendTouched(int sensor) {
  OscMessage mensaje = new OscMessage("/touched"); //crea una etiqueta para el mensaje
  mensaje.add(sensor); // se le agrega un dato

  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
}

public void sendReleased(int sensor) {
  OscMessage mensaje = new OscMessage("/released"); //crea una etiqueta para el mensaje
  mensaje.add(sensor); // se le agrega un dato

  oscP5.send(mensaje, direccionRemota); //Se envia el mensaje
}
/*******************************************************************************
 
 Bare Conductive MPR121 output grapher / debug plotter for TouchBoard and Pi Cap
 -------------------------------------------------------------------------------
 
 graphHelpers.pde - helper functions for mpr121_grapher.pde
 
 Bare Conductive code written by Stefan Dzisiewski-Smith and Szymon Kaliski.
 
 This work is licensed under a MIT license https://opensource.org/licenses/MIT
 
 Copyright (c) 2016, Bare Conductive
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 *******************************************************************************/

public float rescaleWidth(float w) {
  return map(w, 0, DEFAULT_WIDTH, 0, width);
}

public float rescaleHeight(float h) {
  return map(h, 0, DEFAULT_HEIGHT, 0, height);
}

public void setupGraphs() {
  filteredData              = new int[numElectrodes];
  baselineVals              = new int[numElectrodes];
  diffs                     = new int[numElectrodes];
  touchThresholds           = new int[numElectrodes];
  releaseThresholds         = new int[numElectrodes];
  fakeTouchThresholds       = new int[numElectrodes];
  fakeReleaseThresholds     = new int[numElectrodes];
  lastFakeTouchThresholds   = new int[numElectrodes];
  status                    = new int[numElectrodes];
  lastStatus                = new int[numElectrodes];
  filteredDataLines         = new Line[numElectrodes];
  touchLines                = new Line[numElectrodes];
  releaseLines              = new Line[numElectrodes];

  for (int i = 0; i < numElectrodes; i++) {
    status[i] = 128; // 128 is an unused value from the Arduino input
    lastStatus[i] = 128;
    filteredDataLines[i] = new Line();
    touchLines[i] = new Line();
    releaseLines[i] = new Line();
  }

  filteredGraph = new int[numElectrodes][numGraphPoints];
  baselineGraph = new int[numElectrodes][numGraphPoints];
  touchGraph    = new int[numElectrodes][numGraphPoints];
  releaseGraph  = new int[numElectrodes][numGraphPoints];
  statusGraph   = new int[numElectrodes][numGraphPoints];
}

public void updateArrayOSC(int[] array, Object[] data) {
  if (array == null || data == null) {
    return;
  }

  for (int i = 0; i < min(array.length, data.length); i++) {
    array[i] = (int)data[i];
  }
}

public void updateArraySerial(int[] array) {
  if (array == null) {
    return;
  }

  for (int i = 0; i < min(array.length, splitString.length - 1); i++) {
    try {
      array[i] = Integer.parseInt(trim(splitString[i + 1]));
    } 
    catch (NumberFormatException e) {
      array[i] = 0;
    }
  }
}

public void updateGraphs() {
  int lastGraphPtr = globalGraphPtr - 1;

  if (lastGraphPtr < 0) {
    lastGraphPtr = numGraphPoints - 1;
  }

  for (int i = 0; i < numElectrodes; i++) {
    filteredGraph[i][globalGraphPtr] = filteredData[i];
    baselineGraph[i][globalGraphPtr] = baselineVals[i];
    touchGraph[i][globalGraphPtr]    = baselineVals[i] - touchThresholds[i];
    releaseGraph[i][globalGraphPtr]  = baselineVals[i] - releaseThresholds[i];


    if (lastStatus[i] == 0 && status[i] != 0x00) {
      // touched
      statusGraph[i][globalGraphPtr] = 1;
    } else if (lastStatus[i] != 0x00 && status[i] == 0x00) {
      // released
      statusGraph[i][globalGraphPtr] = -1;
    } else {
      statusGraph[i][globalGraphPtr] = 0;
    }
  }

  for (int i = 0; i < numElectrodes; i++) {
    lastStatus[i] = status[i];
  }

  if (++globalGraphPtr >= numGraphPoints) {
    globalGraphPtr = 0;
  }
}

public void drawBars() {
  int scratchColor    = g.strokeColor;
  int scratchFill     = g.fillColor;
  float scratchWeight = g.strokeWeight;

  stroke(0, 0, 0, 0);
  strokeWeight(0);

  rectMode(CORNERS);

  for (int i = 0; i < numElectrodes; i++) {
    if (status[i] == 0) {
      fill(filteredColour);
    } else {
      fill(touchColourBar);
    }
    filteredDataLines[i].left   = graphsLeft + graphFooterLeft + 5 + ((i * (graphsWidth - graphFooterLeft)) / numElectrodes);
    filteredDataLines[i].right  = graphsLeft + graphFooterLeft + (((i + 1) * (graphsWidth - graphFooterLeft)) / numElectrodes);
    filteredDataLines[i].bottom = graphsTop + graphsHeight;
    filteredDataLines[i].top    = filteredDataLines[i].bottom - (int)(graphsHeight * ((float)filteredData[i] / (float)tenBits));

    rect(
      rescaleWidth(filteredDataLines[i].left), 
      rescaleHeight(filteredDataLines[i].top), 
      rescaleWidth(filteredDataLines[i].right), 
      rescaleHeight(filteredDataLines[i].bottom)
      );
  }
  stroke(scratchColor);
  strokeWeight(scratchWeight);
  fill(scratchFill);
}

public void drawGraphs(int[][] graph, int electrode, int graphColour) {
  int scratchColor    = g.strokeColor;
  int scratchFill     = g.fillColor;
  float scratchWeight = g.strokeWeight;

  stroke(graphColour);
  strokeWeight(2);
  fill(0, 0, 0, 0);

  int localGraphPtr = globalGraphPtr;
  int numPointsDrawn = 0;

  int thisX = -1;
  int thisY = -1;

  beginShape();

  while (numPointsDrawn < numGraphPoints) {
    thisX = (int)(graphsLeft + (numPointsDrawn * graphsWidth / numGraphPoints));
    thisY = (int)(graphsTop + (graphsHeight * (1 - ((float)graph[electrode][localGraphPtr] / (float)tenBits))));

    vertex(rescaleWidth(thisX), rescaleHeight(thisY));

    if (++localGraphPtr >= numGraphPoints) {
      localGraphPtr = 0;
    }

    numPointsDrawn++;
  }

  endShape();

  stroke(scratchColor);
  strokeWeight(scratchWeight);
  fill(scratchFill);
}

public void drawDarkModeGraphs(int[][] graph, int electrode) {
  stroke(255);
  strokeWeight(5);
  fill(0, 0, 0, 0);

  int localGraphPtr = globalGraphPtr;
  int numPointsDrawn = 0;

  int thisX = -1;
  int thisY = -1;

  beginShape();

  while (numPointsDrawn < numGraphPoints) {
    thisX = (int)(graphsLeft + (numPointsDrawn * graphsWidth / numGraphPoints));
    thisY = (int)(graphsTop + (graphsHeight * (1 - ((float)graph[electrode][localGraphPtr] / (float)tenBits))));

    vertex(rescaleWidth(thisX), rescaleHeight(thisY));

    if (++localGraphPtr >= numGraphPoints) {
      localGraphPtr = 0;
    }

    numPointsDrawn++;
  }

  endShape();
}

public void drawThresholds() {
  int scratchColor    = g.strokeColor;
  int scratchFill     = g.fillColor;
  float scratchWeight = g.strokeWeight;

  strokeWeight(2);

  for (int i = 0; i < numElectrodes; i++) {
    touchLines[i].left   = graphsLeft + graphFooterLeft + 6 + ((i * (graphsWidth - graphFooterLeft)) / numElectrodes);
    touchLines[i].right  = graphsLeft + graphFooterLeft - 1 + (((i + 1) * (graphsWidth - graphFooterLeft)) / numElectrodes);
    touchLines[i].top    = graphsTop + (int)(graphsHeight * (1 - (((float)baselineVals[i] - (float)fakeTouchThresholds[i]) / (float)tenBits)));
    touchLines[i].bottom = touchLines[i].top;

    stroke(touchedColour);

    line(
      rescaleWidth(touchLines[i].left), 
      rescaleHeight(touchLines[i].top), 
      rescaleWidth(touchLines[i].right), 
      rescaleHeight(touchLines[i].bottom)
      );

    float x_temp = rescaleWidth(touchLines[i].left)+(rescaleWidth(touchLines[i].right)-rescaleWidth(touchLines[i].left))/2;  
    textSize(14);
    textAlign(CENTER);
    text(fakeTouchThresholds[i], x_temp  , rescaleHeight(touchLines[i].top)+14); //Agregado Matias 09/2021

    releaseLines[i].left   = graphsLeft + graphFooterLeft + 6 + ((i * (graphsWidth - graphFooterLeft)) / numElectrodes);
    releaseLines[i].right  = graphsLeft + graphFooterLeft - 1 + (((i + 1) * (graphsWidth - graphFooterLeft)) / numElectrodes);
    releaseLines[i].top    = graphsTop + (int)(graphsHeight * (1 - (((float)baselineVals[i] - (float)fakeReleaseThresholds[i]) / (float)tenBits)));
    releaseLines[i].bottom = releaseLines[i].top;

    stroke(releasedColour);

    line(
      rescaleWidth(releaseLines[i].left), 
      rescaleHeight(releaseLines[i].top), 
      rescaleWidth(releaseLines[i].right), 
      rescaleHeight(releaseLines[i].bottom)
      );

    text(fakeReleaseThresholds[i], x_temp, rescaleHeight(releaseLines[i].top)-4);

  }

  stroke(scratchColor);
  strokeWeight(scratchWeight);
  fill(scratchFill);
}

public void drawStatus(int electrode) {
  int scratchColor    = g.strokeColor;
  float scratchWeight = g.strokeWeight;

  strokeWeight(2);

  int thisX;

  int localGraphPtr = globalGraphPtr;
  int numPointsDrawn = 0;

  while (numPointsDrawn < numGraphPoints) {
    thisX = (int)(graphsLeft + (numPointsDrawn * graphsWidth / numGraphPoints));

    if (statusGraph[electrode][localGraphPtr] == 1) {
      stroke(touchColourGraph);

      line(
        rescaleWidth(thisX), 
        rescaleHeight(graphsTop), 
        rescaleWidth(thisX), 
        rescaleHeight(graphsTop + graphsHeight)
        );
    } else if (statusGraph[electrode][localGraphPtr] == -1) {
      stroke(releaseColour);

      line(
        rescaleWidth(thisX), 
        rescaleHeight(graphsTop), 
        rescaleWidth(thisX), 
        rescaleHeight(graphsTop + graphsHeight)
        );
    }

    if (++localGraphPtr >= numGraphPoints) {
      localGraphPtr = 0;
    }

    numPointsDrawn++;
  }

  stroke(scratchColor);
  strokeWeight(scratchWeight);
}

public void drawGrid() {
  int scratchColor    = g.strokeColor;
  float scratchWeight = g.strokeWeight;

  stroke(textColour);
  strokeWeight(1);

  for (int i = 0; i <= numVerticalDivisions; i++) {
    line(
      rescaleWidth(graphsLeft), 
      rescaleHeight(graphsTop + ((i * graphsHeight) / numVerticalDivisions)), 
      rescaleWidth(graphsLeft + graphsWidth), 
      rescaleHeight(graphsTop + ((i * graphsHeight) / numVerticalDivisions))
      );
  }

  stroke(scratchColor);
  strokeWeight(scratchWeight);
}

public void drawCursor() {
  stroke(textColour);
  strokeWeight(1);

  float posY = constrain(mouseY, rescaleHeight(graphsTop), rescaleHeight(graphsTop + graphsHeight));

  line(rescaleWidth(graphsLeft), posY, rescaleWidth(graphsLeft + graphsWidth), posY);

  fill(200);
  rectMode(CORNER);
  rect(
    rescaleWidth(graphsLeft + graphsWidth) - 30, 
    posY - 10, 
    30, 
    20
    );
}
  public void settings() {  size(1024, 600); }
  static public void main(String[] passedArgs) {
    String[] appletArgs = new String[] { "mpr121_grapher_osc" };
    if (passedArgs != null) {
      PApplet.main(concat(appletArgs, passedArgs));
    } else {
      PApplet.main(appletArgs);
    }
  }
}
