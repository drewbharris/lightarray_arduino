#include <VirtualWire.h>

const int IN_BUFF_LEN = 100;
int oldValues [5] = {-1, -1, -1, -1, -1};
int values [5] = {0, 0, 0, 0, 0};
int outputs [5] = {3, 11, 5, 6, 9};

const int transmit_pin = 12;
char rfValue[4];

void serial_handler(char * line){
  String message = String(line);
  if (message == String("test")){
    Serial.println("ready");
  }
  else if (message.startsWith("[")){
    message = message.substring(1, message.length() - 1);
    values[0] = getValue(message, ',', 0).toInt();
    values[1] = getValue(message, ',', 1).toInt();
    values[2] = getValue(message, ',', 2).toInt();
    values[3] = getValue(message, ',', 3).toInt();
    values[4] = getValue(message, ',', 4).toInt();
    for (int i = 0; i < sizeof(values)/sizeof(values[0]); i++){
      if (values[i] != oldValues[i]){
        // write the value
        if (i == 4) {
           itoa(values[i]*2, rfValue, 10);
           vw_send((uint8_t *)rfValue, strlen(rfValue));
        } else {
           analogWrite(outputs[i], values[i]*2); 
        }
      }
     oldValues[i] = values[i]; 
    }
  }
}

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++){
    pinMode(outputs[i], OUTPUT);
  }
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);
  
  vw_set_tx_pin(transmit_pin);
  vw_setup(4800);
}

void loop() {
  char in_buff[IN_BUFF_LEN] = {0};
  int buff_index = 0;
  int overflow = 0;
  char c;
  
  for(;;) {       
    // Add new characters to a buffer
    if(Serial.available()) {
      c = Serial.read();
      // If the buffer is full
      if(buff_index == IN_BUFF_LEN) {
        buff_index = 0;
        overflow = 1;
      }
      // Add new character to buffer, if it has not overflowed
      if(overflow == 0) {
        in_buff[buff_index] = c;
        buff_index++;
      }
      // Detect newline character
      if(c == '\r' || c == '\n') {
        if(overflow) {
          // Ignore this overflowed line.
          // Now that this overflowed line has ended, 
          // reset the overflow flag.
          overflow = 0;
        } else if(buff_index - 1 < IN_BUFF_LEN) {
          // Replace the '\n' with null terminator
      	  in_buff[buff_index - 1] = 0;
      	  // Parse this line as a command
      	  serial_handler(in_buff);
        }	
        buff_index = 0;
      }
    }
  }
}

