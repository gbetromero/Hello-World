/* this sktech will scan a barcode and send it  through PJON to RECVIEVER Arduino 
 *  or other device receiving, it might have up to 5 slaves(Sender on one PJON BUS) 
 *  and receive with 1 master(Receiver)
 *  
 * hardware used 
 * ---Arduino Mega Sender   (slave)
 * ---Arduino UNO  Reciever (Master)
 * ---UHS(USB HOST SHIELD) with 2.0 library
 * ---bar code scanner....fully functional with code
 * 
 * This sketch will scan a barcode and send to master
 */

//-----------------------start
//hid scanner library
#include <usbhid.h>
#include <usbhub.h>
#include <hiduniversal.h>
#include <hidboot.h>
#include <SPI.h>
//-------------------------end

//------------------------start
//pjon library
#include <PJON.h>
//-------------------------end

//--------------------------start
//pjon bus id protocol
PJON<SoftwareBitBang> bus(1);
//--------------------------end

//----------------------------start
//assigned variable
//int Sendtime = 2000;     //SET EVERY SECOND SEC. SEND THE DATA
//float floatValTemp=0;  //INITIALIZE VARIAB. TEMPERATURE
String Scanner = "";     //NAME SENSORE (attenzione allo spazio finale)
String stringVal = "";   //VAR. SENSOR READING STRING
int xxx= 0 ;
//unsigned long time;
//------------------------------end


//-----------------------------start
void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  Serial.println ("Received data from Master 45");
  String data;

  Serial.print ("length: ");
  Serial.println (length);

  for (int i = 0; i < length; i++) {
    char tmp = payload[i];
    data += tmp;
  }
}
//--------------------------------end

//----------------------------start

/* Master and future v11 definition
   void error_handler(uint8_t code, uint8_t data, void *custom_pointer) { */

void error_handler(uint8_t code, uint8_t data) { // 10.1 error handler definition
  if (code == PJON_CONNECTION_LOST) {
    Serial.print("Connection with device ID ");
    Serial.print(bus.packets[data].content[0], DEC);
    Serial.println(" is lost.");
    digitalWrite (13, HIGH);
    delay (2000);
  }
  digitalWrite (13, LOW);
}

//-----------------------------end


class MyParser : public HIDReportParser {
  public:
    MyParser();
    void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);
  protected:
    uint8_t KeyToAscii(bool upper, uint8_t mod, uint8_t key);
    virtual void OnKeyScanned(bool upper, uint8_t mod, uint8_t key);
    virtual void OnScanFinished();
};

MyParser::MyParser() {}

void MyParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  // If error or empty, return
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print((String)buf[i]);
    Serial.print(" ");
  }
}

uint8_t MyParser::KeyToAscii(bool upper, uint8_t mod, uint8_t key) {
  // Letters
  if (VALUE_WITHIN(key, 0x04, 0x1d)) {
    if (upper) return (key - 4 + 'A');
    else return (key - 4 + 'a');
  }

  // Numbers
  else if (VALUE_WITHIN(key, 0x1e, 0x27)) {
    return ((key == UHS_HID_BOOT_KEY_ZERO) ? '0' : key - 0x1e + '1');
  }

  return 0;
}

void MyParser::OnKeyScanned(bool upper, uint8_t mod, uint8_t key) {
  uint8_t ascii = KeyToAscii(upper, mod, key);
  Serial.print((char)ascii);
}

void MyParser::OnScanFinished() {
  Serial.println(" - Finished");
}

USB          Usb;
USBHub       Hub(&Usb);
HIDUniversal Hid(&Usb);
MyParser     Parser;

//-------------------------------start
void setup() {
  bus.strategy.set_pin(4);  //
  bus.set_receiver(receiver_function);
  bus.set_error(error_handler);
  bus.begin();

  // start serial port
  Serial.begin(9600);

  //built in led set as output
  pinMode(13, OUTPUT);

//scanner bar coode intial setup
  if (Usb.Init() == -1) {
    }
  delay( 200 );
  Hid.SetReportParser(0, &Parser);


};
  

//----------------------------------end

void loop() {

      stringVal = String(Scanner + xxx); // xxx indicate other data 
    uint8_t lun = stringVal.length();
    char charVal[lun];
    stringVal.toCharArray(charVal, lun);

    bus.send(99, charVal, lun);
    bus.update();
    
    // Debug
    Serial.print ("Bar Code:  ");
    Serial.println(stringVal);
    Serial.print("ID:  ");
    //Serial.println(lun);
  Usb.Task();
  bus.update();
  bus.receive(1000);

}

