// sensors libs
#include <Wire.h>
#include <BMP085.h>
#include <DHT22.h>
// SD libs
#include <SdFat.h>
#include <SdFatUtil.h>
// time libs
#include <Time.h>
#include <TimeAlarms.h>
// ethernet libs
#include <Ethernet.h>
#include <SPI.h>

/************ SOME DEF STUFF ************/
#define BUFSIZ 100
#define FILESIZ 13
#define DHT22_PIN 7
#define BMP085_PIN 3
#define READ_INTERVAL 10
//#define READ_INTERVAL 60 * 5

/************ ETHERNET STUFF ************/
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x6A, 0x2F };
byte ip[] = { 192, 168, 0, 2 };
IPAddress server_ip(192, 168, 0, 112);
//IPAddress server_ip(192, 168, 0, 101);
int server_port = 7999;
//int server_port = 80;
EthernetClient client;

/************ SDCARD STUFF ************/
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

/************ SENSORS STUFF ************/
BMP085 myBMP085(BMP085_PIN);
DHT22 myDHT22(DHT22_PIN);





/************************************************
 *  Main Arduino methods
 ***********************************************/
void setup() {
  Wire.begin();
  myBMP085.init();
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!

  if (!card.init(SPI_HALF_SPEED, 4)) ;
  if (!volume.init(&card)) ;
  if (!root.openRoot(&volume)) ;

  Ethernet.begin(mac, ip);

  setSyncProvider(get_timestamp_from_server);
  while(timeStatus() == timeNotSet);

  Alarm.timerRepeat(READ_INTERVAL, dump_data_from_sensors);
}


void loop()
{
  Alarm.delay(15000);  // how much should this be? I don't have to do anything O.o
}





/************************************************
 *  READ/WRITE METHODS
 ***********************************************/

void get_next_filename(char* filename) {
  // generates the next non-existing filename, ready to be created and written.
  // WARNING: names must be short 8.3 (that's why we have to use this method btw)
  time_t next_file_counter = now();
  do {
    String(next_file_counter++).substring(2).toCharArray(filename, FILESIZ);
    strcpy(filename + 8, ".TXT");   
  } while (root.exists(filename));
}


void dump_data_from_sensors() {
  get_data_from_sensors();
  float temperature = get_temperature();
  float humidity = get_humidity();
  float pressure = get_pressure();

  char filename[FILESIZ];
  get_next_filename(filename);

  if (!file.open(&root, filename, O_CREAT | O_WRITE)) {
    return;
  }

  file.print("timestamp=");
  file.print(now());
  file.print("&temperature=");
  file.print(temperature);
  file.print("&humidity=");
  file.print(humidity);
  file.print("&pressure=");
  file.print(pressure);
  file.println();
  file.close(); 
  
  list_directory_and_post_to_server();
}





/************************************************
 *  SD+ETHERNET METHODS
 ***********************************************/

void list_directory_and_post_to_server() {
  dir_t p;
  char filename[FILESIZ];
  
  root.rewind();
  while (root.readDir(p) > 0) {
    // done if past last used entry
    if (p.name[0] == DIR_NAME_FREE) break;

    // skip deleted entry and entries for . and  ..
    if (p.name[0] == DIR_NAME_DELETED || p.name[0] == '.') continue;

    // only check for files
    if (!DIR_IS_FILE(&p)) continue;

    // filter out files that do not end with TXT
    if (strcmp((char*)p.name + 8, "TXT") == 0) {
      strncpy(filename, (char*)p.name, 8);
      strcpy(filename + 8, ".TXT");
      if(post_to_server(filename)) {
        root.remove(&root, filename);
      }
    }
  }
}

// TODO[sp] this sometimes fails to mark as success some successful requests
boolean post_to_server(char* filename) {
  char clientline[BUFSIZ];
  int index = 0;

  if (client.connect(server_ip, server_port)) {
    
    if (! file.open(&root, filename, O_READ)) {
      return false;
    }

    client.print  ("POST /arduino-post/?f=");
    client.print  (filename);
    client.println(" HTTP/1.0");
    client.println("User-Agent: arduino-ethernet-board");
    client.println("Host: webther.sundust.doesntexist.com");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print  ("Content-Length: ");
    client.println(file.fileSize() - 2);  // remove \r\n
    client.println();

    char c;
    while ((c = file.read()) > 0) {
        client.print(c);
    }
    file.close();

    boolean success = false;
    while (client.connected()) {
      if (client.available()) {
        c = client.read();
        
        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          if (index < BUFSIZ) {
            clientline[index++] = c;
          }
          continue;
        }
        
        // got a \n or \r new line, which means the string is done
        clientline[index] = 0;

        if (strstr(clientline, "200 OK") != 0) {
          success = true;
        }
        break;
      }
    }
    delay(1);
    client.stop();

    return success;
    
  } else {
    return false;
  }
}





/************************************************
 *  SENSORS METHODS
 ***********************************************/
DHT22_ERROR_t dht22_state;
void get_data_from_sensors() {
  dht22_state = myDHT22.readData();
  myBMP085.readData();
}

float get_temperature() {
  float temperature = myBMP085.getTemperature() * 0.1;
  if (dht22_state == DHT_ERROR_NONE)
    temperature = (temperature + myDHT22.getTemperatureC()) / 2;
  return temperature;
}

float get_humidity() {
  if (dht22_state == DHT_ERROR_NONE)
    return myDHT22.getHumidity();
  return -1;
}

float get_pressure() {
  return myBMP085.getPressure();
}





/************************************************
 *  TIME METHODS
 ***********************************************/

time_t get_timestamp_from_server() {
  EthernetClient client;
  String line = "";
  
  if (client.connect(server_ip, server_port)) {

    client.println("GET /arduino-timestamp/ HTTP/1.0");
    client.println("Host: webther.sundust.doesntexist.com");
    client.println("User-Agent: arduino-ethernet-board");
    client.println();

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        line += c;
        if (c == '\n') {
          line = "";
        }
      }
    }
    client.stop();
    return line.toInt();
    
  } else {
    return get_timestamp_from_server();
  }
}



