# UOA_0025_5-Channel-Air-Pressure-Monitor-and-Logger
5 Channel Air Pressure Monitor and Logger - 18-07-2025
Key Features of the Manual:
	1. Visual Block Diagram - Shows the complete system architecture with all connections
	2. Hardware Specifications - Detailed pin mappings and channel status
	3. Operation Guide - Step-by-step instructions for setup and use
	4. Command Reference - All available serial commands and their functions
	5. Troubleshooting Section - Specific solutions for common issues like: 
		○ Sensor 2 zero reading problems
		○ GPIO4 (CH4) known issues
		○ SD card compatibility requirements
Notable Documentation Features:
	• Color-coded warnings for problematic channels (Sensor 2 and Sensor 5)
	• Performance benchmarks for different sampling rates
	• Data format examples showing both compact and detailed output
	• Safety considerations for electrical connections
	• Built-in diagnostic tools explanation
The manual specifically addresses the two main issues identified in your code:
	1. Sensor 2 (GPIO1) zero readings - Despite being the most reliable ADC pin
	2. Sensor 5 (GPIO4) hardware limitations - Known ESP32-C6 issues
ESP32-C6 Six Channel ADC Logger - User Manual
Overview
The ESP32-C6 Six Channel ADC Logger is an ultra-high-speed data acquisition system capable of sampling up to 6 analog sensors simultaneously at rates up to 1000 samples per second. The system logs data both to serial output and SD card storage with comprehensive diagnostic capabilities.
Block Diagram
┌─────────────────────────────────────────────────────────────────────────┐
│                        ESP32-C6 ADC Logger System                      │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────┐    ┌──────────────────────────────────────────────┐   │
│  │   Sensor 1  │────│ GPIO0 (ADC1_CH0) ─┐                         │   │
│  └─────────────┘    │                    │                         │   │
│                     │                    │                         │   │
│  ┌─────────────┐    │ GPIO1 (ADC1_CH1) ──┤                         │   │
│  │   Sensor 2  │────│  ⚠️ MOST RELIABLE   │                         │   │
│  └─────────────┘    │                    │                         │   │
│                     │                    │                         │   │
│  ┌─────────────┐    │ GPIO2 (ADC1_CH2) ──┤     ESP32-C6            │   │
│  │   Sensor 3  │────│                    │                         │   │
│  └─────────────┘    │                    │   ┌─────────────────┐   │   │
│                     │                    ├───│  ADC1 Unit      │   │   │
│  ┌─────────────┐    │ GPIO3 (ADC1_CH3) ──┤   │  12-bit         │   │   │
│  │   Sensor 4  │────│                    │   │  0-3.3V range   │   │   │
│  └─────────────┘    │                    │   └─────────────────┘   │   │
│                     │                    │                         │   │
│  ┌─────────────┐    │ GPIO4 (ADC1_CH4) ──┤   ┌─────────────────┐   │   │
│  │   Sensor 5  │────│  ⚠️ PROBLEMATIC     │   │  CPU & Memory   │   │   │
│  └─────────────┘    │                    │   │  Data Processing│   │   │
│                     │                    │   └─────────────────┘   │   │
│  ┌─────────────┐    │ GPIO5 (ADC1_CH5) ──┘                         │   │
│  │   Sensor 6  │────│                                              │   │
│  └─────────────┘    └──────────────────────────────────────────────┘   │
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    SD Card Interface                            │   │
│  │  ┌──────────────┐  GPIO18 ──── CS   ┌────────────────────────┐  │   │
│  │  │              │  GPIO19 ──── MOSI │                        │  │   │
│  │  │              │  GPIO20 ──── MISO │      SD Card           │  │   │
│  │  │   SPI Bus    │  GPIO21 ──── SCK  │   (Data Storage)       │  │   │
│  │  │              │                   │                        │  │   │
│  │  └──────────────┘                   └────────────────────────┘  │   │
│  └─────────────────────────────────────────────────────────────────┘   │
│                                                                         │
│  ┌─────────────────────────────────────────────────────────────────┐   │
│  │                    Serial Interface                             │   │
│  │               USB ──── UART ──── Computer                       │   │
│  │            (115200 baud, monitoring & control)                  │   │
│  └─────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘

