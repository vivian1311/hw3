#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "stdlib.h"
#include "math.h"
#include "mbed_events.h"
#define UINT14_MAX        16383
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7
DigitalOut led1(LED1);
InterruptIn sw2(SW2);

EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
Thread thread1;
Thread thread2;

int tilt[100];
float X[100];
float Y[100];
float Z[100];
I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;


float x, y, z;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void FXOS8700CQ_0(){
    pc.baud(115200);
    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;
    float t[3];
    
    FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);
    FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

            FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

            acc16 = (res[0] << 6) | (res[1] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[0] = ((float)acc16) / 4096.0f;

            acc16 = (res[2] << 6) | (res[3] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[1] = ((float)acc16) / 4096.0f;

            acc16 = (res[4] << 6) | (res[5] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[2] = ((float)acc16) / 4096.0f;

            x = t[0];
            y = t[1];
            z = t[2];
}
void FXOS8700CQ(){
    pc.baud(115200);
    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;
    float t[3];

 // while (true) {
        for (int i = 0; i < 100; i++){
            FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

            acc16 = (res[0] << 6) | (res[1] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[0] = ((float)acc16) / 4096.0f;

            acc16 = (res[2] << 6) | (res[3] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[1] = ((float)acc16) / 4096.0f;

            acc16 = (res[4] << 6) | (res[5] >> 2);
            if (acc16 > UINT14_MAX/2)
                acc16 -= UINT14_MAX;
            t[2] = ((float)acc16) / 4096.0f;

            float lenth_1, lenth_2, ans;
            lenth_1 = pow( (pow(t[0], 2) + pow(t[1], 2) + pow(t[2], 2)),0.5);
            lenth_2 = pow((pow(x, 2) + pow(y, 2) + pow(z, 2)), 0.5);
            ans = (t[0]*x + t[1]*y + t[2]*z)/lenth_1*lenth_2;
            if (ans <= 1/pow(2, 0.5)){
                tilt[i] = 1;
            }else{
                tilt[i] = 0;
            }
          /*  printf("FXOS8700Q ACC: X=%1.4f(%x%x) Y=%1.4f(%x%x) Z=%1.4f(%x%x)\r\n",\
                    t[0], res[0], res[1],\
                    t[1], res[2], res[3],\
                    t[2], res[4], res[5]\
            );*/
            X[i] = t[0];
            Y[i] = t[1];
            Z[i] = t[2];
            pc.printf("%1.3f\r\n", t[0]);
            pc.printf("%1.3f\r\n", t[1]);
            pc.printf("%1.3f\r\n", t[2]);
            pc.printf("%d\r\n", tilt[i]);
            wait(0.1);
        }
  //  }
}
void blink(){
    int i = 0;
        while(i < 100){
        led1 = !led1;
        i++;
        wait(0.1);

    }
    
    
}
void btn_fall_irq(){
    queue1.call(&FXOS8700CQ);
    queue2.call(&blink);
}
int sample = 100;
int main() {
    FXOS8700CQ_0();
  /*  while(1){
        pc.printf("%1.3f\r\n", x);
    }*/
    
    thread1.start(callback(&queue1, &EventQueue::dispatch_forever));
    thread2.start(callback(&queue2, &EventQueue::dispatch_forever));
    sw2.rise(queue1.event(btn_fall_irq));
    for (int i = 0; i < 100; i++){
        pc.printf("x = %1.4f\r\n", X[i]);
        pc.printf("y = %1.4f\r\n", Y[i]);
        pc.printf("z = %1.4f\r\n", Z[i]);
        pc.printf("t = %d\r\n", tilt[i]);
    }
   // sw2.rise(queue2.event(blink));
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}