#include <Pixy2.h>
#include <PIDLoop.h>
#include <Wire.h>

Pixy2 pixy; //Declaration of Pixy cam object
PIDLoop panLoop(100, 0, 400, true);
PIDLoop tiltLoop(300, 0, 500, true);

int enA = 4; // Left Motor
int in1 = 5;
int in2 = 6;

int enB = 9; // Right Motor
int in3 = 7;
int in4 = 8;

int count = 0; //Variables for Pixy cam operation
int32_t stop_height = 70; // Set range for cart to stop and wait for the user
int32_t stop_width = 80;
int32_t stop_area = stop_height*stop_width;
int32_t BU_height = 160; // Set range for cart to reverse away from user to avoid collision
int32_t BU_width = 190;
int32_t BU_area = BU_height*BU_width;
int32_t min_width = 30; // Set max distance between cart and user
int32_t min_height = 20;
int32_t min_area = min_width*min_height;
int32_t current_height = 0; // Initialize the values for the Pixy cam to fill in
int32_t current_width = 0;
int32_t current_area = 0;

int x = 4; // initialize the variable that will be sent to slave arduino

void setup()
{  
  Serial.begin(115200);
  Serial.print("Starting...\n");
 
  // We need to initialize the pixy object 
  pixy.init();
  // Use color connected components program for the pan tilt to track 
  pixy.changeProg("color_connected_components");

  Wire.begin();
}