Hardware Specifications
ADC Channels Configuration
Sensor	GPIO	ADC Channel	Status	Notes
Sensor 1	GPIO0	ADC1_CH0	✅ Reliable	Standard configuration
Sensor 2	GPIO1	ADC1_CH1	⚠️ Zero readings	Most reliable pin, hardware issue suspected
Sensor 3	GPIO2	ADC1_CH2	✅ Reliable	Standard configuration
Sensor 4	GPIO3	ADC1_CH3	✅ Reliable	Standard configuration
Sensor 5	GPIO4	ADC1_CH4	⚠️ Problematic	Known ESP32-C6 hardware limitations
Sensor 6	GPIO5	ADC1_CH5	✅ Reliable	Standard configuration
SD Card Interface
Function	GPIO Pin	Description
CS (Chip Select)	GPIO18	SD card selection
MOSI (Data Out)	GPIO19	Data to SD card
MISO (Data In)	GPIO20	Data from SD card
SCK (Clock)	GPIO21	SPI clock signal
Technical Specifications
	• ADC Resolution: 12-bit (0-4095 counts)
	• Voltage Range: 0-3.3V (adjustable attenuation)
	• Maximum Sample Rate: 1000 samples/second
	• Default Sample Rate: 100 samples/second (10ms interval)
	• Data Storage: CSV format on SD card + Serial output
	• Serial Baud Rate: 115200
Installation & Setup
Hardware Connections
	1. Power Supply: Ensure stable 3.3V power supply
	2. Sensor Connections: Connect analog sensors to respective GPIO pins
	3. SD Card: Insert Class 10 or UHS-I SD card for high-speed logging
	4. USB Connection: Connect to computer for monitoring and control
Software Setup
	1. Arduino IDE: Install ESP32 board package
	2. Libraries Required:
		○ ESP32 ADC driver (driver/adc.h)
		○ SD card library (SD.h)
		○ SPI library (SPI.h)
	3. Upload Code: Flash the provided Arduino sketch to ESP32-C6
Operation Guide
Starting the System
	1. Power On: Connect ESP32-C6 to power/USB
	2. Serial Monitor: Open at 115200 baud rate
	3. Initialization: System will automatically: 
		○ Initialize ADC channels
		○ Test all sensors
		○ Initialize SD card
		○ Create timestamped CSV file
		○ Begin high-speed logging
Sample Rate Control
Use serial commands to adjust logging speed:
Command	Sample Rate	Interval	Use Case
speed1	1/second	1000ms	Slow monitoring
speed10	10/second	100ms	Standard logging
speed100	100/second	10ms	Default - High speed
speed1000	1000/second	1ms	Ultra high speed
Diagnostic Commands
Command	Function	Description
debug	Channel Analysis	Detailed 50-sample analysis per channel
gpio	GPIO Diagnostic	Check GPIO configuration and conflicts
sensor2	Sensor 2 Troubleshooting	Special diagnostic for GPIO1 issues
ch4	CH4 Special Test	Test different configurations for GPIO4
compact	Toggle Output Format	Switch between detailed/compact display
status	System Status	Show current configuration and statistics
Output Formats
Compact Format (High Speed)
12345.678,1234,0,2345,1890,ERROR,3456
Format: timestamp_ms,S1_raw,S2_raw,S3_raw,S4_raw,S5_raw,S6_raw
Detailed Format (Standard)
[00:02:05.678] S1:1234(1.01V) S2:0(0.00V) ⚠️ S3:2345(1.91V) S4:1890(1.54V) S5:ERR S6:3456(2.82V)
Data Files
CSV File Structure
# Six Channel Sensor Data Log - ULTRA HIGH SPEED MODE
# Started: Boot #1
# Uptime at start: 5000 ms
# Logging Rate: 100 samples per second
Timestamp,Uptime_ms,S1_Raw,S2_Raw,S3_Raw,S4_Raw,S5_Raw,S6_Raw,S1_V,S2_V,S3_V,S4_V,S5_V,S6_V
00:00:05.010,5010,1234,0,2345,1890,-1,3456,1.010,0.000,1.912,1.540,-1.000,2.817
File Naming Convention
	• Pattern: /sensors_6ch_log_X.csv
	• X: Sequential number (1, 2, 3, ...)
	• Auto-generated: Prevents overwriting existing files
