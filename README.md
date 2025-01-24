# Pill Box - Smart Medication Reminder System

Pill Box is an intelligent medication reminder system developed using ESP32S3 microcontroller, Bluetooth, real-time clock HT1380, AHT20 temperature and humidity sensor, accelerometer for motion sensing, 595 shift register, HT16C23A LCD driver, LED, and magnetic switch. This system helps users manage their medication schedules effectively by providing timely reminders and tracking their pill intake.

## Features

- **Medication Schedule Management:** Users can set up their medication schedules, including dosage and timing.
- **Reminder System:** Pill Box sends notifications and alerts based on the user's medication schedule, ensuring timely     pill intake.
- **Temperature and Humidity Monitoring:** AHT20 sensor enables monitoring of the environment's temperature and humidity to ensure proper medication storage conditions.
- **Motion Sensing:** Utilizing an accelerometer, Pill Box can detect motion to ensure the user is awake and attentive before issuing reminders.
- **Bluetooth Connectivity:** Enables users to interact with the Pill Box system through a mobile application for additional features and customization. Seamless communication between the Pill Box system and the mobile application via Bluetooth, enabling configuration changes and data synchronization.
- **Firmware Upgrade Functionality:** Allows for easy firmware updates over Bluetooth, ensuring the system stays up-to-date with the latest features and improvements.
- **LCD Display:** HT16C23A LCD driver displays medication reminders, time, and environmental conditions for easy viewing.
- **LED Indicator:** Provides visual feedback on system status and alerts.
- **Magnetic Switch:** Detects the opening of the ```Pill-Box``` to trigger reminders and monitor medication intake.
- **User Behavior Reporting:** Generates reports for understanding user behavior and adherence to medication schedules, helping users and healthcare providers make informed decisions.

## Hardware Components

- ESP32S3 Microcontroller
- Real-time Clock Module (HT1380)
- Temperature and Humidity Sensor (AHT20)
- Accelerometer
- 595 Shift Register
- LCD Driver (HT16C23A)
- LED
- Magnetic Switch

## Installation and Usage

1. **Setting up the Hardware:**
   - Connect all the hardware components according to the provided schematics.
   - Ensure proper connections and power supply for the ESP32S3 microcontroller.

2. **Uploading Firmware:**
   - Upload the firmware provided in the `firmware/` directory to the ESP32S3 microcontroller using the ESP IDF or compatible software.

3. **Customization:**
   - Modify the firmware as needed to adjust medication schedules, reminder intervals, and other settings.
   - Implement firmware upgrade functionality to support over-the-air updates via Bluetooth.

4. **Integration with Mobile Application:**
   - Develop or integrate a mobile application to interact with the Pill Box system via Bluetooth.
   - Enable bi-directional communication for configuration changes, firmware updates, and data synchronization.

5. **Testing and Deployment:**
   - Test the Pill Box system thoroughly to ensure proper functionality and reliability.
   - Deploy the system in the desired location for medication management.

## Contributing

Contributions to the Pill Box project are welcome! Whether it's bug fixes, new features, or enhancements, feel free to contribute by opening issues or submitting pull requests.

## License

The Pill Box project is licensed under the [HES].


