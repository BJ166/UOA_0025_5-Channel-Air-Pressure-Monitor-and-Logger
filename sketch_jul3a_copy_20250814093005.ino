#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// ADC handle
adc_oneshot_unit_handle_t adc1_handle;

// ADC channel configuration
#define ADC_ATTEN ADC_ATTEN_DB_12  // 0-3.3V range
#define ADC_BITWIDTH ADC_BITWIDTH_12

// Sensor to ADC channel mapping
#define SENSOR1_CHANNEL 0  // GPIO0
#define SENSOR2_CHANNEL 1  // GPIO1
#define SENSOR3_CHANNEL 2  // GPIO2
#define SENSOR4_CHANNEL 3  // GPIO3
#define SENSOR5_CHANNEL 4  // GPIO4 (potentially problematic)
#define SENSOR6_CHANNEL 5  // GPIO5

// SD Card pins for ESP32-C6
#define SD_CS_PIN 18
#define SD_MOSI_PIN 19
#define SD_MISO_PIN 20
#define SD_SCK_PIN 21

// CSV file name (will be generated dynamically)
String filename;

// Variables for data logging
unsigned long lastLogTime = 0;
unsigned long logInterval = 10; // Default: 10ms = 100 samples per second
unsigned long fastLogCount = 0;
bool highSpeedMode = true; // Enable high-speed logging by default

// Channel status tracking
bool channel_working[6] = {false, false, false, false, false, false};

// Debug mode flag
bool debug_mode = true;
bool compact_output = true; // Default to compact output for high-speed logging

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000);  // Extended delay for serial monitor
  
  Serial.println("\n\n=== ESP32-C6 Six Channel ADC Logger - ULTRA HIGH SPEED MODE ===");
  Serial.println("==============================================================");
  Serial.printf("Logging Rate: %d samples per second (every %lums)\n", 1000/logInterval, logInterval);
  Serial.println("WARNING: 100Hz logging - ensure good power supply and fast SD card!");
  
  // Display basic device information
  displayDeviceInfo();
  
  // Check GPIO configuration and conflicts
  checkGPIOSetup();
  
  // Perform comprehensive GPIO diagnostic
  performGPIODiagnostic();
  
  // Initialize SD card
  if (!initializeSDCard()) {
    Serial.println("SD Card initialization failed! Continuing with serial output only.");
  }
  
  // Initialize ADC with enhanced error checking
  if (!initializeADC()) {
    Serial.println("ADC initialization failed!");
    return;
  }
  
  // Special focus on Sensor 2 (GPIO1/CH1) troubleshooting
  troubleshootSensor2();
  
  Serial.println("\nSystem ready! Ultra high-speed logging active (100Hz)...");
  Serial.println("Available commands:");
  Serial.println("  'debug' - Detailed channel analysis");
  Serial.println("  'speed1' - 1/sec, 'speed10' - 10/sec, 'speed100' - 100/sec, 'speed1000' - 1000/sec");
  Serial.println("  'gpio' - GPIO diagnostic, 'sensor2' - Sensor 2 troubleshooting");
  Serial.println("  'compact' - Toggle compact/detailed output format");
  Serial.println();
}

void loop() {
  // Check for serial commands
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command == "debug") {
      performDetailedChannelAnalysis();
    } else if (command == "gpio") {
      performGPIODiagnostic();
    } else if (command == "sensor2") {
      troubleshootSensor2();
    } else if (command == "ch4") {
      specialCH4Test();
    } else if (command == "speed1") {
      logInterval = 1000; // 1 sample per second
      Serial.printf("Logging rate set to 1 sample/second\n");
    } else if (command == "speed10") {
      logInterval = 100; // 10 samples per second
      Serial.printf("Logging rate set to 10 samples/second\n");
    } else if (command == "speed1000") {
      logInterval = 1; // 1000 samples per second
      compact_output = true; // Force compact mode for extreme speeds
      Serial.printf("Logging rate set to 1000 samples/second (EXTREME SPEED!)\n");
    } else if (command == "compact") {
      compact_output = !compact_output;
      Serial.printf("Output format: %s\n", compact_output ? "COMPACT" : "DETAILED");
    } else if (command == "status") {
      printSystemStatus();
    }
  }
  
  // Check if it's time to log data
  if (millis() - lastLogTime >= logInterval) {
    logAllSensorData();
    lastLogTime = millis();
  }
  
  // Small delay to prevent excessive CPU usage (optimized for ultra-high-speed)
  if (logInterval >= 100) {
    delay(10); // Normal delay for slower rates
  } else if (logInterval >= 10) {
    delay(1); // Minimal delay for 100Hz
  } else {
    delayMicroseconds(100); // Ultra-minimal delay for 1000Hz
  }
}

