#include "sensor.h"

int main(void) {
    // Initialise the sensors and set address as Sensor Id
    initialiseSensors();

    uint8_t sensorIndex = 0;
    uint16_t sensorData = 0;
    uint8_t activeSensorAddress = 0;
    uint8_t read_in_progress = 0;

    // Set initial Active sensor
    activeSensorAddress = Set_Active_Sensor(0);
    while (1) {
        // Start a new I2C read if no read is in progress
        if (!Sensor_Read_Complete()) {
            // Start the read only if it's not already in progress
            if (!read_in_progress) {  
                Sensor_Start_Read(activeSensorAddress);
                read_in_progress = 1;  // Mark the read as in progress
            }
        } else {
            // If read is complete, process the sensor data
            sensorData = Sensor_Get_Data();
            ProcessSensorData(activeSensorAddress, sensorData);
            
            // Output Sensor Data

            // Move to the next sensor
            sensorIndex = (sensorIndex + 1) % NUM_SENSORS;
            activeSensorAddress = Set_Active_Sensor(sensorIndex);
            
        }

        I2C_Receive(activeSensorAddress);  // Continue I2C state machine
        
        // Perform other tasks when waiting for I2C
    }

    return 0;
}