
//      ******************************************************************
//      *                                                                *
//      *             Header file for ESPStepperMotorServer.cpp          *
//      *                                                                *
//      *               Copyright (c) Paul Kerspe, 2019                  *
//      *                                                                *
//      ******************************************************************

// this project is not supposed to replace a controller of a CNC machine but more of a general approach on working with stepper motors
// for a good Arduino/ESP base Gerber compatible controller Project see:
// https://github.com/gnea/grbl
// and for ESP32: https://github.com/bdring/Grbl_Esp32
// currently no G-Code (http://linuxcnc.org/docs/html/gcode.html) parser is implemented, yet it might be part of a future release
// other usefull informaion when connecting your ESP32 board to your driver boards and you are not sure which pins to use: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

// MIT License
//
// Copyright (c) 2019 Paul Kerspe
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef ESPStepperMotorServer_h
#define ESPStepperMotorServer_h

#define ESPServerMaxSwitches 10
#define ESPServerSwitchStatusRegisterCount 2 //NOTE: this value must be chosen according to the value of ESPServerMaxSwitches: val = ceil(ESPServerMaxSwitches / 8)
#define ESPServerMaxSteppers 10
#define ESPServerMaxRotaryEncoders 5
#define ESPStepperMotorServer_SwitchDisplayName_MaxLength 20

#include <ESP_FlexyStepper.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
#include <ESPAsyncWebServer.h>
#include <ESPStepperMotorServer_WebInterface.h>
#endif

#include <ESPStepperMotorServer_CLI.h>
#include <ESPStepperMotorServer_Configuration.h>
#include <ESPStepperMotorServer_MotionController.h>
#include <ESPStepperMotorServer_MacroAction.h>
#include <ESPStepperMotorServer_PositionSwitch.h>
#include <ESPStepperMotorServer_StepperConfiguration.h>
#include <ESPStepperMotorServer_RotaryEncoder.h>
#include <ESPStepperMotorServer_Logger.h>

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
#include <ESPStepperMotorServer_RestAPI.h>
#endif

#define ESPServerWifiModeDisabled 0
#define ESPServerWifiModeAccessPoint 1
#define ESPServerWifiModeClient 2

#define ESPServerRestApiEnabled 2
#define ESPServerWebserverEnabled 4
#define ESPServerSerialEnabled 8

#define ESPServerSwitchType_ActiveHigh 1
#define ESPServerSwitchType_ActiveLow 2

#define ESPServerSwitchType_HomingSwitchBegin 4
#define ESPServerSwitchType_HomingSwitchEnd 8
#define ESPServerSwitchType_GeneralPositionSwitch 16
#define ESPServerSwitchType_EmergencyStopSwitch 32

#define ESPStepperHighestAllowedIoPin 50

//just forward declare class here for compiler, since we have a circular dependency (due to bad api design :-))
class ESPStepperMotorServer_CLI;
class ESPStepperMotorServer_RestAPI;
class ESPStepperMotorServer_Configuration;
class ESPStepperMotorServer_MotionController;
class ESPStepperMotorServer_MacroAction;

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
class ESPStepperMotorServer_WebInterface;
class ESPStepperMotorServer_RestAPI;
#endif
//
// the ESPStepperMotorServer class
// TODO: remove all wifi stuff if not needed using: #if defined(ESPServerWifiModeClient) || defined(ESPServerWifiModeAccessPoint)
class ESPStepperMotorServer
{
  friend class ESPStepperMotorServer_MotionController;

public:
  ESPStepperMotorServer(byte serverMode, byte logLevel = ESPServerLogLevel_INFO);
  ESPStepperMotorServer(const ESPStepperMotorServer &espStepperMotorServer);
  ~ESPStepperMotorServer();

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
  void setHttpPort(int portNumber);
  void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
  void sendSocketMessageToAllClients(const char *message, size_t len);
#endif

