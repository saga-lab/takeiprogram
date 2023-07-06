import com.leapmotion.leap.Controller;
import com.leapmotion.leap.Frame;
import com.leapmotion.leap.Hand;
import com.leapmotion.leap.Finger;
import processing.serial.*;
import controlP5.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

//communitation definition
Serial myPort;
String portname = "/dev/cu.usbmodem11402";

final int DA_TEST = -1; //sinusoidal wave mode to test DA

int Mode = DA_TEST;

//stimulation and measurement data
final int ELECTRODE_NUM = 16;
int[] Impedance = new int[ELECTRODE_NUM];
int[] StimPattern = new int[ELECTRODE_NUM];
int[] TwoDStimPattern = new int[ELECTRODE_NUM + 1];

//graphical attributes
final int WINDOW_SIZE_X = 400;
final int WINDOW_SIZE_Y = 400;
final int X_OFFSET = 10;
final int Y_OFFSET = 10;
final int BAR_WIDTH = 10;

double freq = 0.0;
int amp = 0;
float cntrspeed = 0.0;

int select_num = 0;
int freq_num = 23;

ControlP5 Button;
PImage toby, pile, satin, fleece, microfiber, organdy, select;

Controller leapController = new Controller();

void settings() {
  size(1400, 800); 
}

void setup() {
  frameRate(30);
  myPort = new Serial(this, portname, 921600);
  myPort.clear();
  textSize(60);
  colorMode(RGB, 256);
  stroke(255, 0, 0);
  noFill();
  
  toby = loadImage("toby.png");
  pile = loadImage("pile.png");
  satin = loadImage("satin.png");
  fleece = loadImage("fleece.png");
  microfiber = loadImage("microfiber.png");
  organdy = loadImage("organdy.png");
  Button = new ControlP5(this);
}

void draw() {
  background(255);
  image(pile, 0 + 20, 0 + 40, 140, 140);
  image(toby, 0 + 20, 150 + 40, 140, 140);
  image(satin, 150 + 20, 0 + 40, 140, 140);
  image(fleece, 150 + 20, 150 + 40, 140, 140);
  image(microfiber, 300 + 20, 0 + 40, 140, 140);
  image(organdy, 300 + 20, 150 + 40, 140, 140);
  
  String ono_text = "";
  String waveType = "";
  strokeWeight(10);
  noFill();
  
  if(select_num == 1) {
    ono_text = "pile";
    waveType = "SINE WAVE, 100Hz";
    rect(20, 40, 140, 140);
    fill(0);
  } else if(select_num == 2) {
    ono_text = "doby";
    waveType = "SAWTOOTH, 60Hz";
    rect(20, 190, 140, 140);
    fill(0);
  } else if(select_num == 3) {
    ono_text = "satin";
    waveType = "SINE WAVE, 300Hz";    
    rect(170, 40, 140, 140);
    fill(0);
  } else if(select_num == 4) {
    ono_text = "fleece";
    waveType = "SQUARE WAVE, 130Hz";
    rect(170, 190, 140, 140);
    fill(0);
  } else if(select_num == 5) {
    ono_text = "microfiber";
    waveType = "DELTA Function, 120Hz";    
    rect(320, 40, 140, 140);
    fill(0);
  } else if(select_num == 6) {
    ono_text = "organdy";
    waveType = "DELTA Function, 180Hz";    
    rect(320, 190, 140, 140);    
    fill(0);
  } else {
    ono_text = "";
    waveType = "";
    freq = 0;
  }

  fill(0);
  textSize(30);
  text("Please select texture", 100, 360);
  textSize(30);
  text("Onomatopie : " + ono_text, 20, 450);
  text("Waveform     : " + waveType, 20, 500);

  text("Frequency    : " + freq + " Hz", 20, 550);
  text("Amplitude  : " + amp + " V", 20, 600);
  textSize(30);
  
  //Leap Motion
  textSize(20);
  Frame frame = leapController.frame();
 
  for(Hand hand : frame.hands()) { //各手の情報を取得するために追加
    for(Finger finger : hand.fingers()) {
      if(finger.type().toString() == "TYPE_INDEX") { // 人差し指
        float vx = finger.tipVelocity().getX() / 10; // x方向
        float vy = finger.tipVelocity().getY() / 10; // y方向
        float vz = finger.tipVelocity().getZ() / 10; // z方向
        float speed = sqrt(vx*vx + vy*vy + vz*vz); // ベクトルの大きさを計算
        
        text("x: " + vx, 700, 50);
        text("y: " + vy, 700, 70);
        text("z: " + vz, 700, 90);
        text("speed: " + speed, 700, 110);
        
        ellipse(finger.tipPosition().getX() + (width / 2) + 300, (height * 2 / 3) - finger.tipPosition().getY() + 100, 5, 5);

        //byte[] cntr = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putFloat(speed).array();
        
        int cntr = int(speed);
        myPort.write(cntr);
      }  
    }
  }
  LeapMotion();
  //receivechar();
  //receiveint();
}