void loop()
{  
  static int i = 0;
  int j;
  char buf[64]; 
  int32_t panOffset, tiltOffset;
  
  // get active blocks from Pixy
  pixy.ccc.getBlocks();
  
  if (pixy.ccc.numBlocks)
  {        
    i++;
    
    if (i%60==0)
      Serial.println(i);   
    
    // calculate pan and tilt "errors" with respect to first object (blocks[0]), 
    // which is the biggest object (they are sorted by size).  
    panOffset = (int32_t)pixy.frameWidth/2 - (int32_t)pixy.ccc.blocks[0].m_x;
    tiltOffset = (int32_t)pixy.ccc.blocks[0].m_y - (int32_t)pixy.frameHeight/2;  

    current_height = pixy.ccc.blocks[0].m_height; // Calculate current size of the object
    current_width = pixy.ccc.blocks[0].m_width;
    current_area = current_height*current_width;
   
    //Serial.println(panOffset);
   
    // update loops
    panLoop.update(panOffset);
    tiltLoop.update(tiltOffset);

    //Serial.println(panLoop.m_command);
  
    // set pan and tilt servos  
    pixy.setServos(panLoop.m_command, tiltLoop.m_command);
   
#if 0 // for debugging
    sprintf(buf, "%ld %ld %ld %ld", rotateLoop.m_command, translateLoop.m_command, left, right);
    Serial.println(buf);   
#endif

  //}  
  if(count == 0)  // reset the Pixy to standard position at start of 1st iteration
  {
    panLoop.reset();
    tiltLoop.reset();
    pixy.setServos(panLoop.m_command, tiltLoop.m_command);
  }

  //Serial.print("current area");
  //Serial.println(current_area);

  if(count == 0)
  {
    delay(2000); // Create delay from detection of object to when the cart can stop moving
  }
  
  if (current_area > stop_area && current_area < BU_area) // If cart is within this range it will stop, maybe turn
  {
    if (panLoop.m_command>350 && panLoop.m_command<550 ) // pixy looking forward, stop and do not turn
    {
       x = 2;
       //Wire.beginTransmission(5);
       //Wire.write(x);
       //Wire.endTransmission();
      
       digitalWrite(in1, HIGH); //STOPPED
       digitalWrite(in2, HIGH);  
       digitalWrite(in3, HIGH); //STOPPED
       digitalWrite(in4, HIGH);  
       //Serial.println("stop")
        
       delay(10);
     }
    else if(panLoop.m_command<350 ) // pixy looking right, stop and spin in place to right
     {
       x = 0;
       //Wire.beginTransmission(5);
       //Wire.write(x);
       //Wire.endTransmission();
      
       digitalWrite(in1, HIGH); //FORWARD
       digitalWrite(in2, LOW);  
       digitalWrite(in3, LOW); //BACKWARD
       digitalWrite(in4, HIGH);  
       analogWrite(enA, 155); //HALF SPEED
       analogWrite(enB, 155); //HALF SPEED
       //Serial.println("stop_right");
       
       delay(10);
     }
     else // if not right or straight, stop and spin in place to left
     {
      x = 1;
      //Wire.beginTransmission(5);
      //Wire.write(x);
      //Wire.endTransmission();
      
      digitalWrite(in1, LOW); //BACKWARD
      digitalWrite(in2, HIGH);  
      digitalWrite(in3, HIGH); //FORWARD
      digitalWrite(in4, LOW);  
      analogWrite(enA, 155); //HALF SPEED
      analogWrite(enB, 155); //HALF SPEED
      //Serial.println(panLoop.m_command);
      //Serial.println("stop_left");
      
      delay(10); 
      }
  }
  else if (current_area > BU_area) // Too close, cart will back up
  {
      x = 3;
      //Wire.beginTransmission(5);
      //Wire.write(x);
      //Wire.endTransmission();
    
      digitalWrite(in1, LOW); //BACKWARD
      digitalWrite(in2, HIGH);  
      digitalWrite(in3, LOW); //BACKWARD
      digitalWrite(in4, HIGH);  
      analogWrite(enA, 250); //FULL SPEED
      analogWrite(enB, 250); //FULL SPEED
      //Serial.println("back up");
      
      delay(10);
  }
  else if (current_area < min_area) // Too far or no object detected, cart will stop
  {
      x = 2;
      //Wire.beginTransmission(5);
      //Wire.write(x);
      //Wire.endTransmission();
    
      digitalWrite(in1, HIGH); //STOPPED
      digitalWrite(in2, HIGH);  
      digitalWrite(in3, HIGH); //STOPPED
      digitalWrite(in4, HIGH);  
      //Serial.println("stop");
    
      delay(10);
  }
  else // User in ideal range then cart will operate normally
  {
    if (panLoop.m_command>400 && panLoop.m_command<500 ) // pixy in middle range go forward full speed
    {
       x = 4;
       //Wire.beginTransmission(5);
       //Wire.write(x);
       //Wire.endTransmission();
      
       digitalWrite(in1, HIGH); //FORWARD
       digitalWrite(in2, LOW);  
       digitalWrite(in3, HIGH); //FORWARD
       digitalWrite(in4, LOW);  
       analogWrite(enA, 250); //FULL SPEED
       analogWrite(enB, 250); //FULL SPEED
       //Serial.println("straight");
    
       delay(10);
     }
     else if(panLoop.m_command<400 ) // pixy turned to right, cart turned right
     {
       x = 0;
       //Wire.beginTransmission(5);
       //Wire.write(x);
       //Wire.endTransmission();
      
       digitalWrite(in1, HIGH); //FORWARD
       digitalWrite(in2, LOW);  
       digitalWrite(in3, HIGH); //FORWARD
       digitalWrite(in4, LOW);  
       analogWrite(enA, 250); //FULL SPEED
       analogWrite(enB, 20); //SLOW (ESSENTIALLY STOPPED)
       //Serial.println("right");
      
       delay(10);
     }
    else // pixy not left or right, cart turned left
      {
       x = 1;
       //Wire.beginTransmission(5);
       //Wire.write(x);
       //Wire.endTransmission();
        
       digitalWrite(in1, HIGH); //FORWARD
       digitalWrite(in2, LOW);  
       digitalWrite(in3, HIGH); //FORWARD
       digitalWrite(in4, LOW);  
       analogWrite(enA, 20); //FULL SPEED
       analogWrite(enB, 250); //SLOW (ESSENTIALLY STOPPED)
       //Serial.println(panLoop.m_command);
       //Serial.println("left");
        
       delay(10); 
      }
  }
  
   if (count == 0)
   {
      count = 1; // update count so that the loop only runs certain commands once
   }

    Wire.beginTransmission(5);
    Wire.write(x);
    Wire.endTransmission();
   
  }
  //Wire.beginTransmission(5);
  //Wire.write(x);
  //Wire.endTransmission();

  Serial.println(x);
}
