// sensor_manager.cpp
#include "sensor_manager.h"

SensorArray::SensorArray(uint8_t trigger, uint8_t echo, unsigned int maxDist) 
    : _sonar(trigger, echo, maxDist) {}

float SensorArray::getFilteredDistance() {
    const int samples = 5;
    float distances[samples];
    
    for(int i=0; i<samples; i++) {
        distances[i] = _sonar.ping_cm();
        delay(30);
    }
    
    // Implementação de filtro de mediana
    for(int i=0; i<samples-1; i++) {
        for(int j=i+1; j<samples; j++) {
            if(distances[j] < distances[i]) {
                float temp = distances[i];
                distances[i] = distances[j];
                distances[j] = temp;
            }
        }
    }
    return distances[samples/2];
}