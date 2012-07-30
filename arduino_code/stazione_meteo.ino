#include <Time.h>
#include <TimeAlarms.h>
#include <SD.h>
#include <SPI.h>         
#include <Ethernet.h>


byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x6A, 0x2F };
IPAddress my_ip(192, 168, 0, 2);
IPAddress server_ip(192, 168, 0, 112);
int server_port = 7999;
int LED_PIN = 9;
int SD_PIN = 4;


/************************************************
 *  Main Arduino methods
 ***********************************************/

void setup() 
{
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("Initializing...");
  pinMode(LED_PIN, OUTPUT);
  pinMode(SD_PIN, OUTPUT);
  pinMode(SS_PIN, OUTPUT);
  setSyncProvider(get_timestamp_from_server);
  
  while(timeStatus() == timeNotSet);

  init_sd();
 
  Alarm.timerRepeat(5, dump_data_from_sensors);  // every 5 seconds
//  Alarm.timerRepeat(5 * 60, dump_data_from_sensors);  // every 5 minutes

  Serial.println("STARTED");
}


void loop()
{
  Alarm.delay(1000);  // how much should this be? I don't have to do anything O.o
}




 

/************************************************
 *  SD METHODS
 ***********************************************/
 
int init_sd() { 
  digitalWrite(SD_PIN, LOW);
  
  if (!SD.begin(SD_PIN)) {
    Serial.println("SD initialization failed!");
    return 0;
  }
  return 1;
}

void get_next_filename(char* filename_buf, int filename_buf_size) {
  // generates the next non-existing filename, ready to be created and written.
  // WARNING: names must be short 8.3 (that's why we have to use this method btw)
  time_t next_file_counter = now();
  String filename;
  String counter_str;
  do {
    counter_str = String(next_file_counter++);
    filename = counter_str.substring(counter_str.length() - 8) + ".TXT";
    filename.toCharArray(filename_buf, filename_buf_size);
  } while (SD.exists(filename_buf));
}


void dump_data_from_sensors() {

  char filename_buf[13];
  get_next_filename(filename_buf, 13);

  Serial.print("writing ");
  Serial.print(filename_buf);
  
  File fout = SD.open(filename_buf, FILE_WRITE);
  if (fout) {
    fout.println(now());
    fout.close();
    Serial.println("\tOK");    
  } else {
    Serial.println("\tUnable to write the file");
  }
}


boolean print_directory(EthernetClient* stream) {
  File dir = SD.open("/");
  if (!dir) {
    Serial.println("Directory doesn't exist O.o");
    return false;
  }

  dir.rewindDirectory();
  while(File entry = dir.openNextFile()) {    
    String filename = String(entry.name());
    if (filename.endsWith(".TXT")) {
      stream->println(filename);
    }
    entry.close();
  }
  dir.close();
  return true;
}


boolean print_file(EthernetClient* stream, char *filename) {
  File fin = SD.open(filename);

  if (fin) {
    while (fin.available()) {
      stream->write(fin.read());
    }
    fin.close();
    return true;
  }  
  return false;
}


boolean delete_file(char *filename) {
  if (SD.exists(filename)) {
    return SD.remove(filename);
  }
  return false;
}





/************************************************
 *  ETHERNET METHODS
 ***********************************************/

void init_ethernet() {
  // disable SD
  digitalWrite(SD_PIN, HIGH);

  Ethernet.begin(mac, my_ip);
}





/************************************************
 *  TIME METHODS
 ***********************************************/

time_t get_timestamp_from_server() {
  EthernetClient client;
  String line = "";

  init_ethernet();
  
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