  void setAccessPointName(const char *accessPointSSID);
  void setAccessPointPassword(const char *accessPointPassword);
  void setWifiCredentials(const char *ssid, const char *pwd);
  void setWifiSSID(const char *ssid);
  void setWifiPassword(const char *pwd);
  void setWifiMode(byte wifiMode);
  void setStaticIpAddress(IPAddress staticIP, IPAddress gatewayIP, IPAddress subnetMask, IPAddress dns1 = (uint32_t) 0x00000000, IPAddress dns2 = (uint32_t) 0x00000000);
  void printWifiStatus();
  void printCompileSettings();
  int addOrUpdateStepper(ESPStepperMotorServer_StepperConfiguration *stepper, int stepperIndex = -1);
  int addOrUpdatePositionSwitch(ESPStepperMotorServer_PositionSwitch *posSwitchToAdd, int switchIndex = -1);
  int addOrUpdateRotaryEncoder(ESPStepperMotorServer_RotaryEncoder *rotaryEncoder, int encoderIndex = -1);
  void removePositionSwitch(int positionSwitchIndex);
  void removeStepper(byte stepperConfigurationIndex);
  void removeRotaryEncoder(byte rotaryEncoderConfigurationIndex);
  void getFormattedPositionSwitchStatusRegister(byte registerIndex, String &output);
  void printPositionSwitchStatus();
  void performEmergencyStop(int stepperIndex = -1);
  void revokeEmergencyStop();
  void start();
  void stop();
  byte getPositionSwitchStatus(int positionSwitchIndex);
  signed char updateSwitchStatusRegister();
  String getIpAddress();
  ESPStepperMotorServer_Configuration *getCurrentServerConfiguration();
  void requestReboot(String rebootReason);
  bool isSPIFFSMounted();

  //delegator functions only
  void setLogLevel(byte);
  void getServerStatusAsJsonString(String &statusString);
  byte getFirstAvailableConfigurationSlotForRotaryEncoder();
  bool isIoPinUsed(int);

  //
  // public member variables
  //
  const char *defaultConfigurationFilename = "/config.json";
  int wifiClientConnectionTimeoutSeconds = 25;
  // a boolean indicating if a position switch that has been configure as emegrency switch, has been triggered
  volatile boolean emergencySwitchIsActive = false;

  const char *version = "0.4.5";

private:
  void scanWifiNetworks();
  void connectToWifiNetwork();
  void startAccessPoint();

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
  void startWebserver();
  void registerWebInterfaceUrls();
  bool checkIfGuiExistsInSpiffs();
  bool downloadFileToSpiffs(const char *url, const char *targetPath);
  AsyncWebServer* getWebServer() {return httpServer;}
#endif

  void startSPIFFS();
  void printSPIFFSStats();
  int getSPIFFSFreeSpace();
  void printSPIFFSRootFolderContents();
  void setupAllIOPins();
  void setupPositionSwitchIOPin(ESPStepperMotorServer_PositionSwitch *posSwitch);
  void setupRotaryEncoderIOPin(ESPStepperMotorServer_RotaryEncoder *rotaryEncoder);
  void detachInterruptForPositionSwitch(ESPStepperMotorServer_PositionSwitch *posSwitch);
  void detachInterruptForRotaryEncoder(ESPStepperMotorServer_RotaryEncoder *rotaryEncoder);
  void detachAllInterrupts();
  void attachAllInterrupts();
  void setPositionSwitchStatus(int positionSwitchIndex, byte status);

  // ISR handling
  static void staticPositionSwitchISR();
  static void staticEmergencySwitchISR();
  static void staticLimitSwitchISR_POS_END();
  static void staticLimitSwitchISR_POS_BEGIN();
  static void staticLimitSwitchISR_COMBINED();
  static void staticRotaryEncoderISR();

  void internalEmergencySwitchISR();
  void internalSwitchISR(byte switchType);
  void internalRotaryEncoderISR();

  //
  // private member variables
  //
  byte enabledServices;
  boolean isWebserverEnabled = false;
  boolean isRestApiEnabled = false;
  boolean isCLIEnabled = false;
  boolean isServerStarted = false;
  boolean isSPIFFSactive = false;
  boolean _isRebootScheduled = false;

  ESPStepperMotorServer_Configuration *serverConfiguration;

#ifndef ESPStepperMotorServer_COMPILE_NO_WEB
  ESPStepperMotorServer_WebInterface *webInterfaceHandler;
  ESPStepperMotorServer_RestAPI *restApiHandler;
  AsyncWebServer *httpServer;
  AsyncWebSocket *webSockerServer;
#endif

  ESPStepperMotorServer_CLI *cliHandler;
  ESPStepperMotorServer_MotionController *motionControllerHandler;
  static ESPStepperMotorServer *anchor; //used for self-reference in ISR
  // the button status register for all configured button switches
  volatile byte buttonStatus[ESPServerSwitchStatusRegisterCount] = {0};
};

// ------------------------------------ End ---------------------------------
#endif
