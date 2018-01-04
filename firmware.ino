#include <ESP8266mDNS.h>


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

#define CONFIG_START 32

enum Channels{
Channel1 =  4,
Channel2 =  5,
Channel3 = 13,
Channel4 = 12
};


//Channel Settings
// String channelNames[4];
// int   channelTimers[4]; //timer in sec
// string channelTime[4];
// //Wifi Settings
//  String wifiMode="AP";
//  char apssid[]="GC-Automation";
//  String apSecurity="WPA";
//  char apPassword[]="default";
//
// //Client WifiSettings
// char clientssid[]="";
// String clientSecurity="";
// char clientPassword[]="";
ESP8266WebServer server(80);
struct config_t
{
  String channelNames[4];
  int   channelTimers[4]; //timer in sec
  String channelTime[4];
  //Wifi Settings
   String wifiMode;
   char apssid[];
   String apSecurity;
   char apPassword[];

  //Client WifiSettings
  char clientssid[];
  String clientSecurity;
  char clientPassword[];
}config={
  {"Channel 1","Channel 2","Channel 3","Channel 4"},
  {5,5,5,5},
  {"","","",""},
  "AP",
  {'G','C','-','A','u','t','o','m','a','t','i','o','n'}
  'WPA2',
  {'G','C','-','A','u','t','o','m','a','t','i','o','n'},
  {},
  "",
  {},
};


void setup() {

  Serial.begin(115200);
 loadConf();
 startWifi();
 server.begin();
 setupMDNS();
 timeClient.update();
}

void loop() {
  timeClient.update();
  server.handleClient();
  //timer check
}

void DefaultValues(){
  //set units names and timers
 config.channelNames[0]="Channel 1";
 config.channelNames[1]="Channel 2";
 config.channelNames[2]="Channel 3";
 config.channelNames[3]="Channel 4";

 config.channelTimers[0]=5;
 config.channelTimers[1]=5;
 config.channelTimers[2]=5;
 config.channelTimers[3]=5;

 config.channelTime[0]="";
 config.channelTime[1]="";
 config.channelTime[2]="";
 config.channelTime[3]="";


 //Wifi Settings
 config.wifiMode="AP";
// ssid="GC-Automation";
 strcpy(config.clientssid,"");
 config.clientSecurity="";
// password="default";
strcpy(config.clientPassword,""); //Wifi Settings

// ssid="GC-Automation";
 strcpy(config.apssid,"GC-Automation");
 config.apSecurity="WPA";
// password="default";
strcpy(config.apPassword,"GC-Automation");

}

void SaveConf(){
  for (unsigned int t=0; t<sizeof(settings); t++)
   { // writes to EEPROM
     EEPROM.write(CONFIG_START + t, *((char*)&settings + t));
     // and verifies the data
     if (EEPROM.read(CONFIG_START + t) != *((char*)&settings + t))
     {
       Serial.println("Error writing to memory.");
     }
   }
}

