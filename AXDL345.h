#ifndef AXDL345_H
#define AXDL345_H


#define MB 0x40
#define WRITE 0x80

#define X_AXIS_0 0x32
#define X_AXIS_1 0x33
#define Y_AXIS_0 0x34
#define Y_AXIS_1 0x35
#define Z_AXIS_0 0x36
#define Z_AXIS_1 0x37

#define POWER_CTL 0x2D
#define DATA_FORMAT 0x31 // 0 - 4 wires, 1 - 3 wires
#define DEVID 0x00 // reading returns 0xE5 if communication is successful

#endif