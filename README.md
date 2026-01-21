# Bot Control via Gemini AI

A network-based robotic control system that uses **Google Gemini AI** to interpret natural language commands and convert them into real-time movement instructions for a robot using UDP/WebSocket communication.

This project enables voice or text driven control of embedded robotic platforms such as Arduino Uno R4 based motor systems.

---

## Table of Contents

- Overview
- Features
- System Architecture
- Project Structure
- Software
- Python Dependencies
- Environment Configuration
- Command Interface
- Robot Communication Protocol
- Hardware Compatibility
- Limitations

---

## Overview

The system accepts user commands through text or speech, processes them using Gemini AI for intent extraction, and sends low-level control signals to a robot over the local network. It supports both desktop and mobile clients. The Gemini Agent can move your robot or answer your queries using inbuilt tools.

---

## Features

- Natural language command parsing using Faster-Whisper
- Real-time Speech-to-Text (STT)  
- UDP and WebSocket communication support 
- Mobile client compatibility (Expo / React Native)  
- Timed and directional movement control  

---

## System Architecture

User Input (Text / Voice)  
→ Speech-to-Text (Faster Whisper)  
→ Gemini AI (Intent Parsing)  
→ Python Control Server  
→ UDP / WebSocket Network  
→ Arduino Uno R4  
→ Motor Driver → Motors

---

## Project Structure

. <br>
├── BotController        # Main AI agent and control server <br>
├── HTTPServer           # If you want to make Arduino an http server to connect with <br>
├── PCController         # Control for directly using keyboard for the rover <br>
├── UDPReciever          # UDp Receiver for Arduino Uno R4 <br>
└── README.md <br>

### Software

- Python 3.9 or higher  
- Google Gemini API access  

### Python Dependencies

Install all required packages:

pip install -r requirements.txt

## Environment Configuration

Create a .env file in the project root:

API_KEY=YOUR_API_KEY_HERE

Or export manually:

export GEMINI_API_KEY=YOUR_API_KEY_HERE

---

## Command Interface

Users may issue natural language commands such as:

- Move forward  
- Turn left for two seconds  
- Go back slowly 

Gemini AI extracts intent and parameters and converts them into structured robot commands.

---

## Robot Communication Protocol

Example UDP packet format:

move$dir=fwd/  
move$dir=left/  
move$dir=right/
command$param={paramValue}

For timed motion, packets are sent repeatedly at fixed intervals (e.g., every 200 ms) for the requested duration.

---

## Hardware
- Arduino Uno R4 WiFi  
- L298N  

---

## Limitations

- Arduino Uno R4 does not support video streaming  
- Real-time STT may require GPU acceleration  
- No closed-loop feedback control (open-loop movement only)  
- Network latency can affect precision
