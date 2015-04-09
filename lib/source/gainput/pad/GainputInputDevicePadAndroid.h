
#ifndef GAINPUTINPUTDEVICEPADANDROID_H_
#define GAINPUTINPUTDEVICEPADANDROID_H_

#include <android/sensor.h>

namespace gainput
{

class InputDevicePadImplAndroid : public InputDevicePadImpl
{
public:
	InputDevicePadImplAndroid(InputManager& manager, InputDevice& device, unsigned index, InputState& state, InputState& previousState) :
		manager_(manager),
		device_(device),
		state_(state),
		previousState_(previousState),
		deviceState_(InputDevice::DS_UNAVAILABLE),
		buttonDialect_(manager_.GetAllocator()),
		sensorManager_(0),
		accelerometerSensor_(0),
		gyroscopeSensor_(0),
		sensorEventQueue_(0)
	{
		ALooper* looper = ALooper_forThread();
		if (!looper)
		{
			looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
		}

		if (!looper)
		{
			return;
		}

		sensorManager_ = ASensorManager_getInstance();
		if (!sensorManager_)
		{
			return;
		}

		sensorEventQueue_ = ASensorManager_createEventQueue(sensorManager_, looper, ALOOPER_POLL_CALLBACK, NULL, NULL);
		if (!sensorEventQueue_)
		{
			return;
		}

		accelerometerSensor_ = ASensorManager_getDefaultSensor(sensorManager_, ASENSOR_TYPE_ACCELEROMETER);
		if (accelerometerSensor_)
		{
			ASensorEventQueue_setEventRate(sensorEventQueue_, accelerometerSensor_, ASensor_getMinDelay(accelerometerSensor_));
			ASensorEventQueue_enableSensor(sensorEventQueue_, accelerometerSensor_);
		}

		gyroscopeSensor_ = ASensorManager_getDefaultSensor(sensorManager_, ASENSOR_TYPE_GYROSCOPE);
		if (gyroscopeSensor_)
		{
			ASensorEventQueue_setEventRate(sensorEventQueue_, gyroscopeSensor_, ASensor_getMinDelay(gyroscopeSensor_));
			ASensorEventQueue_enableSensor(sensorEventQueue_, gyroscopeSensor_);
		}

		magneticFieldSensor_ = ASensorManager_getDefaultSensor(sensorManager_, ASENSOR_TYPE_MAGNETIC_FIELD);
		if (magneticFieldSensor_)
		{
			ASensorEventQueue_setEventRate(sensorEventQueue_, magneticFieldSensor_, ASensor_getMinDelay(magneticFieldSensor_));
			ASensorEventQueue_enableSensor(sensorEventQueue_, magneticFieldSensor_);
		}

		deviceState_ = InputDevice::DS_OK;
	}

	~InputDevicePadImplAndroid()
	{
		if (!sensorManager_)
		{
			return;
		}

		if (!sensorEventQueue_)
		{
			return;
		}


		if (accelerometerSensor_)
		{
			ASensorEventQueue_disableSensor(sensorEventQueue_, accelerometerSensor_);
		}

		if (gyroscopeSensor_)
		{
			ASensorEventQueue_disableSensor(sensorEventQueue_, gyroscopeSensor_);
		}

		if (magneticFieldSensor_)
		{
			ASensorEventQueue_disableSensor(sensorEventQueue_, magneticFieldSensor_);
		}

		ASensorManager_destroyEventQueue(sensorManager_, sensorEventQueue_);
	}

	InputDevice::DeviceVariant GetVariant() const
	{
		return InputDevice::DV_STANDARD;
	}

	void Update(InputDeltaState* delta)
	{
		ASensorEvent event;

		while (ASensorEventQueue_getEvents(sensorEventQueue_, &event, 1) > 0)
		{
			if (event.type == ASENSOR_TYPE_ACCELEROMETER)
			{
				HandleAxis(device_, state_, delta, PadButtonAccelerationX, event.acceleration.x / ASENSOR_STANDARD_GRAVITY);
				HandleAxis(device_, state_, delta, PadButtonAccelerationY, event.acceleration.y / ASENSOR_STANDARD_GRAVITY);
				HandleAxis(device_, state_, delta, PadButtonAccelerationZ, event.acceleration.z / ASENSOR_STANDARD_GRAVITY);
			}
			else if (event.type == ASENSOR_TYPE_GYROSCOPE)
			{
				HandleAxis(device_, state_, delta, PadButtonGyroscopeX, event.vector.x / ASENSOR_STANDARD_GRAVITY);
				HandleAxis(device_, state_, delta, PadButtonGyroscopeY, event.vector.y / ASENSOR_STANDARD_GRAVITY);
				HandleAxis(device_, state_, delta, PadButtonGyroscopeZ, event.vector.z / ASENSOR_STANDARD_GRAVITY);
			}
			else if (event.type == ASENSOR_TYPE_MAGNETIC_FIELD)
			{
				HandleAxis(device_, state_, delta, PadButtonMagneticFieldX, event.magnetic.x / ASENSOR_MAGNETIC_FIELD_EARTH_MAX);
				HandleAxis(device_, state_, delta, PadButtonMagneticFieldY, event.magnetic.y / ASENSOR_MAGNETIC_FIELD_EARTH_MAX);
				HandleAxis(device_, state_, delta, PadButtonMagneticFieldZ, event.magnetic.z / ASENSOR_MAGNETIC_FIELD_EARTH_MAX);
			}
		}
	}

	InputDevice::DeviceState GetState() const
	{
		return deviceState_;
	}

	bool IsValidButton(DeviceButtonId deviceButton) const
	{
		if (deviceButton >= PadButtonAccelerationX && deviceButton <= PadButtonAccelerationZ && accelerometerSensor_)
		{
			return true;
		}
		
		if (deviceButton >= PadButtonGyroscopeX && deviceButton <= PadButtonGyroscopeZ && gyroscopeSensor_)
		{
			return true;
		}

		if (deviceButton >= PadButtonMagneticFieldX && deviceButton <= PadButtonMagneticFieldZ && magneticFieldSensor_)
		{
			return true;
		}

		return false;
	}

	bool Vibrate(float leftMotor, float rightMotor)
	{
		return false;
	}

private:
	InputManager& manager_;
	InputDevice& device_;
	InputState& state_;
	InputState& previousState_;
	InputDevice::DeviceState deviceState_;
	HashMap<unsigned, DeviceButtonId> buttonDialect_;
	ASensorManager* sensorManager_;
	const ASensor* accelerometerSensor_;
	const ASensor* gyroscopeSensor_;
	const ASensor* magneticFieldSensor_;
	ASensorEventQueue* sensorEventQueue_;
};

}

#endif

