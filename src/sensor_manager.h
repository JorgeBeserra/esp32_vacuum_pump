// sensor_manager.h
#include <NewPing.h>

class SensorArray {
public:
    SensorArray(uint8_t trigger, uint8_t echo, unsigned int maxDist);
    float getFilteredDistance();
    
private:
    NewPing _sonar;
    float _medianFilter();
};