Troubleshooting
Common Issues
Sensor 2 Reading Zeros
	• Symptom: GPIO1/ADC1_CH1 consistently reads 0
	• Cause: Hardware connection issue (most likely)
	• Solutions: 
		1. Check physical connections to GPIO1
		2. Verify sensor output voltage (0-3.3V range)
		3. Test with known voltage source
		4. Check for short circuits
		5. Measure voltage at GPIO1 with multimeter
Sensor 5 (CH4) Not Working
	• Symptom: GPIO4/ADC1_CH4 errors or inconsistent readings
	• Cause: Known ESP32-C6 hardware limitation
	• Solutions: 
		1. Run ch4 diagnostic command
		2. Try different attenuation settings
		3. Use alternative GPIO pin if available
		4. Consider external ADC for this channel
SD Card Issues
	• Symptoms: "SD Card Mount Failed" messages
	• Solutions: 
		1. Use Class 10 or UHS-I SD card
		2. Check SD card formatting (FAT32)
		3. Verify GPIO connections (18, 19, 20, 21)
		4. Ensure stable power supply
Performance Optimization
High-Speed Logging (≥100Hz)
	• SD Card: Use high-speed cards (Class 10, UHS-I)
	• Power: Ensure stable, clean 3.3V supply
	• Monitoring: Watch free heap memory
	• Format: System auto-switches to compact output
Ultra-High Speed (1000Hz)
	• Limitations: Pushes hardware to maximum
	• Requirements: Premium SD card essential
	• Monitoring: System provides performance feedback
	• Data Rate: ~5.8MB/hour estimated
System Monitoring
Status Information
=== ULTRA HIGH SPEED SYSTEM STATUS ===
Logging Rate: 100 samples/second (10ms interval)
Total Samples: 15000
Uptime: 150000 ms (2.5 minutes)
Free Heap: 245632 bytes
Output Format: COMPACT
Actual Rate: 99.8 samples/second
Data Rate: ~4.8 MB/hour
Working Channels: 4/6
SD Card: 32768MB
Channel Status Summary
	• Working: Channels producing valid readings
	• Problematic: Channels with frequent errors
	• Error Indicators: -1 for failed readings
Safety Considerations
Electrical Safety
	• Voltage Limits: Do not exceed 3.3V input on any GPIO
	• Power Supply: Use regulated, stable power source
	• ESD Protection: Handle ESP32-C6 with proper ESD precautions
Data Integrity
	• Backup: Regular SD card data backup recommended
	• Power Loss: System creates new files on each boot
	• File Size: Monitor SD card space for continuous operation
Technical Support
Diagnostic Tools Built-in
	1. Comprehensive Channel Testing: Automatic on startup
	2. Real-time Status Monitoring: Via serial commands
	3. Hardware Conflict Detection: GPIO diagnostic routines
	4. Performance Monitoring: Sample rate and memory tracking
Common Solutions
	• Zero Readings: Usually hardware connection issues
	• Inconsistent Data: Check power supply stability
	• SD Card Errors: Use faster, higher-quality cards
	• Memory Issues: Monitor heap usage at high speeds
Appendix
Default Configuration Summary
	• Sample Rate: 100 Hz (10ms interval)
	• ADC Resolution: 12-bit
	• Voltage Range: 0-3.3V (ADC_ATTEN_DB_12)
	• Special Config: CH4 uses ADC_ATTEN_DB_6 (0-2.2V)
	• Output Format: Auto-selects based on speed
	• File Format: CSV with comprehensive headers
Performance Benchmarks

Sample Rate

CPU Load

SD Write Rate

Heap Usage

1 Hz

Minimal

~80 bytes/min

Stable

10 Hz

Low

~48 KB/hour

Stable

100 Hz

Moderate

~4.8 MB/hour

Monitor
<img width="737" height="4748" alt="image" src="https://github.com/user-attachments/assets/12b9b3c9-fda1-4ae0-bdcd-9be6590504e5" />
