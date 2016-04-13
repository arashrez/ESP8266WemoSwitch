/*
 * Wemo Client for Amazon Echo
 * Not compatable with Wemo App due to incomplete protocol implementation
 * reddit.com/u/hapoo
 * 
 */

#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266WebServer.h>

int status = WL_IDLE_STATUS;
const char* ssid = "";  //  your network SSID (name)
const char* pass = "";       // your network password

const char* deviceName = "Wemo Emu";
unsigned int localPort = 2390;      // local port to listen for UDP packets
byte packetBuffer[4096]; //buffer to hold incoming and outgoing packets
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
ESP8266WebServer server ( 49153 );
IPAddress ip;
// Multicast declarations
IPAddress ipMulti(239, 255, 255, 250);
unsigned int portMulti = 1900;      // local port to listen on



void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);

  // setting up Station AP
  WiFi.begin(ssid, pass);

  // Wait for connect to AP
  Serial.print("[Connecting]");
  Serial.print(ssid);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    tries++;
    if (tries > 30) {
      break;
    }
  }
  Serial.println();


  printWifiStatus();

  Serial.println("Connected to wifi");
  Serial.print("Udp Multicast server started at : ");
  Serial.print(ipMulti);
  Serial.print(":");
  Serial.println(portMulti);
  Udp.beginMulticast(WiFi.localIP(),  ipMulti, portMulti);

  server.on ( "/setup.xml", setupxml );
  server.on ( "/upnp/control/basicevent1", event );
  server.begin();

}

void event() {

  if (server.args() > 0) {
    // Turn On Switch
    if ( server.arg(0).indexOf("<BinaryState>1</BinaryState>")  >= 0 ) {

      String response = String("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>\n")
                        + "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:1\">\n"
                        + "<CountdownEndTime>20</CountdownEndTime>\n"
                        + "</u:SetBinaryStateResponse>\n"
                        + "</s:Body> </s:Envelope>\n";
      server.send (200, "text/xml",  response );

      Serial.println("\n\nTurn On!\n");
      // Insert on action

    }
    // Turn Off Switch
    if ( server.arg(0).indexOf("<BinaryState>0</BinaryState>")  >= 0 ) {

      String response = String("<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\"><s:Body>\n")
                        + "<u:SetBinaryStateResponse xmlns:u=\"urn:Belkin:service:basicevent:0\">\n"
                        + "<CountdownEndTime>20</CountdownEndTime>\n"
                        + "</u:SetBinaryStateResponse>\n"
                        + "</s:Body> </s:Envelope>\n";
      server.send (200, "text/xml",  response );

      Serial.println("\n\nTurn Off!\n");
      // Insert off action

    }

  }




}



void setupxml() {

  Serial.println("\n requested setup.xml!!!!\n");


  String response = String("<?xml version=\"1.0\"?>\n")
                    + "<root>\n"
                    + "  <device>\n"
                    + "    <deviceType>urn:MakerMusings:device:controllee:1</deviceType>\n"
                    + "    <friendlyName>" + deviceName + "</friendlyName>\n"
                    + "    <manufacturer>Belkin International Inc.</manufacturer>\n"
                    + "    <modelName>Emulated Socket</modelName>\n"
                    + "    <modelNumber>3.1415</modelNumber>\n"
                    //+"    <UDN>uuid:Socket-1_0-5176f666669636</UDN>\n"
                    + "    <UDN>uuid:Socket-1_0-" + WiFi.macAddress()[0] + WiFi.macAddress()[1] + WiFi.macAddress()[3] + WiFi.macAddress()[4] + "17K0101769</UDN>\n"
                    + "  </device>\n"
                    + "</root>\n";


  server.send (200, "text/xml",  response );

}

void loop()
{
  server.handleClient();
  int noBytes = Udp.parsePacket();
  if ( noBytes ) {
    Serial.print(millis() / 1000);
    Serial.print(":Packet of ");
    Serial.print(noBytes);
    Serial.print(" received from ");
    Serial.print(Udp.remoteIP());
    Serial.print(":");
    Serial.println(Udp.remotePort());
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, noBytes); // read the packet into the buffer

    // display the packet contents in HEX


    if (strstr((char*)packetBuffer, "M-SEARCH") && strstr((char*)packetBuffer, "urn:Belkin")) {
      Serial.println();
      Serial.println();
      Serial.println("FOUND");
      Serial.println();
      Serial.println();

      String response = String("HTTP/1.1 200 OK\r\n")
                        + "CACHE-CONTROL: max-age=86400\r\n"
                        + "DATE: Fri, 4 Dec 2015 19:07:01 GMT\r\n"
                        + "EXT: \r\n"
                        + "LOCATION: http://" + ip[0] + "." + ip[1] + "." + ip[2] + "." + ip[3] + ":49153/setup.xml\r\n"
                        + "OPT: \"http://schemas.upnp.org/upnp/1/0/\"; ns=01\r\n"
                        + "01-NLS: 905bfa3c-1dd2-11b2-89" + WiFi.macAddress()[0] + WiFi.macAddress()[1] + "-fd8aebaf" + WiFi.macAddress()[3] + WiFi.macAddress()[4] + WiFi.macAddress()[6] + "c\n"
                        //+"01-NLS: 905bfa3c-1dd2-11b2-8928-fd8aebaf491c\r\n"
                        + "SERVER: Unspecified, UPnP/1.0, Unspecified\r\n"
                        + "X-User-Agent: redsonic\r\n"
                        + "ST: urn:Belkin:device:**\r\n"
                        //+"USN: uuid:Socket-1_0-221517K0101769::urn:Belkin:device:**";
                        + "USN: uuid:Socket-1_0-" + WiFi.macAddress()[0] + WiFi.macAddress()[1] + WiFi.macAddress()[3] + WiFi.macAddress()[4] + "17K0101769::urn:Belkin:device:**\r\n\r\n";

      char buf[512];
      response.toCharArray(buf, 512);
      Serial.println(response);

      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write(buf);
      Udp.endPacket();


    }


  } 



}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(WiFi.macAddress());


}