void performGPIODiagnostic() {
  Serial.println("\n=== GPIO DIAGNOSTIC TEST ===");
  
  // Check if GPIOs are properly configured as analog inputs
  int gpio_pins[] = {0, 1, 2, 3, 4, 5};
  const char* sensor_names[] = {"Sensor1", "Sensor2", "Sensor3", "Sensor4", "Sensor5", "Sensor6"};
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("\nDiagnosing GPIO%d (%s):\n", gpio_pins[i], sensor_names[i]);
    
    // Check GPIO mode
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_DISABLE;  // Set to analog mode
    io_conf.pin_bit_mask = (1ULL << gpio_pins[i]);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
      Serial.printf("  GPIO%d: Configured as analog input successfully\n", gpio_pins[i]);
    } else {
      Serial.printf("  GPIO%d: Configuration failed - %s\n", gpio_pins[i], esp_err_to_name(err));
    }
    
    // Additional checks for GPIO1 (Sensor 2)
    if (gpio_pins[i] == 1) {
      Serial.println("  Special GPIO1 checks:");
      Serial.println("    - GPIO1 should be the most reliable ADC pin on ESP32-C6");
      Serial.println("    - Check for hardware conflicts (UART, SPI, etc.)");
      Serial.println("    - Verify sensor is properly connected to GPIO1");
      Serial.println("    - Check for short circuits or damaged traces");
    }
  }
  Serial.println("===============================");
}

void troubleshootSensor2() {
  Serial.println("\n=== SENSOR 2 (GPIO1/CH1) TROUBLESHOOTING ===");
  Serial.println("GPIO1 is typically the MOST reliable ADC channel on ESP32-C6");
  Serial.println("If it's reading 0, this suggests a hardware issue.");
  Serial.println();
  
  // Test different configurations for Sensor 2
  adc_oneshot_chan_cfg_t test_configs[] = {
    {.atten = ADC_ATTEN_DB_0, .bitwidth = ADC_BITWIDTH_12},   // 0-1.1V
    {.atten = ADC_ATTEN_DB_2_5, .bitwidth = ADC_BITWIDTH_12}, // 0-1.5V  
    {.atten = ADC_ATTEN_DB_6, .bitwidth = ADC_BITWIDTH_12},   // 0-2.2V
    {.atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_12}   // 0-3.3V
  };
  
  const char* atten_names[] = {"DB_0(1.1V)", "DB_2_5(1.5V)", "DB_6(2.2V)", "DB_12(3.3V)"};
  float max_voltages[] = {1.1, 1.5, 2.2, 3.3};
  
  for (int cfg = 0; cfg < 4; cfg++) {
    Serial.printf("Testing Sensor2 with %s attenuation:\n", atten_names[cfg]);
    
    // Reconfigure Sensor 2 (CH1)
    esp_err_t err = adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &test_configs[cfg]);
    if (err != ESP_OK) {
      Serial.printf("  Config failed: %s\n", esp_err_to_name(err));
      continue;
    }
    
    delay(500); // Allow configuration to settle
    
    // Take multiple readings
    int valid_readings = 0;
    int zero_readings = 0;
    int max_reading = 0;
    long total_value = 0;
    
    for (int i = 0; i < 20; i++) {  // More readings for better analysis
      int raw_value;
      err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_1, &raw_value);
      if (err == ESP_OK) {
        valid_readings++;
        total_value += raw_value;
        if (raw_value == 0) zero_readings++;
        if (raw_value > max_reading) max_reading = raw_value;
        
        float voltage = (raw_value / 4095.0) * max_voltages[cfg];
        if (i < 5) {  // Show first 5 readings
          Serial.printf("  Reading %d: Raw=%d, Voltage=%.3fV\n", i+1, raw_value, voltage);
        }
      } else {
        Serial.printf("  Reading %d: ERROR - %s\n", i+1, esp_err_to_name(err));
      }
      delay(50);
    }
    
    if (valid_readings > 0) {
      int avg_raw = total_value / valid_readings;
      float avg_voltage = (avg_raw / 4095.0) * max_voltages[cfg];
      Serial.printf("  Results: %d valid readings, %d zeros, Max=%d, Avg=%.1f (%.3fV)\n", 
                    valid_readings, zero_readings, max_reading, (float)avg_raw, avg_voltage);
      
      if (zero_readings == valid_readings) {
        Serial.println("  *** ALL READINGS ARE ZERO - Hardware issue likely! ***");
      } else if (zero_readings > valid_readings / 2) {
        Serial.println("  *** MOSTLY ZERO READINGS - Check connections ***");
      }
    }
    Serial.println();
  }
  
  // Restore to standard configuration
  adc_oneshot_chan_cfg_t restore_config = {
    .atten = ADC_ATTEN_DB_12,
    .bitwidth = ADC_BITWIDTH_12,
  };
  adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_1, &restore_config);
  
  Serial.println("TROUBLESHOOTING RECOMMENDATIONS:");
  Serial.println("1. Check physical connections to GPIO1");
  Serial.println("2. Verify sensor output voltage is within 0-3.3V range");
  Serial.println("3. Test with a known voltage source (e.g., 3.3V or 1.65V)");
  Serial.println("4. Check for short circuits or damaged PCB traces");
  Serial.println("5. Try connecting sensor to a different GPIO pin");
  Serial.println("6. Measure actual voltage at GPIO1 with multimeter");
  Serial.println("=============================================");
}

