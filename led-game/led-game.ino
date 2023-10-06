/*
TODO

x pins start at 0, just one player to start
x pushing the button makes the counter increment
x pusing the button flashes the led that's incrementing
x counter translates to front 3 leds being lit
x debounce leds
x when counter gets to certain threshhold, game is over. turn leds off, Flash winner leds and then reset
- add switch: if controller switch != user switch, decrease score instead of increasing it
- some sort of negative feedback for bad switch and button press
- controller switch randomly changes every so often
- add joystick: if controller color ~!= user color, decrease score
- map joystick axes to red/blue and green
- figure out threshholds for color correct and fine tune a lot
- randomly change the controller joystick color
- add two users

b sometimes when switching to the next led, the previous one turns off
b sometimes when pressing the button, the led turns off for the current click

* pwb on leds
* start mode: leds are off, middle color leds are white. Both players hold button to start the game, at which the controller leds count down 3 2 1
* ability to restart game after it's ended/at any point, without using reset button on esp32

*/

#define BUTTON_PIN_P1 23
#define BUTTON_PIN_P2 -1

#define CHARGE_PIN_P1_1 32
#define CHARGE_PIN_P1_2 33
#define CHARGE_PIN_P1_3 25

#define CHARGE_PIN_P2_1 -1
#define CHARGE_PIN_P2_2 -1
#define CHARGE_PIN_P2_3 -1

#define SWITCH_PIN_CONTROLLER -1
#define SWITCH_PIN_P1 22
#define SWITCH_PIN_P2 -1

#define DEBOUNCE_TIME 200
#define SCORE_MAX 192
#define ANIMATION_DURATION 20

int LED_MAX = SCORE_MAX / 3;


int scoreP1 = 0;
int scoreP2 = 0;

bool animationInProgress = false;
unsigned long animationStartTime = 0;

int winner = 0;


class Debouncer {
public:
  Debouncer(int pin) : pin(pin) {}
  
  bool debounce() {
    int currentState = digitalRead(pin);
    
    int now = esp_timer_get_time();
    if (currentState != lastFlickerableState) {
      lastDebounceTime = now;
      lastFlickerableState = currentState;
    }

    bool res = false;

    if ((now - lastDebounceTime) > DEBOUNCE_TIME) {
      if(lastSteadyState == HIGH && currentState == LOW) {
        res = true;
      }
      lastSteadyState = currentState;
    }
    lastState = currentState;
    return res;
  }

private:
  int pin;
  unsigned long lastDebounceTime = 0;
  int lastState = HIGH;
  int lastSteadyState = LOW;
  int lastFlickerableState = LOW;
};


Debouncer buttonP1(BUTTON_PIN_P1);
Debouncer buttonP2(BUTTON_PIN_P2);


int getScore(int player) {
  int score = player == 1 ? scoreP1 : scoreP2;
  if (score > LED_MAX * 2) {
    score -= LED_MAX * 2;
  } else if (score > LED_MAX) {
    score -= LED_MAX;
  }
  return score;
}

int* getPins(int player) {
  int* pins = (int*) malloc(3 * sizeof(int));
  if (player == 1) {
      pins[0] = CHARGE_PIN_P1_1;
      pins[1] = CHARGE_PIN_P1_2;
      pins[2] = CHARGE_PIN_P1_3;
  } else {
      pins[0] = CHARGE_PIN_P2_1;
      pins[1] = CHARGE_PIN_P2_2;
      pins[2] = CHARGE_PIN_P2_3;
  }
  return pins;
}

int getPin(int player) {
  int score = player == 1 ? scoreP1 : scoreP2;
  int pin = -1;

  if (score > LED_MAX * 2) {
    score = score - (LED_MAX * 2);
    pin = player == 1 ? CHARGE_PIN_P1_3 : CHARGE_PIN_P2_3;
  } else if (score > LED_MAX) {
    score = score - LED_MAX;
    pin = player == 1 ? CHARGE_PIN_P1_2 : CHARGE_PIN_P2_2;
  } else {
    pin = player == 1 ? CHARGE_PIN_P1_1 : CHARGE_PIN_P2_1;
  }
  return pin;
}

void setCharge(int player) {
  int score = getScore(player);
  int pin = getPin(player);

  animationInProgress = true;
  animationStartTime = esp_timer_get_time();
  analogWrite(pin, 0);

  int fullScore = player == 1 ? scoreP1 : scoreP2;
  if (fullScore >= SCORE_MAX){
    winner = player;
  }
}

void flashLED(int player) {
  int pin = getPin(player);
  int score = getScore(player);

  if (animationInProgress) {
    unsigned long currentTime = esp_timer_get_time();
    int time = (currentTime - animationStartTime) / 1000;
    if (time > ANIMATION_DURATION) {
      animationInProgress = false;
      analogWrite(pin, score * 4);
    }
  }
}

bool getDigitalInput(int pin, int player) {

}

void handleButton(){
  if (buttonP1.debounce()) {
    Serial.print("P1 button ");
    Serial.println(scoreP1);
    scoreP1++;
    setCharge(1);
  } 

  if (buttonP2.debounce()) {
    Serial.print("P2 button ");
    Serial.println(scoreP2);
    scoreP2++;
    setCharge(2);
  } 

  flashLED(1);
  flashLED(2);
}

void handleSwitch(int player){

}


void gameFinish(){
  int* pins = getPins(winner);

  while (true) {
    for (int i=0; i<3; i++) {
      analogWrite(pins[i], 0);
    }
    delay(500);
    for (int i=0; i<3; i++) {
      analogWrite(pins[i], 255);
    }
    delay(500);
  }

  free(pins);
}



void setup() {
  Serial.begin(9600);
  Serial.println("Starting up...");
  pinMode(BUTTON_PIN_P1, INPUT_PULLUP);
  pinMode(CHARGE_PIN_P1_1, OUTPUT);
  pinMode(CHARGE_PIN_P1_2, OUTPUT);
  pinMode(CHARGE_PIN_P1_3, OUTPUT);
}

void loop() {
  handleButton();
  handleSwitch(1);

  if (winner) {
    gameFinish();
  }
}