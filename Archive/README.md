# Smart Roll Call App

This project implements a user-friendly and efficient attendance tracking system for classrooms, leveraging ESP32 microcontrollers, embedded C programming, and IFTTT API integration.

## Project Overview

This application streamlines the attendance marking process for teachers:

1. **Student Registration:** Students register their mobile devices with the system, linking their unique MAC addresses to their roll numbers.
2. **Attendance Marking:** When a student connects their device to the ESP32's Wi-Fi hotspot, their attendance is automatically marked on a Google Sheets file stored securely in the cloud.
3. **ESP32 and Data Transmission:** The ESP32 microcontroller acts as the core, facilitating device identification, Wi-Fi communication, and data transmission to the cloud using the IFTTT API.

## Benefits

* **Simplified Attendance Marking:** Eliminates time-consuming manual roll calls, freeing up valuable classroom time.
* **Enhanced Accuracy:** Reduces the risk of errors from manual attendance taking.
* **Automated Data Storage:** Securely records attendance data in a centralized Google Sheets file for easy access and analysis.
* **Scalability:** The system can accommodate a growing number of students without requiring significant changes.

## Tech Stack

* **Hardware:** ESP32 microcontroller
* **Programming Language:** Embedded C
* **Networking:** Wi-Fi (device connection and ESP32 hotspot)
* **Data Transfer:** IFTTT API
* **Cloud Storage:** Google Sheets

## Getting Started

To set up and use the Smart Roll Call App, you'll need the following:

* **Hardware:**
    * ESP32 microcontroller
* **Software:**
    * Arduino IDE (or compatible IDE for ESP32 programming)
    * IFTTT account
    * Google Sheets account


## Contributing

We welcome contributions to this project! If you have improvements or suggestions, please feel free to submit a pull request.

## License

This project is licensed under the MIT license.

## Contact

For any questions or feedback about the Smart Roll Call App, please feel free to reach out to shubham.01919011722@ipu.ac.in
