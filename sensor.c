#include "sensor.h"
#include <stddef.h>
#include <unistd.h> 

uint8_t sensor_Addresses[NUM_SENSORS] = {0x10, 0x1A, 0x1E, 0x2A}; // 7 bit address
Sensor sensor[NUM_SENSORS];
// Mock I2C Control Registers
volatile uint8_t *I2C_CONTROL = (uint8_t *)0x4000;
volatile uint8_t *I2C_DATA = (uint8_t *)0x4001;
volatile uint8_t *I2C_STATUS = (uint8_t *)0x4002;

const uint8_t I2C_START = 0x01; // Start Flag
const uint8_t I2C_STOP = 0x02; // Stop Flag
const uint8_t I2C_ACK = 0x04; // Acknowledge Flag
const uint8_t I2C_READY = 0x08; // I2C Ready Flag
static uint16_t i2c_data_buffer = 0; // data buffer to store sensor data
static uint8_t i2c_ready_flag = 0; // flag for I2C operation complete

// Initialise the Sensors
void InitialiseSensors() {
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        sensor[i].id = sensor_Addresses[i];
        sensor[i].isActive = 0;
        sensor[i].maxTemperature = MAX_OPERATING_TEMP;
        sensor[i].minTemperature = MIN_OPERATING_TEMP;
        sensor[i].averageTemperature = MIN_OPERATING_TEMP;
        sensor[i].buffer_index = 0;
    }
}

// Process the sensor Data
void ProcessSensorData(uint8_t sensorAddress, uint16_t sensorTemperatureData){
    int16_t temperature = (sensorTemperatureData * TEMP_SCALE) + MIN_OPERATING_TEMP;

    // Find the sensor in the array with matching ID
    Sensor* sensor = NULL;
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (sensor[i].id == sensorAddress) {
            sensor = &sensor[i];
            break;
        }
    }

    // If no matching sensor was found, return early
    if (sensor == NULL) {
        return;
    }

    // If the sensor is not active, initialize it
    if (!sensor->isActive) {
        // Set all entries in the buffer to the first reading (initialize buffer)
        for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
            sensor->buffer[i] = temperature;
        }
        sensor->isActive = 1;  // Mark the sensor as active
        sensor->buffer_index = 0;  // Reset buffer index to 0 for the first reading
    } else {
        // Update the buffer with the current reading, replacing the oldest value
        sensor->buffer[sensor->buffer_index] = temperature;
        // Progress the buffer index circularly
        sensor->buffer_index = (sensor->buffer_index + 1) % BUFFER_SIZE;
    }

    // Update min and max
    if (temperature < sensor->minTemperature) sensor->minTemperature = temperature;
    if (temperature > sensor->maxTemperature) sensor->maxTemperature = temperature;

    // Only average temperature values added to the buffer. Once the buffer is full use all values
    short bufferSum = 0;
    for (uint8_t i = 0; i < BUFFER_SIZE; i++) {
        bufferSum += sensor->buffer[i];
    }

    sensor->averageTemperature = bufferSum / BUFFER_SIZE;
}

// State machine for I2C Communication
void I2C_Receive(uint8_t address) {
    static enum { IDLE, START, SEND_ADDRESS, READ_MSB, READ_LSB, STOP } state = IDLE;
    static uint8_t msb = 0; // temporary data 

    switch (state) {
        case IDLE:
            *I2C_CONTROL = I2C_START;  // Send start condition
            state = START;
            break;

        case START:
            if (*I2C_STATUS & I2C_READY) {
                *I2C_DATA = (address << 1) | 0x01;  // Send address + read bit (R/W = 1)
                state = SEND_ADDRESS;
            }
            break;

        case SEND_ADDRESS:
            if (*I2C_STATUS & I2C_READY) {
                *I2C_CONTROL = I2C_ACK;  // Send ACK to receive MSB
                state = READ_MSB;
            }
            break;

        case READ_MSB:
            if (*I2C_STATUS & I2C_READY) {
                msb = *I2C_DATA;  // Store MSB
                *I2C_CONTROL = I2C_ACK;  // Send ACK to receive LSB
                state = READ_LSB;
            }
            break;

        case READ_LSB:
            if (*I2C_STATUS & I2C_READY) {
                i2c_data_buffer = (msb << 8) | *I2C_DATA;  // Combine MSB and LSB
                *I2C_CONTROL = I2C_STOP;  // Send stop condition
                state = STOP;
            }
            break;

        case STOP:
            if (*I2C_STATUS & I2C_READY) {
                i2c_ready_flag = 1;  // Set ready flag
                state = IDLE;  // Reset state machine
            }
            break;

        default:
            state = IDLE;  // Reset state on unexpected behavior
            break;
    }
}

// Function to start a sensor read operation
void Sensor_Start_Read(uint8_t sensorAddress){
    i2c_ready_flag = 0; // Reset the ready flag
    I2C_Receive(sensorAddress); // Start I2C Communication
}

// Function to check I2C readyFlag
uint8_t Sensor_Read_Complete(void) {
    return i2c_ready_flag;
}
// Function to retrieve the sensor data 
uint16_t Sensor_I2C_Data(void){
    return i2c_data_buffer;
}
// Set next sensor as active sensor 
uint8_t Set_Active_Sensor(uint8_t index){
    return sensor_Addresses[index];
}
