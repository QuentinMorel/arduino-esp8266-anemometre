unsigned long  next_timestamp = 0;
volatile unsigned long i = 0;
float wind = 0;
float last_wind = 0;
int count = 0;
volatile unsigned long last_micros;
long debouncing_time = 5; //in millis
int input_pin = 13;
char charBuffer[32];

const bool debugOutput = true;  // set to true for serial OUTPUT

/*************************************************/
/* Settings for number of reed contacts in       */
/* abenometer                                    */
/*************************************************/
const float number_reed = 4;

void ICACHE_RAM_ATTR Interrupt()
{
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    i++;
    last_micros = micros();
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(input_pin, INPUT_PULLUP);//D7

    attachInterrupt(input_pin,Interrupt,RISING);
}


void loop() 
{
  if (millis() > next_timestamp )    
  { 
    detachInterrupt(input_pin);
    count++; 
    float rps = i/number_reed; //computing rounds per second 
    if(i == 0)
      wind = 0.0;
    else
      wind = 1.761 / (1 + rps) + 3.013 * rps;// found here: https://www.amazon.de/gp/customer-reviews/R3C68WVOLJ7ZTO/ref=cm_cr_getr_d_rvw_ttl?ie=UTF8&ASIN=B0018LBFG8 (in German)
    if(last_wind - wind > 0.8 || last_wind - wind < -0.8 || count >= 10){
      if(debugOutput){
        Serial.print("Wind: ");
        Serial.print(wind);
        Serial.println(" km/h");
      }
      String strBuffer;
      strBuffer =  String(wind);
      strBuffer.toCharArray(charBuffer,10);
      
      count = 0;
    }
    i = 0;
    last_wind = wind;
   
    next_timestamp  = millis()+1000; //intervall is 1s
    attachInterrupt(input_pin,Interrupt,RISING);
  }
  yield();
}
