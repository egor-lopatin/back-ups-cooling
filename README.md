# External cooler controller for Back UPS

(Read only and not maintained)

This is a simple script to control an external cooler based on the temperature of the UPS. The script reads the temperature of the UPS and turns on the cooler if the temperature is above a certain threshold. The cooler is turned off when the temperature is below the threshold.

Two strategies were checked:
 - Use brushless motor with PWM control
 - Use a simple relay to turn on/off the cooler

Finally, the relay was chosen because the brushless fan was too noisy.
