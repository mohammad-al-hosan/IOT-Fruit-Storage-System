# 📦 IoT-Based Smart Fruit Storage System

**Final Year Project — EEE 416, BUET**

This project presents an IoT-enabled smart fruit storage system that automates the control of temperature, humidity, and air quality using embedded sensors and actuators. The system is built using an Arduino Mega for local control and an ESP8266 module for cloud integration via the Arduino IoT Cloud platform.

It is designed to reduce spoilage and maintain optimal storage conditions through environmental sensing, actuator control, and remote monitoring.

---

## ⚙️ Features

- Real-time monitoring of temperature, humidity, and gas levels
- Automatic misting when humidity falls below threshold
- Peltier module with timed ON/OFF cooling cycle
- Ventilation system with servo-controlled inlet and outlet vents
- Gas detection using MQ135 and buzzer alert for poor air quality
- Local LCD display for on-site feedback
- Remote monitoring and manual override via Arduino IoT Cloud
- Serial communication between Arduino Mega and ESP8266

---

## 🔩 Hardware Components

| Component               | Description                                |
|------------------------|--------------------------------------------|
| Arduino Mega 2560      | Main microcontroller for local automation  |
| ESP8266                | IoT module connected to Arduino via UART   |
| DHT22                  | Temperature and humidity sensor            |
| MQ135                  | Gas/air quality sensor                     |
| 2x 2-Channel Relay     | For mist, Peltier, fans, etc.              |
| Peltier Module         | Active cooling component                   |
| Mist Module            | Ultrasonic humidifier                      |
| Servo Motors           | For controlling inlet/outlet vents         |
| I2C LCD (16x2)         | On-device status display                   |
| Buzzer                 | Alerts for poor air quality                |
| 12V 10A SMPS           | Power supply for all components            |

---


## 🌐 Arduino IoT Cloud Integration

Sensor data and actuator states are transmitted from Arduino Mega to the ESP8266 via Serial1. The ESP8266 publishes this data to the Arduino IoT Cloud, where it is visualized and monitored in real time. Manual override switches are also integrated, allowing remote control of actuators such as the Peltier module and ventilation fan.

---

## 🛠️ Getting Started

1. Clone this repository
2. Upload the Arduino sketch to the Arduino Mega
3. Upload the ESP8266 sketch to the ESP module
4. Assemble the hardware according to the provided circuit diagrams
5. Configure Arduino IoT Cloud dashboard with matching variable names and widgets
6. Power the system using a 12V 10A SMPS
7. Monitor and control the system remotely via Arduino IoT Cloud

---

## YouTube Video Link:
   Explanation: https://youtu.be/SsQY30of0jE

## 👥 Authors

- Mohammad Al Hosan
- Md. Muaz Rahman
- Md. Al Amin
- Puspita Mobarak  
  

