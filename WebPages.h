#ifdef USE_WIFI_WEB

#include "web-images.h"
#include "WebConsole.h"

////////////////////////////////
// List of available sequences by name and matching id
enum
{
    kMAX_FADE = 15,
    kMAX_DELAY = 500,
    kMIN_DELAY = 10,
    kMIN_BRI = 10,

    kMAX_ADJLOOP = 90000,
    kMIN_ADJLOOP = 500,
};

WMenuData mainMenu[] = {
    { "Logics", "/logics" },
    { "Setup", "/setup" }
};

WMenuData setupMenu[] = {
    { "Home", "/" },
    { "Serial", "/serial" },
    { "Sound", "/sound" },
    { "WiFi", "/wifi" },
    { "Remote", "/remote" },
    { "Firmware", "/firmware" },
    { "Console", "/console" },
    { "Servo Tester", "/servotester" },
    { "Back", "/" }
};

WElement mainContents[] = {
    WVerticalMenu("menu", mainMenu, SizeOfArray(mainMenu)),
    rseriesSVG
};

WElement setupContents[] = {
    WVerticalMenu("setup", setupMenu, SizeOfArray(setupMenu)),
    rseriesSVG
};

String logicsSeq[] = {
#define LOGICENGINE_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    nam,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef LOGICENGINE_SEQ
};

unsigned logicsSeqNumber[] = {
#define LOGICENGINE_SEQ(nam, val) \
    BUILTIN_SEQ(nam, LogicEngineDefaults::val)
#define BUILTIN_SEQ(nam, val) \
    val,

#include "logic-sequences.h"

#undef BUILTIN_SEQ
#undef LOGICENGINE_SEQ
};

String logicsColors[] = {
    "Default",
    "Red",
    "Orange",
    "Yellow",
    "Green",
    "Cyan",
    "Blue",
    "Purple",
    "Magenta",
    "Pink"
};

bool sFLDChanged = true;
bool sRLDChanged = true;

int sFLDSequence;
int sRLDSequence;

String sFLDText = "";
String sRLDText = "";
String sFLDDisplayText;
String sRLDDisplayText;

int sFLDColor = LogicEngineRenderer::kDefault;
int sRLDColor = LogicEngineRenderer::kDefault;

int sFLDSpeedScale;
int sRLDSpeedScale;

int sFLDNumSeconds;
int sRLDNumSeconds;