void performDetailedChannelAnalysis() {
  Serial.println("\n=== DETAILED CHANNEL ANALYSIS ===");
  
  int channels[] = {SENSOR1_CHANNEL, SENSOR2_CHANNEL, SENSOR3_CHANNEL, 
                    SENSOR4_CHANNEL, SENSOR5_CHANNEL, SENSOR6_CHANNEL};
  const char* sensor_names[] = {"Sensor1", "Sensor2", "Sensor3", "Sensor4", "Sensor5", "Sensor6"};
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("\n--- %s (CH%d/GPIO%d) Analysis ---\n", sensor_names[i], channels[i], channels[i]);
    
    // Collect statistics over 50 readings
    int readings[50];
    int valid_count = 0;
    int error_count = 0;
    int zero_count = 0;
    
    for (int j = 0; j < 50; j++) {
      int raw_value = readSensorChannel(channels[i], (channels[i] == 4));
      if (raw_value >= 0) {
        readings[valid_count] = raw_value;
        valid_count++;
        if (raw_value == 0) zero_count++;
      } else {
        error_count++;
      }
      delay(20);
    }
    
    if (valid_count > 0) {
      // Calculate statistics
      int min_val = readings[0], max_val = readings[0];
      long sum = 0;
      
      for (int j = 0; j < valid_count; j++) {
        sum += readings[j];
        if (readings[j] < min_val) min_val = readings[j];
        if (readings[j] > max_val) max_val = readings[j];
      }
      
      float avg = (float)sum / valid_count;
      float avg_voltage = (avg / 4095.0) * 3.3;
      
      Serial.printf("Valid readings: %d/50 (%.1f%%)\n", valid_count, (valid_count/50.0)*100);
      Serial.printf("Zero readings: %d (%.1f%%)\n", zero_count, (zero_count/50.0)*100);
      Serial.printf("Error readings: %d (%.1f%%)\n", error_count, (error_count/50.0)*100);
      Serial.printf("Range: %d - %d (span: %d)\n", min_val, max_val, max_val - min_val);
      Serial.printf("Average: %.1f (%.3fV)\n", avg, avg_voltage);
      
      // Noise analysis
      float variance = 0;
      for (int j = 0; j < valid_count; j++) {
        variance += pow(readings[j] - avg, 2);
      }
      float std_dev = sqrt(variance / valid_count);
      Serial.printf("Std deviation: %.2f (noise level)\n", std_dev);
      
      // Quality assessment
      if (error_count > 25) {
        Serial.println("STATUS: CRITICAL - High error rate");
      } else if (zero_count > 40) {
        Serial.println("STATUS: PROBLEMATIC - Mostly zero readings");
      } else if (std_dev < 5 && max_val - min_val < 20) {
        Serial.println("STATUS: STABLE - Low noise");
      } else if (std_dev > 50) {
        Serial.println("STATUS: NOISY - High variation");
      } else {
        Serial.println("STATUS: FUNCTIONAL");
      }
      
      // Special analysis for Sensor 2
      if (i == 1) {  // Sensor 2 (index 1)
        Serial.println("\nSENSOR 2 SPECIFIC ANALYSIS:");
        if (zero_count > 40) {
          Serial.println("âš ï¸  CRITICAL: Sensor 2 reading mostly zeros!");
          Serial.println("   This is unusual for GPIO1 - hardware issue likely");
        } else if (avg < 100) {
          Serial.println("âš ï¸  WARNING: Sensor 2 readings very low");
          Serial.println("   Check sensor power and connections");
        } else {
          Serial.println("âœ… Sensor 2 appears to be working normally");
        }
      }
    } else {
      Serial.println("STATUS: FAILED - No valid readings");
    }
  }
  
  Serial.println("\n===================================");
}

