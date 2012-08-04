#include <SdFat.h>
#include <SdFatUtil.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <Ethernet.h>
#include <SPI.h>

/************ ETHERNET STUFF ************/
byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x6A, 0x2F };
byte ip[] = { 192, 168, 0, 2 };
IPAddress server_ip(192, 168, 0, 112);
int server_port = 7999;
EthernetClient client;

/************ SDCARD STUFF ************/
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

// How big our line buffer should be. 100 is plenty!
#define BUFSIZ 100
#define FILESIZ 13





/************************************************
 *  Main Arduino methods
 ***********************************************/
void setup() {
  Serial.begin(9600);
 
  Serial.println("\n\nInitializing...");
  Serial.println("Free RAM: ");
  Serial.println(FreeRam());  
  
  // initialize the SD card at SPI_HALF_SPEED to avoid bus errors with
  // breadboards.  use SPI_FULL_SPEED for better performance.
  pinMode(10, OUTPUT);                       // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH);                    // but turn off the W5100 chip!

  if (!card.init(SPI_HALF_SPEED, 4)) Serial.println("card.init failed!");
  if (!volume.init(&card)) Serial.println("vol.init failed!");
  if (!root.openRoot(&volume)) Serial.println("openRoot failed");

  Ethernet.begin(mac, ip);

  setSyncProvider(get_timestamp_from_server);
  while(timeStatus() == timeNotSet);

  Alarm.timerRepeat(5, dump_data_from_sensors);  // every 5 seconds
//  Alarm.timerRepeat(5 * 60, dump_data_from_sensors);  // every 5 minutes

  Serial.println("STARTED");
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

  char filename[FILESIZ];
  get_next_filename(filename);

  Serial.print("writing ");
  Serial.print(filename);
  
  if (!file.open(&root, filename, O_CREAT | O_WRITE)) {
    Serial.println("\tUnable to write the file");
    return;
  }

  file.print("timestamp=");
  file.print(now());
  file.print("&temperature=");
  file.print("26.8");
  file.println();
  file.close();
  Serial.println("\tOK");    
  
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

  Serial.print("sending to the server ");
  Serial.print(filename);   
  if (client.connect(server_ip, server_port)) {
    
    if (! file.open(&root, filename, O_READ)) {
      Serial.print("\tunable to read file ");
      Serial.println(filename);
      return false;
    }

    Serial.print("\tconnected!");
    client.println("POST /arduino-post/ HTTP/1.0");
    client.println("User-Agent: arduino-ethernet-board");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(file.fileSize() - 2);  // remove \r\n
    client.println();

    int16_t c;
    while ((c = file.read()) > 0) {
        client.print((char)c);
    }
    file.close();

    boolean success = false;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        // If it isn't a new line, add the character to the buffer
        if (c != '\n' && c != '\r') {
          clientline[index] = c;
          index++;
          // are we too big for the buffer? start tossing out data
          if (index >= BUFSIZ) 
            index = BUFSIZ -1;
          
          // continue to read more data!
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
    Serial.print("\tdisconnected with result: ");
    Serial.println(success);
    return success;
    
  } else {
    Serial.println("\tconnection failed -.-");
    return false;
  }
}





/************************************************
 *  TIME METHODS
 ***********************************************/

time_t get_timestamp_from_server() {  // TODO[sp] avoid using String, use char[]
  EthernetClient client;
  String line = "";
  
  Serial.print("asking current timestamp to the server...");
  if (client.connect(server_ip, server_port)) {

    Serial.print("\tconnected!");
    client.println("GET /arduino-timestamp/ HTTP/1.0");
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
    Serial.println("\tdisconnected");
    return line.toInt();
    
  } else {
    Serial.println("\tconnection failed -.-");
    return 0;
  }
}

