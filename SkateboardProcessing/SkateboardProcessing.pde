import processing.serial.*;
Serial myPort;
PrintWriter output;

double monitorPeriod = 1; //Seconds
double resetTime = 1.2; //Seconds after period is over
double yAxisScale = 16; // Gs

int gAccelValue = 2100;
float lastAx = 0;
float lastAy = 0;
float lastAz = 0;
int lastXPos = 10000;
int lastTime = 0;
int windowStartTime = 0;

int azIntegration = 0;
boolean ready = false;
boolean finishedFrame = false;
boolean won = false;
int score = 0;
int winningScore = 2800;

void setup() {
   size(1920,1000);
   
   printArray(Serial.list());
   myPort = new Serial(this, Serial.list()[0], 38400);
   myPort.clear(); 
   
   background(240,240,240);
   strokeWeight(2);
   textSize(24);
   textAlign(CENTER);
}

void draw () {
   while (myPort.available () > 0) {
     String inString = myPort.readStringUntil('\n');
     if (inString != null) {
       inString = trim(inString);
       String[] xyRaw = splitTokens(inString, ",");
       if (xyRaw.length >= 3) {
         float ax = float(xyRaw[0]) / gAccelValue; // Read raw acceleration values, convert to acceleration in G's
         float ay = float(xyRaw[1]) / gAccelValue;
         float az = float(xyRaw[2]) / gAccelValue;
         
         finishedFrame = lastTime > monitorPeriod * 1000; // True when the graph escapes the left side of the window
         ready = lastTime > (monitorPeriod + resetTime) * 1000; // True [resetTime] seconds after the graph escapes
 
         if (abs(az) > 1.5 && ready){ //Condition to begin a new frame, once we're ready
           windowStartTime = millis();
           score = 0;
           finishedFrame = false;
           ready = false;
           won = false;
         }
         
         int time = millis() - windowStartTime; // Normalize time to the start time of this frame
         int xPos = TtoX(time);
         
         fill(240, 240, 240);
         stroke(240,240, 240);
         rect(xPos, 0, 100, height); // Overwrite box
         fill(0, 0, 0);
         stroke(0,0, 0);
         line(xPos, height/2, xPos + 100, height/2); // Zero line
         
         if (xPos > lastXPos && xPos - lastXPos < 200 && !finishedFrame){ // If the last position isn't ahead of the next position (wrap case) and we're still drawing in the domain space
           azIntegration += (time - lastTime) * abs(az + 1); // Sum integration term
           if (!won){
             myPort.write(int(254 * min(1.0 * azIntegration / winningScore, 1))); // Send 0 - 254 back to the arduino to use in the feedback LEDs and bell for the case of 254.
             won = azIntegration >= winningScore; // Don't send the winning signal multiple times if we get it.
           }
           
           stroke(255, 0, 0); 
           line(lastXPos - 10, GtoY(lastAx), xPos - 10, GtoY(ax)); // Connect lines
           stroke(0, 255, 0);
           line(lastXPos - 10, GtoY(lastAy), xPos - 10, GtoY(ay));
           stroke(0, 0, 255);
           line(lastXPos - 10, GtoY(lastAz), xPos - 10, GtoY(az));
         }
         else if (finishedFrame){ // If we're out of the frame domain
           if (score == 0){ // If we haven't already, draw the score
             score = azIntegration;
             azIntegration = 0;
             fill(240,240,240);
             stroke(240,240,240);
             rect(width - 125,25,100,30);
             fill(0,0,0);
             text(score, width - 75, 50);
           }
           if (ready){ // Once ready, display the "Ready" notice
             stroke(0, 255, 0);
             fill(0, 255, 0);
             rect(25,25,100,30);
             fill(0, 0, 0);
             text("READY", 75, 50);
           }
         }
         
         lastAx = ax; // Set previous values
         lastAy = ay;
         lastAz = az;
         lastXPos = xPos;
         lastTime = time;
       }
     }
   }
}

int GtoY(float g){ // Translate acceleration in G's to Y position in pixels
  return (int)(g/(yAxisScale * 2) * height + height / 2);
}

int TtoX(float t){ // Translate time in milliseconds to X position in pixels
  return (int)(t / (monitorPeriod * 1000) * width);
}