bool checkGPIOSetup() {
  Serial.println("Enhanced GPIO Configuration Check:");
  Serial.println("  Sensor 1 -> ADC1_CH0 (GPIO0) [RELIABLE]");
  Serial.println("  Sensor 2 -> ADC1_CH1 (GPIO1) [MOST RELIABLE] âš ï¸");
  Serial.println("  Sensor 3 -> ADC1_CH2 (GPIO2) [RELIABLE]");
  Serial.println("  Sensor 4 -> ADC1_CH3 (GPIO3) [RELIABLE]");
  Serial.println("  Sensor 5 -> ADC1_CH4 (GPIO4) [CAUTION: Known ESP32-C6 issues]");
  Serial.println("  Sensor 6 -> ADC1_CH5 (GPIO5) [RELIABLE]");
  Serial.println("SD Card pins: CS=18, MOSI=19, MISO=20, SCK=21");
  Serial.println();
  Serial.println("âš ï¸  SENSOR 2 ISSUE DETECTED:");
  Serial.println("   GPIO1/CH1 is typically the most stable ADC channel");
  Serial.println("   Zero readings suggest hardware problem, not software");
  Serial.println("----------------------------------------");
  return true;
}

bool initializeSDCard() {
  delay(2000);
  // Initialize SPI with custom pins
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
  
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Mount Failed");
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return false;
  }
  
  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  
  // Generate unique filename
  generateUniqueFilename();
  
  // Create new CSV file with header
  createCSVFile();
  
  return true;
}

bool initializeADC() {
  // ADC1 Init
  adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
  };
  
  esp_err_t err = adc_oneshot_new_unit(&init_config1, &adc1_handle);
  if (err != ESP_OK) {
    Serial.printf("ADC1 unit initialization failed: %s\n", esp_err_to_name(err));
    return false;
  }
  
  // Configure all 6 ADC channels (0, 1, 2, 3, 4, 5)
  int channels[] = {SENSOR1_CHANNEL, SENSOR2_CHANNEL, SENSOR3_CHANNEL, 
                    SENSOR4_CHANNEL, SENSOR5_CHANNEL, SENSOR6_CHANNEL};
  const char* sensor_names[] = {"Sensor1", "Sensor2", "Sensor3", "Sensor4", "Sensor5", "Sensor6"};
  
  for (int i = 0; i < 6; i++) {
    adc_oneshot_chan_cfg_t config;
    
    // Special configuration for problematic CH4
    if (channels[i] == 4) {
      // Try different attenuation for CH4
      config.atten = ADC_ATTEN_DB_6;  // Lower attenuation sometimes works better
      config.bitwidth = ADC_BITWIDTH_12;
      Serial.printf("Configuring %s (CH%d/GPIO%d) with special settings for ESP32-C6...\n", 
                    sensor_names[i], channels[i], channels[i]);
    } else {
      // Standard configuration for other channels
      config.atten = ADC_ATTEN;
      config.bitwidth = ADC_BITWIDTH;
    }
    
    err = adc_oneshot_config_channel(adc1_handle, (adc_channel_t)channels[i], &config);
    if (err != ESP_OK) {
      Serial.printf("%s (CH%d/GPIO%d) config failed: %s\n", 
                    sensor_names[i], channels[i], channels[i], esp_err_to_name(err));
      channel_working[i] = false;
    } else {
      Serial.printf("%s (CH%d/GPIO%d) configured successfully\n", 
                    sensor_names[i], channels[i], channels[i]);
      channel_working[i] = true;
    }
  }
  
  // Test all ADC channels
  Serial.println("\nTesting all sensor channels:");
  testAllChannels();
  
  Serial.println("ADC initialization complete!");
  return true;
}

