#include "mbed.h"
#include <cstdio>
#include <stdlib.h>

DigitalOut SN74LV595_DIN(p14);
DigitalOut SN74LV595_RCLK(p13);
DigitalOut SN74LV595_CLR(p12);
DigitalOut SN74LV595_CLK(p11);
DigitalIn  SN74LV595_DOUT(p10);
AnalogIn ain(p20);

Serial pc(USBTX, USBRX);

SPI spiDAAD(p5, p6, p7);
DigitalOut DA_sync(p8);
DigitalOut AD_cs(p9);
BusOut myleds(LED1, LED2, LED3, LED4);

const int DA_TEST = 256;
const int pile = 255;
const int doby = 254;
const int satin = 253;
const int fleece = 252;
const int microfiber = 251;
const int organdy = 250;
int Mode = DA_TEST;

const int ELECTRODE_NUM = 16;
const int PC_MBED_STIM_PATTERN = 0xFF;
const int PC_MBED_MEASURE_REQUEST = 0xFE;

short stim_pattern[ELECTRODE_NUM] = { 0 };
short impedance[ELECTRODE_NUM] = { 0 };
short twod_stim_pattern[ELECTRODE_NUM] = { 0 };
double freq = 0.0;
double amp = 0.0;
float vx = 0.0;
Timer timer;

bool AccessDeny = false;
bool trian = true;
bool delta_flag = false;
bool delta_oldflag = false;
int delta_cnt = 0;

void SN74LV595FastScan(int usWhichPin)
{
    int ii, pin;
    static int pos;

    SN74LV595_RCLK = 0;
    if (usWhichPin == 0) {
        SN74LV595_DIN = 0;
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
        SN74LV595_DIN = 1;
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
        pos = 0;
    } else {
        pin = usWhichPin - pos;
        for (ii = 0; ii < pin; ii++) {
            SN74LV595_DIN = 1;
            SN74LV595_CLK = 1;
            SN74LV595_CLK = 0;
            SN74LV595_DIN = 0;
            SN74LV595_CLK = 1;
            SN74LV595_CLK = 0;
        }
        pos = usWhichPin;
    }
    SN74LV595_RCLK = 1;
    SN74LV595_RCLK = 0;
}

void SN74LV595AllScan()
{
    int ii, pin;

    SN74LV595_RCLK = 0;
    for (int i = 1; i < ELECTRODE_NUM + 1; i++) {
        SN74LV595_DIN = ((twod_stim_pattern[i] == 1) ? 1 : 0);
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
        SN74LV595_DIN = ((twod_stim_pattern[i] == 1) ? 0 : 1);
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
    }
    SN74LV595_RCLK = 1;
    SN74LV595_RCLK = 0;
}

void SN74LV595Clear()
{
    SN74LV595_CLR = 0;
    SN74LV595_RCLK  = 0;
    SN74LV595_CLK = 0;
    SN74LV595_CLK = 1;
    SN74LV595_CLK = 0;
    SN74LV595_CLR = 1;
}

void SN74LV595Init(int TotalPin)
{
    int ii;

    SN74LV595_CLR = 1;
    SN74LV595_CLK = 0;
    SN74LV595_RCLK = 0;
    for (ii = 0; ii < TotalPin; ii++) {
        SN74LV595_DIN = 1;
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
        SN74LV595_DIN = 0;
        SN74LV595_CLK = 1;
        SN74LV595_CLK = 0;
    }
    SN74LV595_RCLK = 1;
    SN74LV595_RCLK = 0;
}

short DAAD(short DA)
{
    short AD;

    DA_sync = 0;
    AD_cs = 0;

    AD = spiDAAD.write(DA << 2);

    DA_sync = 1;
    AD_cs = 1;

    return AD >> 2;
}

void DAADinit()
{
    spiDAAD.format(16, 2);
    spiDAAD.frequency(48000000);
}

float SerialLeap()
{
    char receivedBytes[4];
    // 4バイトのデータを受信
    for (int i = 0; i < 4; ++i) {
        receivedBytes[i] = pc.getc();
    }

    // 4バイトをfloatに変換
    float vx;
    memcpy(&vx, receivedBytes, sizeof(float));

    return vx;
}

void SerialReceive()
{
    int i;
    unsigned char data[255];
    int datai[255];

    int rcv = pc.getc();
  
    if (rcv == DA_TEST) {
        Mode = DA_TEST;
        myleds = 1;
    } else if (rcv == pile) {
        Mode = pile;
        myleds = 2;
    } else if (rcv == doby) {
        Mode = doby;
        myleds = 2;
    } else if (rcv == satin) {
        delta_flag = false;
        delta_oldflag = false;
        Mode = satin;
        myleds = 2;
    } else if (rcv == fleece) {
        Mode = fleece;
        myleds = 2;
    } else if (rcv == microfiber) {
        Mode = microfiber;
        myleds = 2;
    } else if (rcv == organdy) {
        delta_flag = false;
        delta_oldflag = false;
        Mode = organdy;
        myleds = 2;
    } else if (rcv == 248) {
        amp -= 10.0;
        if (amp <0.0)
            amp = 0.0;
        myleds = 4;
    } else if (rcv == 247) {
        amp += 10.0;
        if (amp > 500.0)
            amp = 500.0;
        myleds = 4;
    } else if (rcv == 246) {
        freq += 5.0;
        if (freq > 500.0)
            freq = 500.0;
        myleds = 8;
    } else if (rcv == 245) {
        freq -= 5.0;
        if (freq < 0.0)
            freq = 0.0;
        myleds = 8;
    } else {
        SerialLeap();
    }
    
}

