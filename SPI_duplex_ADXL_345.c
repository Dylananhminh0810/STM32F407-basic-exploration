#include <stdint.h>
#include <stdbool.h>
#include "Register.h"
#include "RCC.h"
#include "GPIO.h"
#include "NVIC.h"
#include "EXTI.h"
#include "SYSCFG.h"
#include "Timer.h"
#include "DMA.h"
#include "UART.h"
#include "AXDL345.h"
#include "SPI.h"


static void initGPIOforSPI(void){
//Open data sheet
// pins PA4 - PA7 & AF5 for SPI1 
	GPIOGeneralRegister* GPIOA;
	GPIOA = (GPIOGeneralRegister*)GPIOA_BASE_ADDRESS;
	REG32(RCC_BASE_ADDRESS + RCC_AHB1_OFFSET) |= BIT_0;
	//set pin PA4 - PA7 to alternate function
	GPIOA->MODER |= (BIT_9 | BIT_11 | BIT_13 | BIT_15);
	//output push pull
	GPIOA->OTYPER &= ~((unsigned int)(BIT_4 | BIT_5 | BIT_6 | BIT_7));
	// because SPI is high speed communication -> we set very high speed in the output speed reg
	GPIOA->OSPEEDR |= ((BIT_8 | BIT_9) | (BIT_10 | BIT_11) | (BIT_12 | BIT_13) | (BIT_14 | BIT_15));
	//In the sensor specification, chip select and clock being high at first -> we will select chip select & clock as pull up
	// master in master out as pull down
	GPIOA->PUPDR |= (BIT_8 | BIT_10 | BIT_13 | BIT_15);
	//Set Alternate function 5 for 4 pins PA4 - PA7
	GPIOA->AFRL  |= ((BIT_16 | BIT_18) | (BIT_20 | BIT_22) | (BIT_24 | BIT_26) | (BIT_28 | BIT_30));
}
static void initSPI_REG4(void){
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*)SPI1_;
	REG32(RCC_BASE_ADDRESS + RCC_APB2_OFFSET) |= BIT_12;
	
	//STM32 is the master device.
	//sensor is the slave device.
	//set up informations are in page 882 of the manual
	
	//The sensor maximum SPI speed is 5MHz
	//The frequency of STM32 is 16MHz 
	// -> the frequency needs to be less than 5 MHz
	// -> set baudrate control bits to 1MHz -> 1/16 
	// set this inside CR1 reg
	SPI->SPI_CR1 |= (BIT_3 | BIT_4);
	
	//Next is the clock polarity and clock phase
	//Can find that on page 878 and 879
	// The clock idle state can be of high value as presented up, or low value as presented down - CPOL
	// So when communication starts, the clock cycles will present so 10101010... or 01010101...
	//With the phase, we decide the edge on which we will detect or send data
	//It can be rising or falling depending on the polarity we've chosen
	//If we choose first edge, the data will be read or sent on the falling edge of high polarity or rising edge on low polarity
	//If we choose second edge, the data will be read or sent on the rising edge of high polarity or falling edge on low polarity
	
	//In sensor datasheet it say that data should be sampled at the rising edge of SCLK
	//And also the clock in sensor datasheet starts with high value
	// -> we need to set up accordingly	
	SPI->SPI_CR1 |= (BIT_1 | BIT_0);
	
	//Next need to set up the data frame format (datasheet page 879)
	//we can send most significant bit or least significant bit first
	//And we can choose 8 bits or 16 bits data format
	//From the sensor data sheet, we can see that the data is sent as 8 bits format
	SPI->SPI_CR1 &= ~((unsigned int)BIT_11);
	
	//Because in the sensor manual we can see that sequence of sending data goes most significant bit first
	//Inside the control reg 1 the least significant bit first
	//-> we will choose most significant bit transmitted first
	SPI->SPI_CR1 &= ~((unsigned int)BIT_7);
	
	//Next the NSS, slave select pin will be set as output because we want to select the sensor and then send data to it
	//-> enable SSOE bit
	//This step is not required when the TI mode is selected
	//We won't choose that mode
	// the SSOE bit will be enabled inside the cr2 reg and we will choose to enable it as output
	SPI->SPI_CR2 |= BIT_2;
	
	//Now we need to select correct frame format
	//By enabling the TI protocol, the clock polarity and phase are forced to conform to TI protocol disregards what we enter in the cr1 reg
	// But we are not using the same configuration as this protocol -> we will not enable it
	//-> inside CR2 reg we will keep SPI motorola mode
	SPI->SPI_CR2 &= ~((unsigned int)BIT_4);
	
	//And because STM32 will be a master device, we will need to enable the master bit and SPI Enable bits
	//both are inside cr1 reg
	SPI->SPI_CR1 |= BIT_2;
	SPI->SPI_CR1 |= BIT_6;
	
	// The schematic is somewhat similar to the one in Uart
	//We have cr1 where we can set up multiple things
	//The baud rate using baud rate reg bits
	//The clock polarity and phase
	//The data frame format 8 or 16 bit
	// Last bit first, which bit be transmitted
	//Master selection 
	//and SPI enable bit
	//In cr2. we can set up interrupts for sending and receiving data
	//Slave select, depending on the device being a master or slave device
	//The chip select pin will be output or input pin
	// and frame format reg, which is on the schematic set as 0
	//There is also the status reg, which allows us to see which flag are and are not set
	// Most important one are transmit flag, receive flag and busy flag
	//After setting the control reg, we can send and receive data
	//We can write to or read from the data reg which is 16 bits in size
	//The data reg is divided into the transmit buffer and receive buffer
	//When transmitting, the values from the transmit buffer are copied to the shift reg, after which they are sent to the MOSI or MISO
	//Once data is copied from transmit buffer to the shift reg, the TXE flag gets set up, which means
	//that the transmit buffer is empty and next data can be loaded into the buffer
	//When reading data is stored to the shift reg and then to receive buffer, after which the RXNE flag is set and the data can be read
	//When RXNE flag is set, it indicates that the receive buffer is not empty and the data is ready to be read
	//When the busy flag is set, it means that the SPI is busy communicating
	//The busy flag is useful to detect the enter of a transfer
	//If the software wants to disable the SPI
	
	//Full duplex - 4 wires
	// When devices are communicating using spi full duplex mode, the sending and receiving of data is done simultaneously
	// -> meaning when data is transmitted from the master to the slave, data is also received from the slave to the master bit by bit until all is transmitted
	//PA4 - CS
	//PA5 - SCL
	//PA6 - SDO
	//PA7 - SDA
	//the SDI is the data input into the sensor - MOSI
	//the SDO is the output data into the sensor - MISO
	//Once chip select (SC) goes down, there is some time delay until the clock starts
	// The clock is high polarity and the phase is on rising edge
	
	//we need to check is the data format
	//First comes the address bit, so the addresses we've set up in our ADXL header
	//The address bits are six bits in size and take the positions from bit 0 to bit 5
	//Bit 6 is the multiple byte bit (MB), details of MB can found in page 8 of sensor manual
	// in short, after each set of bytes are sent or received the addressing increments
	//So if we start with a certain address, it will increment and data will be read or written until chip select is pulled high again
	// And last bit - bit 7 is write or read bit, which indicates is the request to the certain address a write or read request
	//The address part are same both for writing and reading
	//The different will be data part
	//When writing, we are sending data to the sensor right after the address byte
	//when reading, we are receiving the data from the sensor right after the address byte
	
	//Because when connecting the sensor to stm32, the sensor will be in standby mode and we need to wake it up as it is written inside the datasheet
	//So after the sensor was powered up, we can write data to it
	//The address will be from power control reg, because we want to wake up the sensor, meaning put it in measure mode
	//The data that is received is marked with X's
	//This need to be taken into consideration when writing the code for write request
	
	SPI->SPI_DR = 0x2D; // we want to write to this address and not use MB, 0x2D is POWER_CTL reg of sensor
	//After we have written to the data reg, we need to wait for the transmission to finish
	//because inside the status reg there is a transmit buffer empty bit TXE
	//it is set up when the transmit buffer is empty
	while (!(SPI->SPI_SR & BIT_1)){} // the buffer is not empty then wait
	//after send address we send data
	//Waking up the sensor and setting it to measure mode is done by writing 1 into bit 3 
	SPI->SPI_DR = 0x08;
	//and again we need to wait till the data is sent and the transfer buffer is empty
	while (!(SPI->SPI_SR & BIT_1)){} // the buffer is not empty then wait
	//after sending this data, the sensor is configured and we can disable the chip select by disabling the SPI
	//In the manual of stm32 on page 877, we can see the chip select or slave select pin management
	//We have disabled the software slave management and enabled the slave select output enable
	//In this configuration, the communication starts, the chip select pin is pulled down and it stays down until the SPI is disabled
	SPI->SPI_CR1 &= ~((unsigned int) BIT_6);
}