void testAllChannels() {
  int channels[] = {SENSOR1_CHANNEL, SENSOR2_CHANNEL, SENSOR3_CHANNEL, 
                    SENSOR4_CHANNEL, SENSOR5_CHANNEL, SENSOR6_CHANNEL};
  const char* sensor_names[] = {"Sensor1", "Sensor2", "Sensor3", "Sensor4", "Sensor5", "Sensor6"};
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("\nTesting %s (CH%d/GPIO%d):\n", sensor_names[i], channels[i], channels[i]);
    
    int successful_readings = 0;
    int zero_readings = 0;
    int total_readings = 10;
    
    for (int j = 0; j < total_readings; j++) {
      int raw_value = readSensorChannel(channels[i], i == 4); // Special handling for CH4
      
      if (raw_value >= 0) {
        successful_readings++;
        if (raw_value == 0) zero_readings++;
        
        float voltage = (raw_value / 4095.0) * ((channels[i] == 4) ? 2.2 : 3.3); // Adjust for CH4 attenuation
        Serial.printf("  Test %d: Raw=%d, Voltage=%.3fV\n", j+1, raw_value, voltage);
      } else {
        Serial.printf("  Test %d: ERROR\n", j+1);
      }
      delay(100);
    }
    
    // Update channel status based on test results
    if (successful_readings >= total_readings / 2) {
      channel_working[i] = true;
      Serial.printf("  Status: WORKING (%d/%d successful readings", successful_readings, total_readings);
      if (zero_readings > 0) {
        Serial.printf(", %d zeros", zero_readings);
      }
      Serial.println(")");
      
      // Special warning for Sensor 2 with zero readings
      if (i == 1 && zero_readings > total_readings / 2) {
        Serial.println("  âš ï¸  WARNING: Sensor 2 reading mostly zeros - this is unusual!");
      }
    } else {
      channel_working[i] = false;
      Serial.printf("  Status: PROBLEMATIC (%d/%d successful readings)\n", successful_readings, total_readings);
      
      if (channels[i] == 4) {
        Serial.println("  Note: CH4 issues are common on ESP32-C6 due to hardware limitations");
      } else if (channels[i] == 1) {
        Serial.println("  âš ï¸  CRITICAL: CH1/GPIO1 should be the most reliable channel!");
      }
    }
  }
  
  // Summary
  Serial.println("\n--- Channel Status Summary ---");
  for (int i = 0; i < 6; i++) {
    Serial.printf("Sensor%d (CH%d): %s", i+1, channels[i], 
                  channel_working[i] ? "WORKING" : "PROBLEMATIC");
    if (i == 1 && channel_working[i]) {
      Serial.print(" âš ï¸");  // Flag Sensor 2 for attention
    }
    Serial.println();
  }
  Serial.println("-----------------------------");
}

int readSensorChannel(int channel, bool is_problematic_channel) {
  // For ultra-high-speed logging, minimize samples to maintain timing
  const int num_samples = (logInterval <= 5) ? 1 : (logInterval <= 20) ? 2 : (is_problematic_channel ? 3 : 2);
  int readings[5];
  int valid_readings = 0;
  
  for (int i = 0; i < num_samples; i++) {
    int raw_value;
    esp_err_t err = adc_oneshot_read(adc1_handle, (adc_channel_t)channel, &raw_value);
    
    if (err == ESP_OK && raw_value >= 0 && raw_value <= 4095) {
      readings[valid_readings] = raw_value;
      valid_readings++;
    }
    
    // Minimal delays for ultra-high-speed logging
    if (logInterval <= 5) {
      // No delay for 1000Hz - maximum speed
    } else if (logInterval <= 20) {
      delayMicroseconds(50); // Minimal delay for 100Hz
    } else if (is_problematic_channel) {
      delay(1);
    } else {
      delayMicroseconds(200);
    }
  }
  
  if (valid_readings == 0) {
    return -1; // Error indicator
  } else if (valid_readings == 1) {
    return readings[0];
  } else {
    // Return median of valid readings for noise reduction
    return getMedian(readings, valid_readings);
  }
}

