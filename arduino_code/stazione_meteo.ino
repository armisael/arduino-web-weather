#include <Time.h>
#include <TimeAlarms.h>
#include <SD.h>
#include <SPI.h>         
#include <Ethernet.h>


byte mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x6A, 0x2F };
IPAddress my_ip(192, 168, 0, 2);
IPAddress sheeva(192, 168, 0, 112);
int sheeva_port = 7999;
int LED_PIN = 9;
int SD_PIN = 4;
EthernetServer server(80);


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
  setSyncProvider(get_timestamp_from_sheeva);
  
  while(timeStatus() == timeNotSet);

  init_sd();
 
  Alarm.timerRepeat(5, dump_data_from_sensors);  // every 5 seconds
//  Alarm.timerRepeat(5 * 60, dump_data_from_sensors);  // every 5 minutes

  Serial.println("STARTED");
}


void loop()
{
  listen_for_http_requests();  // TODO[sp] remove this and POST data to the server instead
  Alarm.delay(1000);  // TODO[sp] let this be dynamic!
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

  print_success(stream);
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
    print_success(stream);
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
 *  WEBSERVER
 ***********************************************/

void init_ethernet() {
  // disable SD
  digitalWrite(SD_PIN, HIGH);

  Ethernet.begin(mac, my_ip);
}


void listen_for_http_requests() {
  EthernetClient client = server.available();
  String line = "";
  String request = "";
  char resource[12];
  if (client) {
    Serial.print("new client");
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        line += c;
        
        if (c == '\n') {
          if (line == "\r\n") {  // request completed
            
            Serial.print('\t');
            Serial.print('"');
            Serial.print(request);
            Serial.print('"');
            Serial.print("\t");
            Serial.print('"');
            Serial.print(resource);
            Serial.print('"');
            
            boolean success = false;
            if (request == "DELETE") {
              success = delete_file(resource);  // TODO[sp] any kind of security check on this?
              if (success) {  // we still need to print the success, since it's not done by the delete_file
                print_success(&client);
              }
            } else {
              if (strlen(resource) == 0) {
                success = print_directory(&client);              
              } else {
                success = print_file(&client, resource);
              }
            }
            
            if (!success) {
              print_404(&client);
            }
            break;
          } else {  // still something to say
            if (request == "") {
              int first_space = line.indexOf(' ');
              int second_space = line.indexOf(' ', first_space + 1);
              request = line.substring(0, first_space);
              // [sp] this happened: new client	"GET"	""R+¥±¥é¥¹¹..
              line.substring(first_space + 2, second_space).toCharArray(resource, 12);  // +1 if for the space, the other +1 is to remove the initial /
            }
            line = "";
          }
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("\tDONE");
  }
}

void print_success(EthernetClient *stream) {
  stream->println("HTTP/1.1 200 OK");
  stream->println("Content-Type: text/csv");
  stream->println("Connnection: close");
  stream->println();
}

void print_404(EthernetClient *stream) {
  stream->println("HTTP/1.1 404 NOT FOUND");
  stream->println("Connnection: close");
  stream->println();
}





/************************************************
 *  TIME METHODS
 ***********************************************/

time_t get_timestamp_from_sheeva() {
  EthernetClient client;
  String line = "";

  init_ethernet();
  
  Serial.print("connecting to the sheeva...");
  if (client.connect(sheeva, sheeva_port)) {

    Serial.print("\tconnected!");
    client.println("GET /arduino-timestamp/ HTTP/1.0");
    client.println("User-Agent: arduino-ethernet");
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

