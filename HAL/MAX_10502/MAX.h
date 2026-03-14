#include "../../LIB/BIT_MATH.h"
#include "../../LIB/STD_TYPES.h"

//#define BEAT_THRESHOLD_HIGH  40000  // adjust according to your readings
//#define BEAT_THRESHOLD_LOW   30000
#define BEAT_THRESHOLD_HIGH 140000
#define BEAT_THRESHOLD_LOW  100000

#define MAX30102_ADDR        0x57
#define MAX30102_WRITE_ADDR  ((MAX30102_ADDR << 1) | 0)  // 0xAE
#define MAX30102_READ_ADDR   ((MAX30102_ADDR << 1) | 1)  // 0xAF
#define RATE_SIZE 10


void MAX30102_Init(void);
u32 MAX30102_ReadIR(void);
u8 checkForBeat(u32 irValue);
f32 removeDC(u32 irValue);
void update_display(u8 avgBPM, u32 rawIR);
