/*
TODO

- pins start at 0, just one player to start
- pushing the button makes the counter increment
- pusing the button flashes the led that's incrementing
- counter translates to front 3 leds being lit
- add switch: if controller switch != user switch, decrease score instead of increasing it
- some sort of negative feedback for bad switch and button press
- controller switch randomly changes every so often
- add joystick: if controller color ~!= user color, decrease score
- map joystick axes to red/blue and green
- figure out threshholds for color correct and fine tune a lot
- randomly change the controller joystick color
- add two users

* start mode: leds are off, middle color leds are white. Both players hold button to start the game, at which the controller leds count down 3 2 1
* when counter gets to certain threshhold, game is over. turn leds off, Flash winner leds and then reset

*/

#define BUTTON_PIN_P1 21
#define BUTTON_PIN_P2 -1

#define CHARGE_PIN_P1_1 32
#define CHARGE_PIN_P1_2 33
#define CHARGE_PIN_P1_3 25

#define CHARGE_PIN_P2_1 -1
#define CHARGE_PIN_P2_2 -1
#define CHARGE_PIN_P2_3 -1

#define DEBOUNCE_TIME 25
#define SCORE_MAX 192

int LED_MAX = SCORE_MAX / 3;


int scoreP1 = 0;
int scoreP2 = 0;

int lastStateP1 = HIGH;
int currentStateP1; 
int lastSteadyStateP1 = LOW; 
int lastFlickerableStateP1 = LOW; 
unsigned long lastDebounceTimeP1 = 0;  

int lastStateP2 = HIGH;
int currentStateP2; 
int lastSteadyStateP2 = LOW; 
int lastFlickerableStateP2 = LOW; 
unsigned long lastDebounceTimeP2 = 0;  

int ledPin = 32; // The LED is connected to digital pin 9
int brightness = 0; // Initialize the brightness value
int fadeAmount = 5; // Rate of brightness change


void handleButton(int player){
  if (player == 1){
     currentStateP1 = digitalRead(BUTTON_PIN_P1);

    if (currentStateP1 != lastFlickerableStateP1) {
      lastDebounceTimeP1 = millis();
      lastFlickerableStateP1 = currentStateP1;
    }

    if ((millis() - lastDebounceTimeP1) > DEBOUNCE_TIME) {
      if(lastSteadyStateP1 == HIGH && currentStateP1 == LOW) {
        Serial.print("P1 press");
        Serial.println(scoreP1);
        scoreP1++;
        setCharge(1);
      }
      lastSteadyStateP1 = currentStateP1;
    }
    lastStateP1 = currentStateP1;
  } else {
    currentStateP2 = digitalRead(BUTTON_PIN_P2);

    if (currentStateP2 != lastFlickerableStateP2) {
      lastDebounceTimeP2 = millis();
      lastFlickerableStateP2 = currentStateP2;
    }

    if ((millis() - lastDebounceTimeP2) > DEBOUNCE_TIME) {
      if(lastSteadyStateP2 == HIGH && currentStateP2 == LOW) {
        Serial.print("P2 press");
        Serial.println(scoreP2);
        scoreP2++;
      }
      lastSteadyStateP2 = currentStateP2;
    }
    lastStateP2 = currentStateP2;
  }
}

void setCharge(int player) {
  int score = player == 1 ? scoreP1 : scoreP2;
  int pin = -1;

  Serial.println(score * 4);
  if (score > LED_MAX * 2) {
    score = score - (LED_MAX * 2);
    pin = player == 1 ? CHARGE_PIN_P1_3 : CHARGE_PIN_P2_3;
  } else if (score > LED_MAX) {
    score = score - LED_MAX;
    pin = player == 1 ? CHARGE_PIN_P1_2 : CHARGE_PIN_P2_2;
  } else {
    pin = player == 1 ? CHARGE_PIN_P1_1 : CHARGE_PIN_P2_1;
  }

  analogWrite(pin, score * 4);
}


void setup() {
  Serial.begin(9600);
  Serial.println("Hi");
  pinMode(BUTTON_PIN_P1, INPUT_PULLUP);
  pinMode(CHARGE_PIN_P1_1, OUTPUT);
  pinMode(CHARGE_PIN_P1_2, OUTPUT);
  pinMode(CHARGE_PIN_P1_3, OUTPUT);
}

void loop() {
  handleButton(1);
  
}

  // analogWrite(ledPin, brightness); // Set the LED brightness

  // brightness = brightness + fadeAmount; // Increase brightness

  // // If brightness reaches the maximum (255), change direction
  // if (brightness == 255 || brightness == 0) {
  //   fadeAmount = -fadeAmount; // Change direction
  //   delay(500); // Delay for a moment at the maximum and minimum brightness
  // }

  // delay(5); // Adjust the delay to control the speed of the oscillation

