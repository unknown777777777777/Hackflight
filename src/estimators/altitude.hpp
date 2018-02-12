/* 
    altitude.hpp: Altitude estimation via barometer/accelerometer fusion

    Adapted from

    https://github.com/multiwii/baseflight/blob/master/src/imu.c

    This file is part of Hackflight.

    Hackflight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Hackflight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with EM7180.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "filter.hpp"
#include "sensors/barometer.hpp"
#include "sensors/imu.hpp"
#include "debug.hpp"
#include "datatypes.hpp"

namespace hf {

    class AltitudeEstimator {

        private: 

            // Complementry filter for accel/baro
            const float    cfAlt  = 0.965f;
            const float    cfVel  = 0.985f;

            // Barometer
            Barometer baro;

            // IMU
            IMU imu;

        public:

            void init(uint16_t imuAccel1G)
            {
                baro.init();
                imu.init(imuAccel1G);
            }

            void updateAccel(int16_t accel[3], uint32_t currentTime)
            {
                imu.updateAccel(accel, currentTime);
            }

            void updateGyro(float gyro[3], uint32_t currentTime)
            {
                imu.updateGyro(gyro, currentTime);
            }

            void updateBaro(float pressure)
            {
                baro.update(pressure);
            }

            void estimate(vehicle_state_t & state, uint32_t currentTime)
            {  
                // Calibrate baro AGL at rest
                if (!state.armed) {
                    baro.calibrate();
                    return;
                }

                // Get estimated altitude from barometer
                state.position.values[2] = baro.getAltitude();

                //Debug::printf("%f\n", state.position.values[2]);

                // Apply complementary Filter to keep the calculated velocity based on baro velocity (i.e. near real velocity). 
                // By using CF it's possible to correct the drift of integrated accelerometer velocity without loosing the phase, 
                // i.e without delay.
                float imuVel = imu.getVerticalVelocity();

                //Debug::printf("%+2.2f\n", imuVel);

                float baroVel = baro.getVelocity(currentTime);
                
                state.position.derivs[2] = Filter::complementary(imuVel, (float)baroVel, cfVel);
            }

    }; // class AltitudeEstimator

} // namespace hf
