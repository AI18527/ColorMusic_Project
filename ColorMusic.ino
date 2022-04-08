#include <Adafruit_NeoPixel.h>
#include <arduinoFFT.h>

#define NLED 59 //The number of the LEDs
#define L_PIN 2 //Digital pin for the LEDs (Output)

#define S_PIN A0 //Analog pin for the sound sensor (Input)
#define SAMPLES 128             //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno
#define SAMPLING_FREQUENCY 2048 //Ts = Based on Nyquist, must be 2 times the highest expected frequency

/*Variables for the LEDs part*/
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NLED, L_PIN, NEO_RGB+NEO_KHZ800); //create Adafruit_NeoPixel object

int brightness = 0; 
int colors[3] = {0, 0, 0}; //[0]green, [1]red, [2]blue 

/*Variables for the fft part*/
arduinoFFT FFT = arduinoFFT(); //create arduinoFFT object 

unsigned int samplingPeriod;
unsigned long microSeconds;
 
double vReal[SAMPLES]; //create array(vector) of size SAMPLES to hold real values
double vImag[SAMPLES]; //create array(vector) of size SAMPLES to hold imaginary values

double peak = 0; //create variable to hold the calculated frequency

void setup() 
{
   Serial.begin(115200);
   samplingPeriod = round(1000000*(1.0/SAMPLING_FREQUENCY)); //Period in microseconds [1/(1 Hz) = 1000000 µs]
    
   pixels.begin(); 
   firstTurn(); // the function will execute only once at the begining
}
 
void loop() 
{ 
  double prevPeak = peak; //keep the previous value of peak
  peak = getPeak();
  Serial.println(peak);
  
  getColor();
  pixels.setBrightness(getBrg(prevPeak));
  
  for(int i = 0; i < NLED; i++){
    pixels.setPixelColor(i, pixels.Color(colors[0], colors[1], colors[2]));
    pixels.show();
  }

}
int getBrg(double prevPeak){
  /*Get brightness*/
  if(peak > prevPeak){
    if(peak - prevPeak > 255){
      brightness = 255;
    }
    else{
      brightness += (peak - prevPeak);
    }
  }
  else if(peak < prevPeak){
    if(prevPeak - peak > 255){
       brightness = 0;
    }
    else{
       brightness -= (prevPeak - peak);
     }
  }
  //else keep the same brightness
  return brightness;
}

void getColor(){
  /*Get color*/
  int caseNum = (int)(peak*2)/255; //9 cases (black-> red),(red-> yellow),(yellow-> green),(green-> cyan),(cyan-> blue),(blue-> magenta),(magenta-> red),(red-> white), (white)
  int extraColor = (int)(peak*2) % 255; // number between 0 and 254

  switch (caseNum){
    case 0:
      colors[1] = extraColor;
      colors[0] = 0;
      colors[2] = 0;
      break;
    case 1:
      colors[1] = 255;
      colors[0] = extraColor;
      colors[2] = 0;break;
    case 2:
      colors[1] = 255 - extraColor;
      colors[0] = 255;
      colors[2] = 0;break;
    case 3:
      colors[1] = 0;
      colors[0] = 255;
      colors[2] = extraColor;break;
    case 4:
      colors[1] = 0;
      colors[0] = 255 - extraColor;
      colors[2] = 255;break;
    case 5:
      colors[1] = extraColor;
      colors[0] = 0;
      colors[2] = 255;break;
    case 6:
      colors[1] = 255;
      colors[0] = 0;
      colors[2] = 255 - extraColor;break;
    case 7:
      colors[1] = 255;
      colors[0] = extraColor;
      colors[2] = extraColor;break;
    case 8:
      colors[0] = 255;
      colors[1] = 255;
      colors[2] = 255;break;
  }
}
void firstTurn(){
   for(int i = 0; i < NLED; i++){
      brightness ++;
      pixels.setPixelColor(i, pixels.Color(255, 255, 255)); 
      pixels.show();
      delay(100);
    }
}
double getPeak(){
  for(int i=0; i<SAMPLES; i++)
      {
          microSeconds = micros();    //Returns the number of microseconds since the Arduino board began running the current script. 
       
          vReal[i] = analogRead(S_PIN); //Reads the value from analog pin, quantize it and save it as a real term.
          vImag[i] = 0; //Makes imaginary term 0 always
  
          /*remaining wait time between samples if necessary*/
          while(micros() < (microSeconds + samplingPeriod))
          {
            //do nothing
          }
      }
   
      /*Perform FFT on samples*/
      FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
      FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

      /*Find the most dominant frequency*/
      double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  
      return (peak);
}