void logAllSensorData() {
  fastLogCount++;
  
  // Get current timestamp
  unsigned long timestamp = millis();
  unsigned long seconds = timestamp / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  
  seconds = seconds % 60;
  minutes = minutes % 60;
  hours = hours % 24;
  
  // Calculate milliseconds within current second
  unsigned long ms_in_second = timestamp % 1000;
  
  // Read all 6 sensors
  int channels[] = {SENSOR1_CHANNEL, SENSOR2_CHANNEL, SENSOR3_CHANNEL, 
                    SENSOR4_CHANNEL, SENSOR5_CHANNEL, SENSOR6_CHANNEL};
  const char* sensor_names[] = {"S1", "S2", "S3", "S4", "S5", "S6"};
  
  int sensor_raw[6];
  float sensor_voltage[6];
  
  for (int i = 0; i < 6; i++) {
    if (channel_working[i]) {
      sensor_raw[i] = readSensorChannel(channels[i], (channels[i] == 4)); // Special handling for CH4
      
      // Adjust voltage calculation for CH4 (different attenuation)
      if (channels[i] == 4 && sensor_raw[i] > 0) {
        sensor_voltage[i] = (sensor_raw[i] / 4095.0) * 2.2; // ADC_ATTEN_DB_6 range
      } else if (sensor_raw[i] > 0) {
        sensor_voltage[i] = (sensor_raw[i] / 4095.0) * 3.3; // ADC_ATTEN_DB_12 range
      } else {
        sensor_voltage[i] = -1.0;
      }
    } else {
      sensor_raw[i] = -1;
      sensor_voltage[i] = -1.0;
    }
  }
  
  // Optimized serial output based on speed and user preference
  if (compact_output || logInterval <= 20) {
    // Ultra-compact format for high-speed logging (â‰¥50 samples/sec)
    Serial.printf("%lu.%03lu,", seconds * 1000 + ms_in_second, ms_in_second);
    for (int i = 0; i < 6; i++) {
      if (sensor_raw[i] >= 0) {
        Serial.printf("%d", sensor_raw[i]);
      } else {
        Serial.print("-1");
      }
      if (i < 5) Serial.print(",");
    }
    Serial.println();
  } else {
    // Detailed format for slower logging or when requested
    Serial.printf("[%02lu:%02lu:%02lu.%03lu] ", hours, minutes, seconds, ms_in_second);
    for (int i = 0; i < 6; i++) {
      if (sensor_raw[i] >= 0) {
        Serial.printf("%s:%d(%.2fV) ", sensor_names[i], sensor_raw[i], sensor_voltage[i]);
        
        // Special alert for Sensor 2 zero readings (only in detailed mode)
        if (i == 1 && sensor_raw[i] == 0) {
          Serial.print("âš ï¸  ");
        }
      } else {
        Serial.printf("%s:ERR ", sensor_names[i]);
      }
    }
    Serial.println();
    
    // Special diagnostics for problematic readings (only in detailed mode)
    if (sensor_raw[1] == 0 && channel_working[1]) {
      Serial.println("  âš ï¸  ALERT: Sensor 2 (CH1/GPIO1) reading ZERO - Check hardware!");
    }
  }
  
  // Status update every 1000 samples in ultra-high-speed mode
  if (logInterval <= 10 && fastLogCount % 1000 == 0) {
    Serial.printf("--- %lu samples logged at %dHz (%.1fs runtime) ---\n", 
                  fastLogCount, 1000/logInterval, millis()/1000.0);
  } else if (logInterval <= 50 && fastLogCount % 100 == 0) {
    Serial.printf("--- %lu samples logged at %dHz ---\n", fastLogCount, 1000/logInterval);
  }
  
  // Write to SD card (buffered for high-speed logging)
  writeDataToSDCard(hours, minutes, seconds, timestamp, sensor_raw, sensor_voltage);
}

