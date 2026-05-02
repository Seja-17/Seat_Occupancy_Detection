<img width="596" height="454" alt="image" src="https://github.com/user-attachments/assets/8a3a4ebe-515f-475c-a8ad-e52bee371ff8" /># Seat Occupancy Detection


## Overview


An IoT-based system that detects whether a desk or seat in a classroom is occupied using a combination of **touch sensors** and **ultrasonic sensors**, connected via **MQTT** and visualized through a **Node-RED dashboard**.

---

## System Architecture

| Parameter | Details |
|-----------|---------|
| **Sensors** | Touch (seat surface) + Ultrasonic (person distance) |
| **Protocol** | MQTT |
| **Dashboard** | Node-RED |
| **Place** | One Classroom |

---

## Objective

When a person touches the desk or sits down, the touch pad detects occupancy. An ultrasonic sensor then confirms physical presence to avoid false triggers.

---

## Hardware Setup

<img width="248" height="178" alt="Screenshot 2026-05-02 at 11 38 46 AM" src="https://github.com/user-attachments/assets/03a3b379-ccb3-49cb-829c-1047815ec938" />


*The node hardware consists of an ESP32 microcontroller connected to an HC-SR04 ultrasonic sensor (mounted on the desk back panel) and a foil-based capacitive touch pad (on the desk surface). Data is transmitted wirelessly via MQTT.*

**Components per node:**
- Microcontroller (ESP32)
- HC-SR04 Ultrasonic Sensor
- Capacitive Touch Pad (aluminium foil)
- Wi-Fi connectivity

---

## MQTT
<img width="267" height="736" alt="PHOTO-2025-11-10-09-52-08" src="https://github.com/user-attachments/assets/dabbd5ee-5afc-4531-8c3b-bece2d65e6a4" />

---

## Dashboard
The Node-RED flow subscribes to the MQTT topic for each seat, passes incoming sensor data through an **Occupancy Logic** function node to determine presence, and routes the result to a **UI LED indicator**, a **debug node** for monitoring, and a **chart node** for real-time timeline visualization.

<img width="1280" height="627" alt="PHOTO-2025-11-10-09-52-06" src="https://github.com/user-attachments/assets/d0e1b64f-471f-4889-a234-69c4eb05e487" />


### Real-Time Seat Status

<img width="502" height="282" alt="PHOTO-2025-11-10-09-52-12" src="https://github.com/user-attachments/assets/dadddea0-8e37-424d-9ba6-4d9a83f9235f" />

<img width="268" height="169" alt="Screenshot 2026-05-02 at 11 39 30 AM" src="https://github.com/user-attachments/assets/f93dcea4-7c20-4075-a2ba-401d4c87e069" />


*Each circle on the Node-RED dashboard represents one seat. **Green** indicates the seat is **vacant**; **Orange/Red** indicates the seat is **occupied**. The MQTT topic (e.g., `iotb/team2/chair3`) and timestamp are shown on hover.*

### Seat Occupancy Timeline

<img width="218" height="202" alt="Screenshot 2026-05-02 at 11 39 56 AM" src="https://github.com/user-attachments/assets/ca063a64-f6c1-4b3a-a08c-f553366be886" />

*The timeline chart tracks occupancy history for each seat across the session. Signal highs indicate occupied periods; flat lines indicate vacancy. This enables regression-based usage pattern analysis.*

---

## Analytics

### Rule-Based Detection
A seat is marked **occupied** only when **both** sensors agree:
- ✅ Touch sensor detects contact
- ✅ Ultrasonic sensor confirms presence within threshold distance

### Regression Analysis
Historical occupancy data is used to **predict seat usage patterns** throughout the day, enabling smart scheduling and space optimization.

---

## Outcome

**"Smart Study Zone Utilization Dashboard"**

- Per-seat occupancy status (vacant / occupied)
- Historical timeline for all monitored seats
- Predictive analytics for classroom utilization

---

## Tech Stack

| Layer | Technology |
|-------|-----------|
| Sensors | Touch Pad, HC-SR04 Ultrasonic |
| Microcontroller | ESP32 |
| Messaging | MQTT (Mosquitto Broker) |
| Dashboard | Node-RED |
| Analytics | Rule-Based + Regression |