void LeapMotion() { //mbedから受け取った値を処理
  while (myPort.available() > 0) {
    cntrspeed = myPort.read();
    println("Speed:" + cntrspeed);
  }
}

void receivechar() {
  while (myPort.available() > 0) {
    char receivedChar = myPort.readChar(); // 文字を受信
    println("Received char: " + receivedChar); // 受け取った文字をコンソールに表示
  }
}

void receiveint() {
  while (myPort.available() > 0) {
    int receivedInt = myPort.read() - 256; // 文字を受信
    println("Received char: " + receivedInt); // 受け取った文字をコンソールに表示
  }
}

void keyPressed() {
  if (key == '1') {
    println("DA test using sine wave");
    myPort.write(-1);
    Mode = DA_TEST;
  } else if(keyCode == DOWN) {
    amp -= 10;
    if(amp < 0)
      amp = 0;
    myPort.write(-9);
  } else if(keyCode == UP) {
    amp += 10;
    if(amp > 300)
      amp = 300;
    myPort.write(-10);
  } else if(keyCode == RIGHT) {
    freq += 5;
    if(freq > 500)
      freq = 500;
    myPort.write(-11);
  } else if(keyCode == LEFT) {
    freq -= 5;
    if(freq < 0)
      freq = 0;
    myPort.write(-12);
  } else if (key == ESC) {
    myPort.write(-13);
    exit();
  }
}

void mouseClicked(){
  if(mouseX >= 0 + 20 && mouseX <= 160 && mouseY >= 0 + 40 && mouseY <= 180) {
    select_num = 1;
    println ("Mouse Click");
    myPort.write(-2);
    Mode = -2;
  } else if(mouseX >= 0 + 20 && mouseX <= 160 && mouseY >= 150 + 40 && mouseY <= 330) {
    select_num = 2;
    println ("Mouse Click2");
    myPort.write(-3);
    Mode = -3;
  } else if(mouseX >= 150 + 20 && mouseX <= 340 && mouseY >= 0 + 40 && mouseY <= 180) {
    select_num = 3;
    println ("Mouse Click3");
    myPort.write(-4);
    Mode = -4;
  } else if(mouseX >= 150 + 20 && mouseX <= 340 && mouseY >= 150 + 40 && mouseY <= 330) {
    select_num = 4;
    println ("Mouse Click4");
    myPort.write(-5);
    Mode = -5;
  } else if(mouseX >= 300 + 20 && mouseX <= 460 && mouseY >= 0 + 40 && mouseY <= 180) {
    select_num = 5;
    println ("Mouse Click5");
    myPort.write(-6);
    Mode = -6;
  } else if(mouseX >= 300 + 20 && mouseX <= 460 && mouseY >= 150 + 40 && mouseY <= 330) {
    select_num = 6;
    println ("Mouse Click6");
    myPort.write(-7);
    Mode = -7;
  } else {
    select_num = 0;
    println ("Copymode");
    myPort.write(-8);
    Mode = -8;
  }
}
