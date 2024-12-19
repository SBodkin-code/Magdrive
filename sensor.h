#include <stdint.h>

#define MIN_OPERATING_TEMP -40.0 // Celsius
#define MAX_OPERATING_TEMP 120.0 // Celsius
#define NUM_SENSORS 4
#define INPUT_MAX 0xFFFF
#define BUFFER_SIZE 10
#define TEMP_SCALE ((MAX_OPERATING_TEMP - MIN_OPERATING_TEMP) / INPUT_MAX) // Conversion scale BITS -> TEMP

typedef struct {
	uint8_t id; // Sensor ID / ADC channel
	uint8_t isActive; // sensor active toggle
	int16_t minTemperature; // Min Temperature
	uint16_t maxTemperature; // Max Temperature
	uint16_t averageTemperature; // Average Temperature
	uint8_t buffer[BUFFER_SIZE];  // Circular buffer to store readings
    uint8_t buffer_index;        // Current index in the buffer
} Sensor;

// Function to Initalise Sensors
void InitialiseSensors();

// Function to Process Sensor Data
void ProcessSensorData(uint8_t sensorAddress, uint16_t sensorTemperatureData);

// Function to read and process data from all sensors
void Read_Process_Sensors();

// Function to receive 16-bit data from a sensor
void I2C_Receive(uint8_t address);

// Function to check I2C readyFlag
uint8_t Sensor_Read_Complete(void);

// Function to retrieve the sensor data 
uint16_t Sensor_I2C_Data(void);

// Function to Set active sensor 
uint8_t Set_Active_Sensor(uint8_t index);