static void SPI_Transmit4(uint8_t *data, int size){
//function with data want to send and size as the number of data bytes to send
//Create a loop and go through the data array and send data using same logic	
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	
	int k = 0;
	while (k < size){
		while (!(SPI->SPI_SR & BIT_1)) {}
		SPI->SPI_DR = data[k];
		k++;
	}
	while (!(SPI->SPI_SR & BIT_1)) {}
	while (SPI->SPI_SR & BIT_7){} 
	// clear the receiver reg to clear the overrun flag
	uint32_t temp = SPI->SPI_DR;
	//This function sends certain amount of data from an array to the sensor
}

static void SPI_Receive4(uint8_t *data, int size){
//This function will read data received by the receive buffer and store it
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	//First we need to wait the previous communication to end
	while (SPI->SPI_SR & BIT_7){}
	while (size){
		SPI->SPI_DR = (0x00);
		while (!(SPI->SPI_SR & BIT_0)){}
		*data++ = SPI->SPI_DR;
		size--;
	}
	//When reading data from the sensor
	//data being received from the sensor but also sent to the sensor 
	//this needs to be taken into consideration
	//So first we will send dummy data to the sensor
	//and wait until data is received inside the receive buffer by checking the receive flag inside the status reg
	//Now we will just read the data that was received to the receive buffer
}
void adxl_read4(uint8_t address, uint8_t* RxData, uint8_t dataSize){

	//before any SPI communication we need to enable the SPI
	SerialPeripheralInterface* SPI;
	SPI = (SerialPeripheralInterface*) SPI1_;
	
	address |= 0x80; //read operation
	address |= 0x40; // MB
	
	SPI->SPI_CR1 |= BIT_6;
	//We will also add the MB but as a comment so that we can enable and test it later
	
	SPI_Transmit4(&address, 1);
	SPI_Receive4(RxData, dataSize);
	//Because we transmitted a sensor address with read request
	// In the next step we will receive data we asked for
	//So we will use second function to store the data to the appropriate array
	//And in the end we will disable the SPI because we saw in the STM32 manual it sets chip select pin high
	SPI->SPI_CR1 &= ~((unsigned int)BIT_6);
}

