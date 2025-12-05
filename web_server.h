#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "config.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "web_app.h"
#include "motor_control.h"
#include "eeprom_utils.h"
#include "ui_helpers.h"

WebServer server(80);

inline void handleRoot(){ noteUserActivity(); server.send_P(200,"text/html", WEB_INDEX); }
inline void handleNotFound(){ server.send(404,"text/plain","404"); }

// /api/drive?dir=±1&p=5..100
inline void apiDrive() {
  noteUserActivity();
  int dir = server.hasArg("dir") ? server.arg("dir").toInt() : 0;
  int p   = server.hasArg("p")   ? server.arg("p").toInt()   : 40;
  manualDrive(dir, (uint8_t)clampT(p,5,100));
  server.send(200,"text/plain","OK");
}

// /api/jog?mm=±value
inline void apiJog() {
  noteUserActivity();
  float mm = server.hasArg("mm") ? server.arg("mm").toFloat() : 0.0f;
  if (mm != 0.0f) moveDeltaMM(mm, g_speedPercent);
  server.send(200,"text/plain","OK");
}

// /api/stop
inline void apiStop(){ noteUserActivity(); plan.active=false; server.send(200,"text/plain","OK"); }

// /api/setSpeed?p=%
inline void apiSetSpeed(){ noteUserActivity(); int p=server.hasArg("p")?server.arg("p").toInt():40; setSpeedPercent(p); server.send(200,"text/plain","OK"); }

// OTA page
static const char OTA_INDEX[] PROGMEM = R"OTA(
<!doctype html><html><head><meta name=viewport content="width=device-width,initial-scale=1">
<title>SlidePilot OTA</title></head><body style="font-family:system-ui;background:#101418;color:#fff">
<div style="max-width:480px;margin:40px auto;padding:16px;border:1px solid #24313c;border-radius:10px;background:#0e1419">
<h2>SlidePilot OTA Update</h2>
<form method="POST" action="/update" enctype="multipart/form-data">
<input type="file" name="firmware" accept=".bin" required><br><br>
<input type="submit" value="Upload & Flash">
</form>
<p>After upload completes, the board will reboot.</p>
</div></body></html>
)OTA";

inline void handleOTA(){ noteUserActivity(); server.send_P(200,"text/html", OTA_INDEX); }
inline void handleUpdate(){
  noteUserActivity();
  HTTPUpload& up = server.upload();
  if(up.status == UPLOAD_FILE_START){
    Update.begin(UPDATE_SIZE_UNKNOWN);
  } else if(up.status == UPLOAD_FILE_WRITE){
    Update.write(up.buf, up.currentSize);
  } else if(up.status == UPLOAD_FILE_END){
    if(Update.end(true)){ server.send(200,"text/plain","OK, rebooting"); delay(300); ESP.restart(); return; }
  }
  server.send(200,"text/plain","OK");
}

inline void startWebServerAP() {
  if (strlen(runtimeState.ap_ssid)==0) strncpy(runtimeState.ap_ssid, "SlidePilot", sizeof(runtimeState.ap_ssid));
  if (strlen(runtimeState.ap_pass)==0) strncpy(runtimeState.ap_pass, "slidepilot", sizeof(runtimeState.ap_pass));

  WiFi.mode(WIFI_AP);
  WiFi.softAP(runtimeState.ap_ssid, runtimeState.ap_pass);
  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP: "); Serial.println(ip);

  server.on("/", handleRoot);
  server.on("/api/drive", apiDrive);
  server.on("/api/jog",   apiJog);
  server.on("/api/stop",  apiStop);
  server.on("/api/setSpeed", apiSetSpeed);
  server.on("/ota", handleOTA);
  server.on("/update", HTTP_POST, [](){ server.send(200,"text/plain","OK"); }, handleUpdate);
  server.onNotFound(handleNotFound);
  server.begin();
}

inline void webServerLoop(){ server.handleClient(); }

#endif