void loadConf(){
  for (unsigned int t=0; t<sizeof(settings); t++)
       *((char*)&settings + t) = EEPROM.read(CONFIG_START + t);
}
void startWifi(){
   byte ledStatus = LOW;
  Serial.println();
  // Serial.println("Connecting to: " + String(ssid));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(config.apssid, config.apPassword);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {

    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void setupMDNS()
{
  // Call MDNS.begin(<domain>) to set up mDNS to point to
  // "GC.local"
  if (!MDNS.begin("GC"))
  {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

}
void startServer(){
  server.on("/", handleRoot);
server.onNotFound(handleNotFound);
server.begin();
Serial.println("Server on and Listening");
}

void handleRoot()
{
  if (server.hasArg("wifiType")) {
    String sWifiType = server.arg("wifiType");
    if (sWifiType="ap"){
      strcpy(config.apssid,server.arg("apssid").c_str());
      config.apSecurity=server.arg("apSecurity");
     strcpy(config.apPassword,server.arg("apPassword").c_str());

    }
    else if(sWifiType="client"){
      strcpy(config.clientssid,server.arg("clientssid").c_str());
      config.clientSecurity=server.arg("clientSecurity");
     strcpy(config.clientPassword,server.arg("clientPassword").c_str());
    }
    else if(sWifiType="apClient"){
      //client Settings
      strcpy(config.clientssid,server.arg("clientssid").c_str());
      config.clientSecurity=server.arg("clientSecurity");
     strcpy(config.clientPassword,server.arg("clientPassword").c_str());
     //AP settings
     strcpy(config.apssid,server.arg("apssid").c_str());
     config.apSecurity=server.arg("apSecurity");
    strcpy(config.apPassword,server.arg("apPassword").c_str());
    }
    //restart device
    SaveConf();
  }
  else if (server.hasArg("Channels")) {
    for(int i=1;i<5;i++){

      if (server.hasArg("channel"+i)&&server.arg("channel"+i)=="on"){
        //setTimer for Channel i
        startTimer(i);
      }
      else{
        //StopTimer for channel i
        stopTimer(i);
      }
    }
    SaveConf();
  }
  else if (server.hasArg("ChannelSettings")) {
    for(int i=1;i<5;i++){
      if (server.hasArg("channel"+String(i)+"-name")){
        config.channelNames[i-1]=server.arg("channel"+String(i)+"-name");
      }
      else{config.channelNames[i-1]="Channel"+i;
    }

    if (server.hasArg("channel"+String(i)+"-timer")){
      config.channelTimers[i-1]=String(server.arg("channel"+String(i)+"-timer")).toInt();
    }
    else{config.channelTimers[i-1]=5;
  }
  if (server.hasArg("channel"+String(i)+"-time-stamp")){
    config.channelNames[i-1]=server.arg("channel"+String(i)+"-time-stamp");
    if (server.arg("channel"+String(i)+"-time-stamp")=="min") {
      config.channelTimers[i-1]=config.channelTimers[i-1]*60;
    }
    if (server.arg("channel"+String(i)+"-time-stamp")=="sec") {
      //do nothing
    }
  }

}
SaveConf();
}
  else {
    String INDEX_HTML ="<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <!-- <meta http-equiv=\"refresh\" content=\"30\"> --> <title>GC Automation</title> <script src=\"//code.jquery.com/jquery-1.12.0.min.js\"></script> <script type=\"text/javascript\"> // Parse the URL parameter function getParameterByName(name, url) { if (!url) url = window.location.href; name = name.replace(/[\[\]]/g, \"\\$&\"); var regex = new RegExp(\"[?&]\" + name + \"(=([^&#]*)|&|#|$)\"), results = regex.exec(url); if (!results) return null; if (!results[2]) return ''; return decodeURIComponent(results[2].replace(/\+/g, \" \")); } // Give the parameter a variable name var dynamicContent = getParameterByName('gc'); $(document).ready(function() { if (dynamicContent == 'WifiSettings') { $('#WifiSettings').show(); } else if (dynamicContent == 'ChannelSettings') { $('#ChannelSettings').show(); } else if (dynamicContent == 'Channels') { $('#Channels').show(); } else { $('#Home').show(); } }); </script> <style> /*Colour Palet*/ .pastel-blue-1-l2 {color:#000 !important; background-color:#1b7a9f !important} .pastel-blue-1-l1 {color:#000 !important; background-color:#1b7a9f !important} .pastel-blue-1-base {color:#000 !important; background-color:#1b7a9f !important} .pastel-blue-1-d2 {color:#fff !important; background-color:#1b7a9f !important} .pastel-blue-1-d1 {color:#fff !important; background-color:#333 !important} body { font-family: 'Lato', sans-serif; background:#999; } .pagecontent { /* padding: 45px 12px; */ width: 700px; /* float:centre; */ position:relative; left:25%; top:55px; height: 100%; } .navbar{ overflow: hidden; background-color: #333; position: fixed; top: 0; width: 100%; } .navbar p { float: left; display: block; color: #f2f2f2; text-align: center; padding: 0px 16px; text-decoration: none; font-size:15px; cursor:pointer } .overlay { height: 100%; width: 0; position: fixed; z-index: 1; top: 0; left: 0; background-color: rgb(0,0,0); background-color: rgba(0,0,0, 0.9); overflow-x: hidden; transition: 0.5s; } .overlay-content { position: relative; top: 25%; width: 100%; text-align: center; margin-top: 30px; } .overlay a { padding: 8px; text-decoration: none; font-size: 36px; color: #818181; display: block; transition: 0.3s; } .overlay a:hover, .overlay a:focus { color: #f1f1f1; } .overlay .closebtn { position: absolute; top: 20px; right: 45px; font-size: 60px; } @media screen and (max-height: 450px) { .overlay a {font-size: 20px} .overlay .closebtn { font-size: 40px; top: 15px; right: 35px; } } .channelTable{ border:1px solid black; } .settingsTable{ float:center; border:1px solid black; } .WiFi{ border:0px; } .button{ background-color: #222; color: white; border-radius: 12px; -webkit-transition-duration: 0.7s; /* Safari */ transition-duration: 0.7s; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; border: 2px solid #333; padding: 5px 40px; } .button-refresh{ position: absolute; right: 15px; top:10px; background-color: #222; display: inline-block; } .button:hover{ background-color: #0592c8; color: #000; } .textbtn{ cursor: pointer; } input[type=checkbox]:after { content: \"\"; position: absolute; display: none; } input[type=checkbox]{ background-color: #222; color: #fff; } input[type=text]{ display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; background-color: #222; color: #fff; } input[type=text]:focus { display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; background-color: #0592c8; color:#000; } .timer{width: 35px; } </style> </head> <body onLoad=\"loadstart()\"> <div class=\"navbar\"> <p onclick=\"openNav()\">&#9776; MENU</p> <!-- <p id=\"menuName\" style=\"cursor:default\">Home</p> --> <input type=\"button\" onclick=\"location.reload(true)\" class=\"button button-refresh\" value=\"Refresh\" > </div> <div id=\"Home\" class=\"pagecontent\" hidden> <!-- Discription of Product Logo --> <svg height=\"840.35\" version=\"1.1\" viewBox=\"0 0 1056 928.24615\" xmlns=\"http://www.w3.org/2000/svg\" width=\"970\"> <defs> <linearGradient id=\"grad\" x1=\"0%\" y1=\"0%\" x2=\"0%\" y2=\"100%\"> <stop offset=\"0%\" style=\"stop-color:rgb(208,246,173);stop-opacity:1\"></stop> <stop offset=\"100%\" style=\"stop-color:rgb(182,248,255);stop-opacity:1\"></stop> </linearGradient> </defs> <path d=\"m673.39 924.74c-1.8607-2.9479-74.692-129.05-142.35-246.46-11.903-20.657-18.457-31.32-19.624-31.928-1.3749-0.71592-37.352-0.91453-167.21-0.92307l-165.46-0.0109-1.891-2.0308c-1.04-1.1169-7.5345-11.834-14.432-23.815-43.583-75.706-162.31-281.18-164.14-284.07-2.1887-3.4512-2.2097-3.5846-2.2097-14.053v-10.569l7.6596-13.102c4.2128-7.206 36.899-63.779 72.637-125.72 35.737-61.938 72.699-125.99 82.137-142.34l17.16-29.723h378.59l-1.1512 3.5077c-1.662 5.0642-31.096 57.357-34.43 61.167-1.852 2.1172-3.6133 3.3517-5.1692 3.6233-1.2981 0.22659-66.994 0.68726-145.99 1.0237-101.06 0.43039-144.22 0.85141-145.6 1.4203-1.247 0.51081-3.0571 2.6591-4.9146 5.833-1.6173 2.7634-12.233 21.141-23.591 40.84-11.358 19.698-41.83 72.535-67.715 117.42-25.885 44.88-47.671 82.658-48.411 83.95-1.2842 2.2413-1.2873 2.4639-0.066 4.8 3.5215 6.7363 142.13 246.3 143.06 247.25 0.95016 0.97584 18.062 1.129 143.76 1.2865 112.52 0.14102 143.05-0.0151 144.42-0.73846 1.1032-0.58268 5.5975-7.6517 12.321-19.379 5.8211-10.154 33.827-58.671 62.236-107.82l51.652-89.354-108.47-0.18706c-93.238-0.1608-108.47-0.33462-108.47-1.2382 0-0.5781-0.52516-1.1585-1.167-1.2899-1.1046-0.22599-32.495-52.936-36.204-60.793-1.8004-3.8138-1.7358-5.6424 0.28045-7.9385 1.0776-1.2272 17.963-1.2923 335.11-1.2923 183.69 0 334.5 0.2017 335.15 0.44824 0.8873 0.34049 1.1681 1.5238 1.1681 4.9232v4.4749l-15.625 27.015c-19.608 33.902-19.59 33.873-22.097 35.17-1.8027 0.93224-21.425 1.0765-146.44 1.0765-86.173 0-145.08 0.27407-146.15 0.67997-1.3553 0.51527-8.0131 11.471-27.488 45.231-14.135 24.503-46.039 79.775-70.898 122.83-24.859 43.052-45.761 79.378-46.449 80.723-1.2374 2.4211-1.2246 2.4918 1.2726 7.0154 7.742 14.024 140.36 243.28 141.27 244.2 0.95489 0.9748 18.614 1.152 147.12 1.4769 141.18 0.357 146.11 0.4153 148.22 1.7539 2.8345 1.798 4.1889 3.4873 7.4752 9.323 1.4867 2.64 6.752 11.778 11.701 20.308 16.564 28.548 18.084 31.554 18.084 35.763v3.7443h-376.46z\" stroke-width=\".73846\" fill=\"url(#grad)\"> </path></svg> </div> <div id=\"Channels\" class=\"pagecontent\" hidden> <form action=\"/\" method=\"POST\"> <!-- List Channels and values --> <table style=\"width:100%\" border=\"1\" class=\"channelTable\"> <tr> <th>Channel</th> <th>Channel Name</th> <th>Timer</th> <th>End Time</th> <th>Active</th> </tr> <tr> <th>1</th> <th><var id=\"chn-1-name\"></var></th> <th><var id=\"chn-1-timer\"></var></th> <th><var id=\"chn-1-time\"></var></th> <th><input id=\"chn-1\" my_id=\"\" type=\"checkbox\" name=\"channel1\"/> </th> </tr> <tr> <th>2</th> <th><var id=\"chn-2-name\"></var></th> <th><var id=\"chn-2-timer\"></var></th> <th><var id=\"chn-2-time\"></var></th> <th><input id=\"chn-2\" my_id=\"\" type=\"checkbox\" name=\"channel2\" /> </th> </tr> <tr> <th>3</th> <th><var id=\"chn-3-name\"></var></th> <th><var id=\"chn-3-timer\"></var></th> <th><var id=\"chn-3-time\"></var></th> <th><input id=\"chn-3\" my_id=\"\" type=\"checkbox\" name=\"channel3\" /> </th> </tr> <tr> <th>4</th> <th><var id=\"chn-4-name\"></var></th> <th><var id=\"chn-4-timer\"></var></th> <th><var id=\"chn-4-time\"></var></th> <th><input id=\"chn-4\" my_id=\"\" type=\"checkbox\" name=\"channel4\" /> </th> </tr> </table> <br> <br> <INPUT type='submit' value='Save' class=\"button\"> </form> </div> <div id=\"WifiSettings\" class=\"pagecontent\" hidden> <forum> <!--Select Channel and change value --> <!-- Change WiFi AP settings --> <!-- Change WiFi Client settings --> <h3>Wifi Settings</h3> <br> <fieldset class=\"radiogroup\"> <legend>WiFi Modes</legend> <ul class=\"radio\"> <li><input type=\"radio\" name=\"wifiType\" id=\"wifiT1\" value=\"ap\" onclick=\"changeWiFiMode(this)\" checked>AP</li> <li><input type=\"radio\" name=\"wifiType\" id=\"wifiT2\" value=\"client\" onclick=\"changeWiFiMode(this)\">Client</li> <li><input type=\"radio\" name=\"wifiType\" id=\"wifiT3\" value=\"apClient\" onclick=\"changeWiFiMode(this)\">AP & Client</li> </ul> </fieldset> <fieldset class=\"WiFi\" id=\"settingsAP\"> <legend id=\"wiFiAP\">AP Settings</legend> <table class=\"settingsTable\"> <tr> <th> SSID: </th> <th> <input type=\"text\" name=\"apssid\" value=\"";
    INDEX_HTML += config.apssid; //APSSID
    INDEX_HTML +="\"> </th> </tr> <tr> <th> Password:</th> <th> <input type=\"password\" name=\"apPassword\" value=\"";
    INDEX_HTML += config.apPassword; //APPassword
    INDEX_HTML +="\"></th> </tr> <tr> <th> Security: </th> <th><select name=\"apSecurity\"> <option value=\"wpa\">WPA</option> <option value=\"wpa2\">WPA2</option> </select></th> </tr> </table> </fieldset> <fieldset id=\"settingsClient\" class=\"WiFi\" hidden> <legend id=\"wiFiClient\">Client Settings</legend> <table class=\"settingsTable\"> <tr> <th> SSID: </th> <th> <input type=\"text\" name=\"clientssid\" value=\"";
    INDEX_HTML += config.clientssid; //ClientSSID
    INDEX_HTML +="\"> </th> </tr> <tr> <th> Password:</th> <th> <input type=\"password\" name=\"clientPassword\" value=\"";
    INDEX_HTML += config.clientPassword; //ClientPassword
    INDEX_HTML +="\"></th> </tr> <tr> <th> Security: </th> <th><select name=\"clientSecurity\"> <option value=\"wpa\">WPA</option> <option value=\"wpa2\">WPA2</option> </select></th> </tr> </table> </fieldset> <INPUT type='submit' value='Save' class=\"button\"> </forum> </div> <div id=\"ChannelSettings\" class=\"pagecontent\" hidden> <form action=\"/\" method=\"POST\"> <!-- List Channels and values --> <table style=\"width:100%\" border=\"1\" class=\"channelTable\"> <tr> <th>Channel</th> <th>Channel Name</th> <th>Timer</th> </tr> <tr> <th>1</th> <th><input name=\"Channel1-name\" type=\"text\" class=\"inputText\" id=\"chn-1-name2\"> </th> <th><input name=\"Channel1-Timer\" type=\"text\" class=\"inputText timer\" id=\"chn-1-timer2\"> <select name=\"Channel1-time-stamp\" id=\"chn-1-time-stamp\"> <option value=\"min\">min</option> <option value=\"sec\">sec</option> </select></th> </tr> <tr> <th>2</th> <th><input name=\"Channel2-name\" type=\"text\" class=\"inputText\" id=\"chn-2-name2\"></th> <th><input name=\"Channel2-Timer\" type=\"text\" class=\"inputText timer\" id=\"chn-2-timer2\"> <select name=\"Channel2-time-stamp\" id=\"chn-2-time-stamp\"> <option value=\"min\">min</option> <option value=\"sec\">sec</option> </select></th> </tr> <tr> <th>3</th> <th><input name=\"Channel3-name\" type=\"text\" class=\"inputText\" id=\"chn-3-name2\"></th> <th><input name=\"Channel3-Timer\" class=\"inputText timer\" type=\"text\" id=\"chn-3-timer2\"> <select name=\"Channel3-time-stamp\" id=\"chn-3-time-stamp\"> <option value=\"min\">min</option> <option value=\"sec\">sec</option> </select></th> </tr> <tr> <th>4</th> <th><input name=\"Channel4-name\" type=\"text\" class=\"inputText\" id=\"chn-4-name2\"></th> <th><input name=\"Channel4-Timer\" class=\"inputText timer\" type=\"text\" id=\"chn-4-timer2\"> <select name=\"Channel4-time-stamp\" id=\"chn-4-time-stamp\"> <option value=\"min\">min</option> <option value=\"sec\">sec</option> </select></th> </tr> </table> <br> <br> <INPUT type='submit' value='Save' class=\"button\"> </form> </div> <div id=\"myNav\" class=\"overlay\"> <a href=\"javascript:void(0)\" class=\"closebtn\" onclick=\"closeNav()\">&times;</a> <div class=\"overlay-content\"> <a href=\"?gc=Home\" class=\"textbtn\">Home</a> <a href=\"?gc=Channels\" class=\"textbtn\">Channels</a> <a onclick=\"openNavSettings()\" class=\"textbtn\">Settings</a> </div> </div> <div id=\"myNavSettings\" class=\"overlay\"> <a href=\"javascript:void(0)\" class=\"closebtn\" onclick=\"closeNavSettings()\" onclick=\"openNav()\">&times;</a> <div class=\"overlay-content\"> <a href=\"?gc=WifiSettings\" class=\"textbtn\">Wifi Settings</a> <a href=\"?gc=ChannelSettings\" class=\"textbtn\">Channel Settings</a> </div> </div> <script> var channelV = document.querySelectorAll('input'); function loadstart(){ $(\"#chn-1-name\").text(\"";
    INDEX_HTML += config.channelNames[0];
    INDEX_HTML +="\"); $(\"#chn-2-name\").text(\"";
    INDEX_HTML +=  config.channelNames[1];
    INDEX_HTML +="\"); $(\"#chn-3-name\").text(\"";
    INDEX_HTML += config.channelNames[2];
    INDEX_HTML +="\"); $(\"#chn-4-name\").text(\"";
    INDEX_HTML += config.channelNames[3];
    INDEX_HTML +="\"); $(\"#chn-1-timer\").text(\"";
    INDEX_HTML += (config.channelTimers[0]>60)? float(config.channelTimers[0]/60) : config.channelTimers[0];
    INDEX_HTML += (config.channelTimers[0]>60)? " min" : " sec";
    INDEX_HTML +="\"); $(\"#chn-2-timer\").text(\"";
    INDEX_HTML += (config.channelTimers[1]>60)? float(config.channelTimers[1]/60) : config.channelTimers[1];
    INDEX_HTML += (config.channelTimers[1]>60)? " min" : " sec";
    INDEX_HTML +="\"); $(\"#chn-3-timer\").text(\"";
    INDEX_HTML += (config.channelTimers[2]>60)? float(config.channelTimers[2]/60) : config.channelTimers[2];
    INDEX_HTML += (config.channelTimers[2]>60)? " min" : " sec";
    INDEX_HTML +="\"); $(\"#chn-4-timer\").text(\"";
    INDEX_HTML += (config.channelTimers[3]>60)? float(config.channelTimers[3]/60) : config.channelTimers[3];
    INDEX_HTML += (config.channelTimers[3]>60)? " min" : " sec";
    INDEX_HTML +="\"); $(\"#chn-1-time\").text(\"";
    INDEX_HTML += config.channelTime[0];
    INDEX_HTML +="\"); $(\"#chn-2-time\").text(\"";
    INDEX_HTML += config.channelTime[1];
    INDEX_HTML +="\"); $(\"#chn-3-time\").text(\"";
    INDEX_HTML += config.channelTime[2];
    INDEX_HTML +="\"); $(\"#chn-4-time\").text(\"";
    INDEX_HTML += config.channelTime[3];
    INDEX_HTML +="\"); for(var i = 1; i <= 4; i++) { document.getElementById(\"chn-\"+i+\"-name2\").value=document.getElementById(\"chn-\"+i+\"-name\").textContent; document.getElementById(\"chn-\"+i+\"-timer2\").value=document.getElementById(\"chn-\"+i+\"-timer\").textContent.split(\" \")[0]; var dy='chn-'+i+'-time'; if(document.getElementById(dy).textContent==\"\"){ document.getElementById('chn-'+i).checked=false; } else{ document.getElementById('chn-'+i).checked=true; } var dy='chn-'+i+'-timer'; console.log(document.getElementById(dy).textContent.split(\" \")[1]); if(document.getElementById(dy).textContent.split(\" \")[2]==\"min\"){ document.getElementById(\"chn-\"+i+\"-time-stamp\").selectedIndex=0; }else if(document.getElementById(dy).textContent.split(\" \")[1]==\"sec\"){ document.getElementById(\"chn-\"+i+\"-time-stamp\").selectedIndex=1; } } for(var i = 0; i < 4; i++) { channelV[i].my_id=i; channelV[i].addEventListener('change', function(e){ SwitchChannel(e.currentTarget.my_id); }); } } function SwitchChannel(i){ alert(channelV[i].my_id); } function openNav() { document.getElementById(\"myNav\").style.width = \"100%\"; } function closeNav() { document.getElementById(\"myNav\").style.width = \"0%\"; } function openNavSettings() { closeNav(); document.getElementById(\"myNavSettings\").style.width = \"100%\"; } function closeNavSettings() { document.getElementById(\"myNavSettings\").style.width = \"0%\"; } function openPage(evt, pageName){ var i, tabcontent; tabcontent = document.getElementsByClassName(\"pagecontent\"); console.log(tabcontent); for (i = 0; i < tabcontent.length; i++) { tabcontent[i].style.display = \"none\"; } document.getElementById(\"menuName\").innerHTML=pageName; document.getElementById(pageName).style.display = \"block\"; evt.currentTarget.className += \" active\"; closeNav(); closeNavSettings(); } function changeWiFiMode(wifiT){ document.getElementById(\"settingsClient\").style.display=\"none\"; document.getElementById(\"settingsAP\").style.display=\"none\"; if(wifiT.value==\"apClient\"){ document.getElementById(\"settingsClient\").style.display=\"block\"; document.getElementById(\"settingsAP\").style.display=\"block\"; } else if (wifiT.value==\"ap\") { document.getElementById(\"settingsAP\").style.display=\"block\"; } else if (wifiT.value==\"client\") { document.getElementById(\"settingsClient\").style.display=\"block\"; } } </script> </body> </html>";


    server.send(200, "text/html", INDEX_HTML);
  }
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}



void returnOK()
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "OK\r\n");
}



void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void startTimer(int channel){
  if(config.channelTime[channel-1]==""){
    int timerSec=(config.channelTimers[channel-1]) %60;
    int timerMin=(config.channelTimers[channel-1]/60)%60;
    int timerHours=(config.channelTimers[channel-1]/60)/60;

    config.channelTime[channel-1]=(timeClient.getHours+timerHours)+":"+(timeClient.getMinutes+timerMin)+":"+(timeClient.getSeconds+timerSec);//getTime+config.channelTimers
  }
}

void stopTimer(int channel){
  if(config.channelTime[channel-1]!=""){
    config.channelTime[channel-1]="";
  }
}