//static uint8_t RxData;
static uint8_t RxData[6];
static volatile int16_t x = 0xFFFF, y = 0xFFFF, z = 0xFFFF;

int main(){
	initGPIOforSPI();

	initSPI_REG4(); // the 4 indicates the number of wires used inside the communication
	//read device id
	//adxl_read4(DEVID, &RxData, sizeof(RxData)); //->RxData = 0xE5
	//in reg map 0x32 -> 0x37 in hex we see the data output for different axes - we have 2 bytes per axis because resolution is ten bits and they cant be covered by one byte
	//So by combining 2 reg values, we can get the ten bit value
	//And on page 18 we can see the details of the reg data
	//The first reg contains the least and second reg the most significant byte
	//So lets create variables that will save the converted data from regs
	//Now lets read the values from the sensor using these variables
	static volatile double xg, yg, zg;
	
	while(1){
		//adxl_read4(0x32, &RxData, sizeof(RxData));
		//x = RxData;
		//adxl_read4(0x33, &RxData, sizeof(RxData));
		//x += (RxData<<8);
		
		//adxl_read4(0x34, &RxData, sizeof(RxData));
		//y = RxData;
		//adxl_read4(0x35, &RxData, sizeof(RxData));
		//y += (RxData<<8);
		
		//adxl_read4(0x36, &RxData, sizeof(RxData));
		//z = RxData;
		//adxl_read4(0x37, &RxData, sizeof(RxData));
		//z += (RxData<<8);
		//in the sensor manual page 8, we can see that the sensor range by default is +-2g
		//this means for a spectre of ten bits value
		//so from 0 to 1023 we have a distribution from -2g to 2g
		//the gravity is roughly 9.8 - g~9.8m/s^2
		//so the distribution is from -19.6 to 19.6 meters per second squared
		//The range which is covered is 39.2
		//So one bit covers roughly a difference of 39.2 divided by 1024, which is roughly equal to 0.038
		//So to get the correct value, we just need to multiply the values which we received from the regs with this number
		adxl_read4(0x32, RxData, sizeof(RxData));
		x = (RxData[0]);
		x += (RxData[1]<<8);
		
		y = RxData[2];
		y += (RxData[3]<<8);
		
		z = RxData[4];
		z += (RxData[5]<<8);
		
		xg = 0.038 * x;
		yg = 0.038 * y;
		zg = 0.038 * z;
	}
	
	return 0;
} 