/////////////////////////////////////////////////////////////////////////
// Web Interface for logic engine animation sequences
WElement logicsContents[] = {
    WSelect("Front Logic Sequence", "frontseq",
        logicsSeq, SizeOfArray(logicsSeq),
        []() { return sFLDSequence; },
        [](int val) { sFLDSequence = val; sFLDChanged = true; } ),
    WSelect("Front Color", "frontcolor",
        logicsColors, SizeOfArray(logicsColors),
        []() { return sFLDColor; },
        [](int val) { sFLDColor = val; sFLDChanged = true; } ),
    WSlider("Animation Speed", "fldspeed", 0, 9,
        []()->int { return sFLDSpeedScale; },
        [](int val) { sFLDSpeedScale = val; sFLDChanged = true; } ),
    WSlider("Number of seconds", "fldseconds", 0, 99,
        []()->int { return sFLDNumSeconds; },
        [](int val) { sFLDNumSeconds = val; sFLDChanged = true; } ),
    WTextField("Front Text:", "fronttext",
        []()->String { return sFLDText; },
        [](String val) { sFLDText = val; sFLDChanged = true; } ),
    WSelect("Rear Logic Sequence", "rearseq",
        logicsSeq, SizeOfArray(logicsSeq),
        []() { return sRLDSequence; },
        [](int val) { sRLDSequence = val; sRLDChanged = true; } ),
    WSelect("Rear Color", "rearcolor",
        logicsColors, SizeOfArray(logicsColors),
        []() { return sRLDColor; },
        [](int val) { sRLDColor = val; sRLDChanged = true; } ),
    WSlider("Animation Speed", "rldspeed", 0, 9,
        []()->int { return sRLDSpeedScale; },
        [](int val) { sRLDSpeedScale = val; sRLDChanged = true; } ),
    WSlider("Number of seconds", "rldseconds", 0, 99,
        []()->int { return sRLDNumSeconds; },
        [](int val) { sRLDNumSeconds = val; sRLDChanged = true; } ),
    WTextField("Rear Text:", "reartext",
        []()->String { return sRLDText; },
        [](String val) { sRLDText = val; sRLDChanged = true; } ),
    WButton("Run", "run", []() {
        if (sFLDChanged)
        {
            sFLDDisplayText = sFLDText;
            sFLDDisplayText.replace("\\n", "\n");
            FLD.selectSequence(logicsSeqNumber[sFLDSequence], (LogicEngineRenderer::ColorVal)sFLDColor, sFLDSpeedScale, sFLDNumSeconds);
            FLD.setTextMessage(sFLDDisplayText.c_str());
            sFLDChanged = false;
        }
        if (sRLDChanged)
        {
            sRLDDisplayText = sRLDText;
            sRLDDisplayText.replace("\\n", "\n");
            RLD.selectSequence(logicsSeqNumber[sRLDSequence], (LogicEngineRenderer::ColorVal)sRLDColor, sRLDSpeedScale, sRLDNumSeconds);
            RLD.setTextMessage(sRLDDisplayText.c_str());
            sRLDChanged = false;
        }
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    rseriesSVG
};

////////////////////////////////

String swBaudRates[] = {
    "2400",
    "9600",
};

int marcSerial1Baud;
int marcSerial2Baud;
bool marcSerialPass;
bool marcSerialEnabled;
bool marcWifiEnabled;
bool marcWifiSerialPass;

WElement serialContents[] = {
    WSelect("Serial2 Baud Rate", "serial2baud",
        swBaudRates, SizeOfArray(swBaudRates),
        []() { return (marcSerial2Baud = (preferences.getInt(PREFERENCE_MARCSERIAL2, MARC_SERIAL2_BAUD_RATE)) == 2400) ? 0 : 1; },
        [](int val) { marcSerial2Baud = (val == 0) ? 2400 : 9600; } ),
    WVerticalAlign(),
    WCheckbox("Serial pass-through to Serial2", "serialpass",
        []() { return (marcSerialPass = (preferences.getBool(PREFERENCE_MARCSERIAL_PASS, MARC_SERIAL_PASS))); },
        [](bool val) { marcSerialPass = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite on Serial2", "enabled",
        []() { return (marcSerialEnabled = (preferences.getBool(PREFERENCE_MARCSERIAL_ENABLED, MARC_SERIAL_ENABLED))); },
        [](bool val) { marcSerialEnabled = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite on Wifi (port 2000)", "wifienabled",
        []() { return (marcWifiEnabled = (preferences.getBool(PREFERENCE_MARCWIFI_ENABLED, MARC_WIFI_ENABLED))); },
        [](bool val) { marcWifiEnabled = val; } ),
    WVerticalAlign(),
    WCheckbox("JawaLite Wifi pass-through to Serial2", "wifipass",
        []() { return (marcWifiSerialPass = (preferences.getBool(PREFERENCE_MARCWIFI_SERIAL_PASS, MARC_WIFI_SERIAL_PASS))); },
        [](bool val) { marcWifiSerialPass = val; } ),
    WVerticalAlign(),
    WButton("Save", "save", []() {
        preferences.putInt(PREFERENCE_MARCSERIAL1, marcSerial1Baud);
        preferences.putInt(PREFERENCE_MARCSERIAL2, marcSerial2Baud);
        preferences.putBool(PREFERENCE_MARCSERIAL_PASS, marcSerialPass);
        preferences.putBool(PREFERENCE_MARCSERIAL_ENABLED, marcSerialEnabled);
        preferences.putBool(PREFERENCE_MARCWIFI_ENABLED, marcWifiEnabled);
        preferences.putBool(PREFERENCE_MARCWIFI_SERIAL_PASS, marcWifiSerialPass);
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

String soundPlayer[] = {
    "Disabled",
    "MP3 Trigger",
    "DFMiniPlayer",
    "HCR"
};

String soundSerial[] = {
    "AUX4/AUX5",
    "Serial2"
};

int marcSoundPlayer;
int marcSoundSerial;
int marcSoundVolume;
int marcSoundStartup;
bool marcSoundRandom;
int marcSoundRandomMin;
int marcSoundRandomMax;

WElement soundContents[] = {
    WSelect("Sound Player", "soundPlayer",
        soundPlayer, SizeOfArray(soundPlayer),
        []() { return (marcSoundPlayer = preferences.getInt(PREFERENCE_MARCSOUND, MARC_SOUND_PLAYER)); },
        [](int val) { marcSoundPlayer = val; } ),
    WVerticalAlign(),
    WSelect("Sound Serial", "soundSerial",
        soundSerial, SizeOfArray(soundSerial),
        []() { return (marcSoundSerial = preferences.getInt(PREFERENCE_MARCSOUND_SERIAL, MARC_SOUND_SERIAL)); },
        [](int val) { marcSoundSerial = val; } ),
    WVerticalAlign(),
    WSlider("Sound Volume", "soundVolume", 0, 1000,
        []() { return (marcSoundVolume = preferences.getInt(PREFERENCE_MARCSOUND_VOLUME, MARC_SOUND_VOLUME)); },
        [](int val) {
            marcSoundVolume = val;
            sMarcSound.setVolume(marcSoundVolume / 1000.0);
        } ),
    WVerticalAlign(),
    WTextFieldInteger("Sound Startup", "soundStartup",
        []()->String { return String(marcSoundStartup = preferences.getInt(PREFERENCE_MARCSOUND_STARTUP, MARC_SOUND_STARTUP)); },
        [](String val) { marcSoundStartup = val.toInt(); }),
    WVerticalAlign(),
    WCheckbox("Random Sound", "soundRandom",
        []() { return (marcSoundRandom = (preferences.getBool(PREFERENCE_MARCSOUND_RANDOM, MARC_SOUND_RANDOM))); },
        [](bool val) { marcSoundRandom = val; } ),
    WVerticalAlign(),
    WTextFieldInteger("Random Min Millis", "soundRandomMin",
        []()->String { return String(marcSoundRandomMin = preferences.getInt(PREFERENCE_MARCSOUND_RANDOM_MIN, MARC_SOUND_RANDOM_MIN)); },
        [](String val) { marcSoundRandomMin = val.toInt(); }),
    WVerticalAlign(),
    WTextFieldInteger("Random Max Millis", "soundRandomMax",
        []()->String { return String(marcSoundRandomMax = preferences.getInt(PREFERENCE_MARCSOUND_RANDOM_MAX, MARC_SOUND_RANDOM_MAX)); },
        [](String val) { marcSoundRandomMax = val.toInt(); }),
    WVerticalAlign(),
    WButton("Save", "save", []() {
        preferences.putInt(PREFERENCE_MARCSOUND, marcSoundPlayer);
        preferences.putInt(PREFERENCE_MARCSOUND_SERIAL, marcSoundSerial);
        preferences.putInt(PREFERENCE_MARCSOUND_VOLUME, marcSoundVolume);
        preferences.putInt(PREFERENCE_MARCSOUND_STARTUP, marcSoundStartup);
        preferences.putBool(PREFERENCE_MARCSOUND_RANDOM, marcSoundRandom);
        preferences.putInt(PREFERENCE_MARCSOUND_RANDOM_MIN, marcSoundRandomMin);
        preferences.putInt(PREFERENCE_MARCSOUND_RANDOM_MAX, marcSoundRandomMax);
        if (marcSoundRandom)
        {
            sMarcSound.startRandom();
        }
        else
        {
            sMarcSound.stopRandom();
        }
        sMarcSound.setVolume(marcSoundVolume / 1000.0);
        // Flip values around if min is greater than max
        if (marcSoundRandomMin > marcSoundRandomMax)
        {
            int t = marcSoundRandomMin;
            marcSoundRandomMin = marcSoundRandomMax;
            marcSoundRandomMax = t;
        }
        sMarcSound.setRandomMin(marcSoundRandomMin);
        sMarcSound.setRandomMax(marcSoundRandomMax);
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

String wifiSSID;
String wifiPass;
bool wifiAP;

WElement wifiContents[] = {
    W1("WiFi Setup"),
    WCheckbox("WiFi Enabled", "enabled",
        []() { return wifiEnabled; },
        [](bool val) { wifiEnabled = val; } ),
    WHR(),
    WCheckbox("Access Point", "apmode",
        []() { return (wifiAP = preferences.getBool(PREFERENCE_WIFI_AP, WIFI_ACCESS_POINT)); },
        [](bool val) { wifiAP = val; } ),
    WTextField("WiFi:", "wifi",
        []()->String { return (wifiSSID = preferences.getString(PREFERENCE_WIFI_SSID, WIFI_AP_NAME)); },
        [](String val) { wifiSSID = val; } ),
    WPassword("Password:", "password",
        []()->String { return (wifiPass = preferences.getString(PREFERENCE_WIFI_PASS, WIFI_AP_PASSPHRASE)); },
        [](String val) { wifiPass = val; } ),
    WLabel("WiFi Disables Droid Remote", "label2"),
    WHR(),
    WButton("Save", "save", []() {
        DEBUG_PRINTLN("WiFi Changed");
        preferences.putBool(PREFERENCE_REMOTE_ENABLED, remoteEnabled);
        preferences.putBool(PREFERENCE_WIFI_ENABLED, wifiEnabled);
        preferences.putBool(PREFERENCE_WIFI_AP, wifiAP);
        preferences.putString(PREFERENCE_WIFI_SSID, wifiSSID);
        preferences.putString(PREFERENCE_WIFI_PASS, wifiPass);
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

String remoteHostName;
String remoteSecret;

WElement remoteContents[] = {
    W1("Droid Remote Setup"),
    WCheckbox("Droid Remote Enabled", "remoteenabled",
        []() { return remoteEnabled; },
        [](bool val) { remoteEnabled = val; } ),
    WHR(),
    WTextField("Device Name:", "hostname",
        []()->String { return (remoteHostName = preferences.getString(PREFERENCE_REMOTE_HOSTNAME, SMQ_HOSTNAME)); },
        [](String val) { remoteHostName = val; } ),
    WPassword("Secret:", "secret",
        []()->String { return (remoteSecret = preferences.getString(PREFERENCE_REMOTE_SECRET, SMQ_SECRET)); },
        [](String val) { remoteSecret = val; } ),
    WButton("Save", "save", []() {
        DEBUG_PRINTLN("Remote Changed");
        preferences.putBool(PREFERENCE_REMOTE_ENABLED, remoteEnabled);
        preferences.putString(PREFERENCE_REMOTE_HOSTNAME, remoteHostName);
        preferences.putString(PREFERENCE_REMOTE_SECRET, remoteSecret);
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

WElement firmwareContents[] = {
    W1("Firmware Setup"),
    WFirmwareFile("Firmware:", "firmware"),
    WFirmwareUpload("Reflash", "firmware"),
    WLabel("Current Firmware Build Date:", "label"),
    WLabel(__DATE__, "date"),
#ifdef BUILD_VERSION
    WHRef(BUILD_VERSION, "Sources"),
#endif
    WButton("Clear Prefs", "clear", []() {
        DEBUG_PRINTLN("Clear all preference settings");
        preferences.clear();
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Reboot", "reboot", []() {
        reboot();
    }),
    WHorizontalAlign(),
    WButton("Back", "back", "/setup"),
    WHorizontalAlign(),
    WButton("Home", "home", "/"),
    WVerticalAlign(),
    rseriesSVG
};

////////////////////////////////

WElement consoleContents[] = {
    WHTML(R"HTML(
<div style="max-width:900px;margin:0 auto;padding:10px;font-family:sans-serif;text-align:left">
<h2 style="text-align:center">Serial Console</h2>
<div id="term" style="background:#1e1e1e;color:#d4d4d4;font-family:monospace;font-size:13px;
  height:420px;overflow-y:auto;padding:8px;border-radius:4px;white-space:pre-wrap;
  word-break:break-all;border:1px solid #444"></div>
<div style="display:flex;margin-top:8px;gap:8px">
  <input id="cmdin" type="text" placeholder="e.g. DM:SCREAM or :SE01"
    style="flex:1;padding:8px;font-family:monospace;font-size:13px;
      border:1px solid #555;border-radius:4px;background:#2d2d2d;color:#d4d4d4"
    onkeydown="if(event.key==='Enter')sendCmd()">
  <button onclick="sendCmd()"
    style="padding:8px 16px;background:#007acc;color:#fff;border:none;border-radius:4px;cursor:pointer;font-size:13px">Send</button>
  <button onclick="document.getElementById('term').innerHTML=''"
    style="padding:8px 16px;background:#555;color:#fff;border:none;border-radius:4px;cursor:pointer;font-size:13px">Clear</button>
</div>
<p style="margin-top:8px;font-size:12px;color:#888;text-align:center">
  <a href="/" style="color:#888">Home</a> &bull;
  <a href="/setup" style="color:#888">Setup</a>
</p>
</div>
)HTML"),
    WJavaScript(R"JS(
var cseq=0,term=document.getElementById('term');
function ts(){var d=new Date();return d.toTimeString().substr(0,8);}
function addLine(t,c){
  var div=document.createElement('div');
  div.style.color=c||'#d4d4d4';
  div.textContent='['+ts()+'] '+t;
  term.appendChild(div);
  term.scrollTop=term.scrollHeight;
}
function pollLog(){
  fetch('/api/log?since='+cseq+'&')
    .then(function(r){return r.json();})
    .then(function(d){
      if(d.lines)d.lines.forEach(function(l){addLine(l,null);});
      cseq=d.seq;
    })
    .catch(function(){})
    .finally(function(){setTimeout(pollLog,1000);});
}
function sendCmd(){
  var inp=document.getElementById('cmdin'),cmd=inp.value.trim();
  if(!cmd)return;
  fetch('/api/send?cmd='+encodeURIComponent(cmd)+'&')
    .then(function(r){return r.json();})
    .catch(function(){});
  inp.value='';
}
addLine('Console ready — polling every 1s','#57a64a');
pollLog();
)JS"),
};

WAPI consoleLogAPI("/api/log", [](Print& out, String query) {
    int since = 0;
    int idx = query.indexOf("since=");
    if (idx >= 0) {
        int end = query.indexOf('&', idx);
        since = ((end >= 0) ? query.substring(idx+6, end) : query.substring(idx+6)).toInt();
    }
    String json = webConsoleJSON(since);
    out.println("HTTP/1.0 200 OK");
    out.println("Content-type:application/json");
    out.println("Cache-Control: no-cache");
    out.println("Connection: close");
    out.print("Content-Length:"); out.println(json.length());
    out.println();
    out.print(json);
});

WAPI consoleSendAPI("/api/send", [](Print& out, String query) {
    String json = "{\"ok\":false}";
    int idx = query.indexOf("cmd=");
    if (idx >= 0) {
        int end = query.indexOf('&', idx+4);
        String cmd = webUrlDecode((end >= 0) ? query.substring(idx+4, end) : query.substring(idx+4));
        if (cmd.length() > 0) {
            webConsoleLog(">>> " + cmd);
            Marcduino::processCommand(player, cmd.c_str());
            json = "{\"ok\":true}";
        }
    }
    out.println("HTTP/1.0 200 OK");
    out.println("Content-type:application/json");
    out.println("Cache-Control: no-cache");
    out.println("Connection: close");
    out.print("Content-Length:"); out.println(json.length());
    out.println();
    out.print(json);
});

////////////////////////////////
// Servo Tester — live raw-pulse control for calibrating dome panel min/max
// ranges. Moves servos directly by pulse width via servoDispatch.moveToPulse(),
// temporarily widening that servo's start/end bounds to the full hardware
// range (500-2500us) so the slider isn't clamped to whatever is currently
// baked into servoSettings[] in the .ino — the whole point is finding out
// what those numbers *should* be. Indices match the servoSettings[] panel
// entries documented at the top of DomeSequences.h (0-12; holo servos 13-18
// are out of scope here).
////////////////////////////////

const uint8_t kServoTesterCount = 13;

const char* const kServoTesterNames[kServoTesterCount] = {
    "Door 4 (P4)",
    "Door 3 (P3)",
    "Door 2 (P2)",
    "Door 1 (P1)",
    "Door 5 (P7)",
    "Door 9 (P10)",
    "Mini Door 2 (P11)",
    "Mini PSI Door (P13)",
    "Pie Panel 1 (PP1)",
    "Pie Panel 2 (PP2)",
    "Pie Panel 3 (PP5)",
    "Pie Panel 4 (PP6)",
    "Dome Top Panel"
};

WElement servoTesterContents[] = {
    WHTML(R"HTML(
<div style="max-width:520px;margin:0 auto;padding:10px;font-family:sans-serif;text-align:left">
<h2 style="text-align:center">Servo Tester</h2>
<p style="font-size:12px;color:#666">
Moves a single panel servo by raw pulse width so you can find its safe open/close
limits in a tight space. Movement ignores the firmware's current min/max — go
slowly and watch for binding. <b>Set Min</b> / <b>Set Max</b> just record the
slider value below so you can copy the numbers into <code>servoSettings[]</code>.
</p>

<label for="svsel"><b>Panel:</b></label>
<select id="svsel" style="width:100%;padding:6px;margin:6px 0"></select>

<p id="svinfo" style="font-size:12px;color:#666"></p>

<p style="text-align:center;font-size:28px;margin:4px 0" id="svpulse">--</p>
<input type="range" id="svslider" min="500" max="2500" value="1500" style="width:100%">

<div style="display:flex;justify-content:center;gap:6px;margin:8px 0">
  <button onclick="nudge(-10)">-10</button>
  <button onclick="nudge(-1)">-1</button>
  <button onclick="nudge(1)">+1</button>
  <button onclick="nudge(10)">+10</button>
</div>

<div style="display:flex;justify-content:center;gap:6px;margin:8px 0">
  <button onclick="goFirmware('start')">Go to Firmware Min</button>
  <button onclick="goFirmware('end')">Go to Firmware Max</button>
</div>

<div style="display:flex;justify-content:center;gap:6px;margin:8px 0">
  <button onclick="setMin()" style="background:#2d7;color:#fff;border:none;padding:8px 16px;border-radius:4px">Set Min Here</button>
  <button onclick="setMax()" style="background:#d72;color:#fff;border:none;padding:8px 16px;border-radius:4px">Set Max Here</button>
  <button onclick="relax()" style="background:#555;color:#fff;border:none;padding:8px 16px;border-radius:4px">Relax</button>
</div>

<p id="svmeasured" style="text-align:center;font-weight:bold"></p>
<input id="svresult" type="text" readonly style="width:100%;padding:6px;font-family:monospace;box-sizing:border-box" onclick="this.select()">

<p id="svstatus" style="text-align:center;color:#c00;font-weight:bold"></p>

<p style="margin-top:8px;font-size:12px;color:#888;text-align:center">
  <a href="/setup" style="color:#888">Setup</a> &bull;
  <a href="/" style="color:#888">Home</a>
</p>
</div>
)HTML"),
    WJavaScript(R"JS(
var svServos = [];
var svIdx = -1;
var svMeasuredMin = null, svMeasuredMax = null;
var svLastMoveSent = 0;

var svSel = document.getElementById('svsel');
var svSlider = document.getElementById('svslider');
var svPulse = document.getElementById('svpulse');
var svInfo = document.getElementById('svinfo');
var svMeasured = document.getElementById('svmeasured');
var svResult = document.getElementById('svresult');
var svStatus = document.getElementById('svstatus');

function svCurrent() { return svServos[svIdx]; }

function refreshInfo() {
    var s = svCurrent();
    if (!s) return;
    svInfo.textContent = 'Pin ' + s.pin + ' — firmware range currently ' + s.start + ' - ' + s.end;
}

function refreshMeasured() {
    var s = svCurrent();
    if (!s) return;
    var minTxt = (svMeasuredMin === null) ? '?' : svMeasuredMin;
    var maxTxt = (svMeasuredMax === null) ? '?' : svMeasuredMax;
    svMeasured.textContent = 'Measured: min=' + minTxt + '  max=' + maxTxt;
    svResult.value = 'servoSettings[' + svIdx + '] (' + s.name + ', pin ' + s.pin + '): startPulse=' +
        minTxt + ', endPulse=' + maxTxt;
}

function loadServos() {
    fetch('/api/servolist').then(function(r) { return r.json(); }).then(function(list) {
        svServos = list;
        svSel.innerHTML = '';
        list.forEach(function(s, i) {
            var opt = document.createElement('option');
            opt.value = i;
            opt.textContent = s.name;
            svSel.appendChild(opt);
        });
        selectServo(0);
    });
}

function selectServo(i) {
    svIdx = i;
    svSel.value = i;
    svMeasuredMin = null;
    svMeasuredMax = null;
    var s = svCurrent();
    svSlider.value = s.start;
    svPulse.textContent = s.start;
    refreshInfo();
    refreshMeasured();
}

svSel.onchange = function() { selectServo(parseInt(this.value)); };

function sendMove(pulse) {
    fetch('/api/servomove?idx=' + svIdx + '&pulse=' + pulse + '&')
        .then(function(r) { return r.json(); })
        .then(function(d) {
            svStatus.textContent = d.ok ? '' : ('Blocked: ' + d.reason);
        })
        .catch(function() {});
}

function moveTo(pulse) {
    pulse = Math.max(500, Math.min(2500, Math.round(pulse)));
    svSlider.value = pulse;
    svPulse.textContent = pulse;
    var now = Date.now();
    if (now - svLastMoveSent > 60) {
        svLastMoveSent = now;
        sendMove(pulse);
    }
}

svSlider.oninput = function() { moveTo(this.value); };
svSlider.onchange = function() { sendMove(this.value); };

function nudge(delta) { moveTo(parseInt(svSlider.value) + delta); }

function goFirmware(which) {
    var s = svCurrent();
    if (!s) return;
    moveTo(s[which]);
    sendMove(s[which]);
}

function setMin() { svMeasuredMin = parseInt(svSlider.value); refreshMeasured(); }
function setMax() { svMeasuredMax = parseInt(svSlider.value); refreshMeasured(); }

function relax() {
    fetch('/api/servorelax?idx=' + svIdx + '&');
}

loadServos();
)JS"),
};

WAPI servoListAPI("/api/servolist", [](Print& out, String query) {
    String json = "[";
    for (uint8_t i = 0; i < kServoTesterCount; i++)
    {
        if (i) json += ",";
        json += "{\"name\":\"" + String(kServoTesterNames[i]) + "\"";
        json += ",\"pin\":" + String((int)pgm_read_word(&servoSettings[i].pinNum));
        json += ",\"start\":" + String((int)pgm_read_word(&servoSettings[i].startPulse));
        json += ",\"end\":" + String((int)pgm_read_word(&servoSettings[i].endPulse));
        json += "}";
    }
    json += "]";
    out.println("HTTP/1.0 200 OK");
    out.println("Content-type:application/json");
    out.println("Cache-Control: no-cache");
    out.println("Connection: close");
    out.print("Content-Length:"); out.println(json.length());
    out.println();
    out.print(json);
});

// Moves servo `idx` straight to raw pulse `pulse`, bypassing its configured
// startPulse/endPulse clamp for the duration of the move so the tester can
// probe the servo's full hardware range.
WAPI servoMoveAPI("/api/servomove", [](Print& out, String query) {
    int idx = webQueryParamInt(query, "idx", -1);
    int pulse = webQueryParamInt(query, "pulse", -1);
    String json;
    if (idx < 0 || idx >= kServoTesterCount || pulse < 0)
    {
        json = "{\"ok\":false,\"reason\":\"bad request\"}";
    }
    else if (dome_eStopActive)
    {
        json = "{\"ok\":false,\"reason\":\"ESTOP active - send DM:RESET\"}";
    }
    else if (dome_seqRunning)
    {
        json = "{\"ok\":false,\"reason\":\"a dome sequence is running\"}";
    }
    else
    {
        if (pulse < 500) pulse = 500;
        if (pulse > 2500) pulse = 2500;
        uint16_t origStart = servoDispatch.getStart(idx);
        uint16_t origEnd = servoDispatch.getEnd(idx);
        servoDispatch.setStart(idx, 500);
        servoDispatch.setEnd(idx, 2500);
        servoDispatch.moveToPulse(idx, 0, (uint16_t)pulse);
        servoDispatch.setStart(idx, origStart);
        servoDispatch.setEnd(idx, origEnd);
        json = "{\"ok\":true}";
    }
    out.println("HTTP/1.0 200 OK");
    out.println("Content-type:application/json");
    out.println("Cache-Control: no-cache");
    out.println("Connection: close");
    out.print("Content-Length:"); out.println(json.length());
    out.println();
    out.print(json);
});

// Releases the state machine's hold on `idx` (still holds position under power).
WAPI servoRelaxAPI("/api/servorelax", [](Print& out, String query) {
    int idx = webQueryParamInt(query, "idx", -1);
    if (idx >= 0 && idx < kServoTesterCount)
        servoDispatch.disable(idx);
    String json = "{\"ok\":true}";
    out.println("HTTP/1.0 200 OK");
    out.println("Content-type:application/json");
    out.println("Cache-Control: no-cache");
    out.println("Connection: close");
    out.print("Content-Length:"); out.println(json.length());
    out.println();
    out.print(json);
});

//////////////////////////////////////////////////////////////////

WPage pages[] = {
    WPage("/", mainContents, SizeOfArray(mainContents)),
      WPage("/logics", logicsContents, SizeOfArray(logicsContents)),
    WPage("/setup", setupContents, SizeOfArray(setupContents)),
      WPage("/serial", serialContents, SizeOfArray(serialContents)),
      WPage("/sound", soundContents, SizeOfArray(soundContents)),
      WPage("/wifi", wifiContents, SizeOfArray(wifiContents)),
      WPage("/remote", remoteContents, SizeOfArray(remoteContents)),
      WPage("/firmware", firmwareContents, SizeOfArray(firmwareContents)),
      WPage("/console", consoleContents, SizeOfArray(consoleContents), "Console"),
      WPage("/servotester", servoTesterContents, SizeOfArray(servoTesterContents), "Servo Tester"),
    consoleLogAPI,
    consoleSendAPI,
    servoListAPI,
    servoMoveAPI,
    servoRelaxAPI,
        WUpload("/upload/firmware",
            [](Client& client)
            {
                if (Update.hasError())
                    client.println("HTTP/1.0 200 FAIL");
                else
                    client.println("HTTP/1.0 200 OK");
                client.println("Content-type:text/html");
                client.println("Vary: Accept-Encoding");
                client.println();
                client.println();
                client.stop();
                if (!Update.hasError())
                {
                    delay(1000);
                    preferences.end();
                    ESP.restart();
                }
                FLD.selectSequence(LogicEngineDefaults::FAILURE);
                FLD.setTextMessage("Flash Fail");
                FLD.selectSequence(LogicEngineDefaults::TEXTSCROLLLEFT, LogicEngineRenderer::kRed, 1, 0);
                FLD.setEffectWidthRange(1.0);
                FLD.setEffectWidthRange(1.0);
                otaInProgress = false;
            },
            [](WUploader& upload)
            {
                if (upload.status == UPLOAD_FILE_START)
                {
                    otaInProgress = true;
                    unmountFileSystems();
                    FLD.selectSequence(LogicEngineDefaults::NORMAL);
                    RLD.selectSequence(LogicEngineDefaults::NORMAL);
                    FLD.setEffectWidthRange(0);
                    RLD.setEffectWidthRange(0);
                    Serial.printf("Update: %s\n", upload.filename.c_str());
                    if (!Update.begin(upload.fileSize))
                    {
                        //start with max available size
                        Update.printError(Serial);
                    }
                }
                else if (upload.status == UPLOAD_FILE_WRITE)
                {
                    float range = (float)upload.receivedSize / (float)upload.fileSize;
                    DEBUG_PRINTLN("Received: "+String(range*100)+"%");
                   /* flashing firmware to ESP*/
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                    {
                        Update.printError(Serial);
                    }
                    FLD.setEffectWidthRange(range);
                    RLD.setEffectWidthRange(range);
                }
                else if (upload.status == UPLOAD_FILE_END)
                {
                    DEBUG_PRINTLN("GAME OVER");
                    if (Update.end(true))
                    {
                        //true to set the size to the current progress
                        Serial.printf("Update Success: %u\nRebooting...\n", upload.receivedSize);
                    }
                    else
                    {
                        Update.printError(Serial);
                    }
                }
            })
};

WifiWebServer<10,SizeOfArray(pages)> webServer(pages, wifiAccess);
#endif