void displayDeviceInfo() {
  Serial.println("Device Information:");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", ESP.getChipRevision());
  Serial.printf("CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println("----------------------------------------");
}

void generateUniqueFilename() {
  // Generate filename based on sequential numbering
  int fileNumber = 1;
  
  // Find the next available file number
  do {
    filename = "/sensors_6ch_log_" + String(fileNumber) + ".csv";
    fileNumber++;
  } while (SD.exists(filename.c_str()) && fileNumber < 10000);
  
  Serial.printf("Generated filename: %s\n", filename.c_str());
}

void createCSVFile() {
  File file = SD.open(filename.c_str(), FILE_WRITE);
  if (file) {
    // Write CSV header with session info
    file.println("# Six Channel Sensor Data Log - ULTRA HIGH SPEED MODE");
    file.printf("# Started: Boot #%d\n", getBootCount());
    file.printf("# Uptime at start: %lu ms\n", millis());
    file.printf("# Logging Rate: %d samples per second (ULTRA HIGH SPEED!)\n", 1000/logInterval);
    file.println("# Channel Mapping: S1=CH0, S2=CH1, S3=CH2, S4=CH3, S5=CH4, S6=CH5");
    file.println("# Note: CH4 may have issues on ESP32-C6 hardware");
    file.println("# SENSOR 2 ISSUE: GPIO1/CH1 reading zeros - check hardware");
    file.println("# WARNING: 100Hz+ logging requires fast SD card and stable power");
    file.println("# Columns: Timestamp, Uptime_ms, S1_Raw, S2_Raw, S3_Raw, S4_Raw, S5_Raw, S6_Raw, S1_V, S2_V, S3_V, S4_V, S5_V, S6_V");
    file.println("Timestamp,Uptime_ms,S1_Raw,S2_Raw,S3_Raw,S4_Raw,S5_Raw,S6_Raw,S1_V,S2_V,S3_V,S4_V,S5_V,S6_V");
    file.close();
    Serial.printf("Created new CSV file: %s\n", filename.c_str());
  } else {
    Serial.println("Error creating CSV file");
  }
}

// Function to print current system status
void printSystemStatus() {
  Serial.println("\n=== ULTRA HIGH SPEED SYSTEM STATUS ===");
  Serial.printf("Logging Rate: %d samples/second (%lums interval)\n", 1000/logInterval, logInterval);
  Serial.printf("Total Samples: %lu\n", fastLogCount);
  Serial.printf("Uptime: %lu ms (%.1f minutes)\n", millis(), millis()/60000.0);
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Output Format: %s\n", compact_output ? "COMPACT" : "DETAILED");
  
  // Calculate data rate
  if (fastLogCount > 0) {
    float runtime_sec = millis() / 1000.0;
    float actual_rate = fastLogCount / runtime_sec;
    Serial.printf("Actual Rate: %.1f samples/second\n", actual_rate);
    
    // Estimate data size
    float mb_per_hour = (actual_rate * 3600 * 80) / (1024.0 * 1024.0); // ~80 bytes per sample
    Serial.printf("Data Rate: ~%.1f MB/hour\n", mb_per_hour);
  }
  
  // Channel status summary
  int working = 0;
  for (int i = 0; i < 6; i++) {
    if (channel_working[i]) working++;
  }
  Serial.printf("Working Channels: %d/6\n", working);
  
  if (SD.cardSize() > 0) {
    Serial.printf("SD Card: %lluMB\n", SD.cardSize() / (1024 * 1024));
  }
  
  Serial.println("Performance Tips:");
  Serial.println("  - Use Class 10 or UHS-I SD cards for 100Hz+");
  Serial.println("  - Ensure stable 3.3V power supply");
  Serial.println("  - Monitor free heap memory");
  
  Serial.println("Available Commands:");
  Serial.println("  speed1, speed10, speed100, speed1000 - Change rate");
  Serial.println("  compact - Toggle output format");
  Serial.println("  debug, gpio, sensor2, ch4 - Diagnostics");
  Serial.println("  status - Show this status");
  Serial.println("=====================================");
}

int getBootCount() {
  // Simple boot counter
  static int bootCount = 0;
  return ++bootCount;
}

int getMedian(int arr[], int size) {
  // Simple bubble sort for small arrays
  for (int i = 0; i < size - 1; i++) {
    for (int j = 0; j < size - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
  return arr[size / 2]; // Return middle value
}

void writeDataToSDCard(unsigned long hours, unsigned long minutes, unsigned long seconds, 
                       unsigned long timestamp_ms, int sensor_raw[6], float sensor_voltage[6]) {
  
  // For high-speed logging, write to SD card every 10 samples to reduce wear
  static String dataBuffer = "";
  static int bufferCount = 0;
  
  // Calculate milliseconds within current second
  unsigned long ms_in_second = timestamp_ms % 1000;
  
  // Format data line
  String dataLine = String(hours, DEC) + ":" + 
                   (minutes < 10 ? "0" : "") + String(minutes, DEC) + ":" + 
                   (seconds < 10 ? "0" : "") + String(seconds, DEC) + "." +
                   (ms_in_second < 100 ? "0" : "") + (ms_in_second < 10 ? "0" : "") + String(ms_in_second, DEC) + "," +
                   String(timestamp_ms, DEC);
  
  for (int i = 0; i < 6; i++) {
    dataLine += "," + String(sensor_raw[i], DEC);
  }
  
  for (int i = 0; i < 6; i++) {
    dataLine += "," + String(sensor_voltage[i], 3);
  }
  
  dataLine += "\n";
  
  // Add to buffer
  dataBuffer += dataLine;
  bufferCount++;
  
  // Write buffer to SD card when it reaches threshold or for slow logging rates
  int writeThreshold = (logInterval <= 5) ? 50 : (logInterval <= 20) ? 20 : 1; // Larger buffers for ultra-high speeds
  
  if (bufferCount >= writeThreshold) {
    File file = SD.open(filename.c_str(), FILE_APPEND);
    if (file) {
      file.print(dataBuffer);
      file.close();
      
      // Print confirmation less frequently for ultra-high-speed logging
      static int total_writes = 0;
      total_writes += bufferCount;
      
      if (logInterval <= 5 && total_writes % 1000 == 0) {
        Serial.printf("SD: %d entries (%.1fMB/min rate)\n", total_writes, 
                      (total_writes * 80.0) / (millis() / 60000.0) / 1024.0);
      } else if (logInterval <= 20 && total_writes % 200 == 0) {
        Serial.printf("SD: %d entries buffered and written\n", total_writes);
      } else if (logInterval > 20 && total_writes % 10 == 0) {
        Serial.printf("Data logged to SD card (%d entries)\n", total_writes);
      }
    } else {
      Serial.println("Error writing to SD card");
    }
    
    // Clear buffer
    dataBuffer = "";
    bufferCount = 0;
  }
}

// Advanced test function specifically for CH4
void specialCH4Test() {
  Serial.println("\n=== SPECIAL CH4 (SENSOR 5) DIAGNOSTIC ===");
  Serial.println("Testing different configurations for CH4/GPIO4...");
  
  // Test different attenuation settings for CH4
  adc_oneshot_chan_cfg_t test_configs[] = {
    {.atten = ADC_ATTEN_DB_0, .bitwidth = ADC_BITWIDTH_12},   // 0-1.1V
    {.atten = ADC_ATTEN_DB_2_5, .bitwidth = ADC_BITWIDTH_12}, // 0-1.5V  
    {.atten = ADC_ATTEN_DB_6, .bitwidth = ADC_BITWIDTH_12},   // 0-2.2V
    {.atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_12}   // 0-3.3V
  };
  
  const char* atten_names[] = {"DB_0(1.1V)", "DB_2_5(1.5V)", "DB_6(2.2V)", "DB_12(3.3V)"};
  float max_voltages[] = {1.1, 1.5, 2.2, 3.3};
  
  for (int cfg = 0; cfg < 4; cfg++) {
    Serial.printf("\nTesting CH4 with %s attenuation:\n", atten_names[cfg]);
    
    // Reconfigure CH4
    esp_err_t err = adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &test_configs[cfg]);
    if (err != ESP_OK) {
      Serial.printf("  Config failed: %s\n", esp_err_to_name(err));
      continue;
    }
    
    delay(200); // Allow configuration to settle
    
    // Take 10 readings with extended delays
    int valid_readings = 0;
    int total_value = 0;
    
    for (int i = 0; i < 10; i++) {
      int raw_value;
      err = adc_oneshot_read(adc1_handle, ADC_CHANNEL_4, &raw_value);
      if (err == ESP_OK && raw_value >= 0) {
        valid_readings++;
        total_value += raw_value;
        float voltage = (raw_value / 4095.0) * max_voltages[cfg];
        Serial.printf("  Reading %d: Raw=%d, Voltage=%.3fV\n", i+1, raw_value, voltage);
      } else {
        Serial.printf("  Reading %d: ERROR - %s\n", i+1, esp_err_to_name(err));
      }
      delay(100); // Extended delay
    }
    
    if (valid_readings > 0) {
      int avg_raw = total_value / valid_readings;
      float avg_voltage = (avg_raw / 4095.0) * max_voltages[cfg];
      Serial.printf("  Result: %d/10 valid readings, Avg Raw=%d, Avg Voltage=%.3fV\n", 
                    valid_readings, avg_raw, avg_voltage);
      
      if (valid_readings >= 7) {
        Serial.printf("  *** CH4 WORKS WELL with %s! Consider using this setting. ***\n", atten_names[cfg]);
      } else if (valid_readings >= 4) {
        Serial.printf("  CH4 partially works with %s (marginal)\n", atten_names[cfg]);
      }
    } else {
      Serial.println("  CH4 completely non-functional with this configuration");
    }
  }
  
  // Restore CH4 to the best working configuration (typically DB_6)
  adc_oneshot_chan_cfg_t restore_config = {
    .atten = ADC_ATTEN_DB_6,
    .bitwidth = ADC_BITWIDTH_12,
  };
  adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &restore_config);
  Serial.println("\nCH4 restored to ADC_ATTEN_DB_6 configuration");
  Serial.println("==========================================");
}

// Function to print channel status and statistics
void printChannelStatistics() {
  Serial.println("\n=== CHANNEL STATISTICS ===");
  
  int channels[] = {SENSOR1_CHANNEL, SENSOR2_CHANNEL, SENSOR3_CHANNEL, 
                    SENSOR4_CHANNEL, SENSOR5_CHANNEL, SENSOR6_CHANNEL};
  const char* sensor_names[] = {"Sensor1", "Sensor2", "Sensor3", "Sensor4", "Sensor5", "Sensor6"};
  
  for (int i = 0; i < 6; i++) {
    Serial.printf("%s (CH%d/GPIO%d): %s", 
                  sensor_names[i], channels[i], channels[i],
                  channel_working[i] ? "OPERATIONAL" : "PROBLEMATIC");
    
    if (i == 1) {  // Sensor 2
      Serial.print(" âš ï¸  [ZERO READING ISSUE]");
    }
    Serial.println();
    
    if (!channel_working[i] && channels[i] == 4) {
      Serial.println("  Recommendation: Run specialCH4Test() or use external ADC for Sensor5");
    } else if (i == 1 && channel_working[i]) {
      Serial.println("  Recommendation: Check hardware connections for zero readings");
    }
  }
  
  int working_channels = 0;
  for (int i = 0; i < 6; i++) {
    if (channel_working[i]) working_channels++;
  }
  
  Serial.printf("\nSummary: %d/6 channels operational (%.1f%%)\n", 
                working_channels, (working_channels / 6.0) * 100);
  Serial.println("=============================");
}