int main()
{
    double t = 0.0;
    char rcv;
    int i;
    short AD;

    int prevmode;

    float v = 0.0;

    DAADinit();
    SN74LV595Init(ELECTRODE_NUM);
    pc.attach(SerialReceive);
    pc.baud(921600);
    timer.start();

    for (int t = 0; t < 8; t++) {
        myleds = 1 << (t % 4);
        wait_us(50000);       
    }

    myleds = 1;

    while (1) {
        if (Mode == DA_TEST) {
            t = (double)timer.read_us() * 0.000001;
			AD = DAAD((short)(200.0 * (1.0 + sin(2.0 * 3.1415926 * 200.0 * t))));
        } else if (Mode == pile) {
            t = (double)timer.read_us() * 0.000001;
            v = 55.925696;
            AD = DAAD((short)(amp * (1.0 + sin(2.0 * 3.1415926 * 100.0 * (vx / v) * t))));
            prevmode = Mode;
        } else if (Mode == doby) {
            t = (double)timer.read_us() * 0.000001;
            v = 53.475115;
            int d = (int)(t * 100 * 60.0 * (vx / v)) % 100;
            double dd = (double)d / 100;
            dd *= 2 * amp;
            if (dd > 600)
                dd = 600;
            AD = DAAD((short)dd);
            prevmode = Mode;
        } else if (Mode == satin) {
            t = (double)timer.read_us() * 0.000001;
            v = 58.459970;
            AD = DAAD((short)(amp * (1.0 + sin(2.0 * 3.1415926 * 300.0 * t * (vx / v))))); 
            prevmode = Mode;
        } else if (Mode == fleece) {
            t = (double)timer.read_us() * 0.000001;
            v = 37.587665;
            if (sin(2.0 * 3.1415926 * 130.0 * t * (vx / v)) > 0.0)
                AD = DAAD((short)amp * 2);
            else
                AD = DAAD((short)0);
            prevmode = Mode;
        } else if (Mode == microfiber) {
            t = (double)timer.read_us() * 0.000001;
            v = 37.022014;
            if (sin(2.0 * 3.1415926 * 120.0 * t * (vx / v)) > 0.995)
                delta_flag = true;
            else
                delta_flag = false;
            if (delta_cnt == 8) {
                delta_cnt = 0;
                AD = DAAD((short)0);
            }
            if (delta_cnt == 7) {
                delta_cnt = 8;
            }
            if (delta_cnt == 6) {
                delta_cnt = 7;
            }
            if (delta_cnt == 5) {
                delta_cnt = 6;
            }
            if (delta_cnt == 4) {
                delta_cnt = 5;
            }
            if (delta_cnt == 3) {
                delta_cnt = 4;
            }
            if (delta_cnt == 2) {
                delta_cnt = 3;
            }
            if (delta_cnt == 1) {
                delta_cnt = 2;
            }
            if (delta_oldflag == false && delta_flag == true) {
                delta_cnt = 1;
                AD = DAAD((short) amp * 2);
            }
            delta_oldflag = delta_flag;
            prevmode = Mode;
        } else if (Mode == organdy) {
            t = (double)timer.read_us() * 0.000001;
            v = 56.202903;
            if (sin(2.0 * 3.1415926 * 180.0 * t * (vx / v)) > 0.995)
                delta_flag = true;
            else
                delta_flag = false;
            if (delta_cnt == 8) {
                delta_cnt = 0;
                AD = DAAD((short)0);
            }
            if (delta_cnt == 7) {
                delta_cnt = 8;
            }
            if (delta_cnt == 6) {
                delta_cnt = 7;
            }
            if (delta_cnt == 5) {
                delta_cnt = 6;
            }
            if (delta_cnt == 4) {
                delta_cnt = 5;
            }
            if (delta_cnt == 3) {
                delta_cnt = 4;
            }
            if (delta_cnt == 2) {
                delta_cnt = 3;
            }
            if (delta_cnt == 1) {
                delta_cnt = 2;
            }
            if (delta_oldflag == false && delta_flag == true) {
                delta_cnt = 1;
                AD = DAAD((short) amp * 2);
            }
            delta_oldflag = delta_flag;
            prevmode = Mode;
        }
    }
}
