/*

P1 Button @ GPIO23/23
P2 Button @ GPIO22/22
P1 Switch @ GPIO17/17
P2 Switch @ GPIO16/16
P1 White @ GPIO21/21
P2 White @ GPIO19/19
C White @ GPIO18/18
P1 VRX @ GPIO36/VP
P1 VRY @ GPIO39/VN
P2 VRX @ GPIO34/34
P2 VRY @ GPIO35/35
C RGB R + ~68ohm @ GPIO32/32
C RGB G + ~1kohm @ GPIO33/33
P1 RGB R + ~68ohm @ GPIO25/25
P1 RGB G + ~1kohm @ GPIO26/26
P2 RGB R + ~68ohm @ GPIO27/27
P2 RGB G + ~1kohm @ GPIO14/14
P1 Green 1 @ GPIO12/12
P1 Green 2 @ GPIO13/13
P1 Green 3 @ GPIO4/4
P1 Green 1 @ GPIO0/0
P1 Green 2 @ GPIO2/2
P1 Green 3 @ GPIO15/15

P
*/
/*
TODO

x pins start at 0, just one player to start
x pushing the button makes the counter increment
x pusing the button flashes the led that's incrementing
x counter translates to front 3 leds being lit
x debounce leds
x when counter gets to certain threshhold, game is over. turn leds off, Flash winner leds and then reset
x ability to restart game after it's ended/at any point, without using reset button on esp32
- add switch: if controller switch != user switch, decrease score instead of increasing it
- some sort of negative feedback for bad switch and button press
- controller switch randomly changes every so often
- add joystick: if controller color ~!= user color, decrease score
- map joystick axes to red/blue and green
- figure out threshholds for color correct and fine tune a lot
- randomly change the controller joystick color
- add two users

* pwb on leds
* start mode: leds are off, middle color leds are white. Both players hold button to start the game, at which the controller leds count down 3 2 1
*/

const int P1_BUTTON = 23; // GPIO23
const int P2_BUTTON = 22; // GPIO22

// Switches
const int P1_SWITCH = 5; // GPIO17 +20ohm
const int P2_SWITCH = 16; // GPIO16 + 20ohm
const int C_SWITCH = 18;  // GPIO18 + 20ohm

// VRX and VRY for Joysticks
// -VRX = deltaY
// -VRY = deltaX
// 5V to 3.3V
const int P1_VRX = 36; // GPIO36/VP
const int P1_VRY = 39; // GPIO39/VN
const int P2_VRX = 34; // GPIO34
const int P2_VRY = 35; // GPIO35

// RGB LEDs
const int C_RGB_R = 32; // GPIO32 with ~68ohm resistor
const int C_RGB_B = 33; // GPIO33 with ~1kohm resistor
const int P1_RGB_R = 25; // GPIO25 with ~68ohm resistor
const int P1_RGB_B = 26; // GPIO26 with ~1kohm resistor
const int P2_RGB_R = 27; // GPIO27 with ~68ohm resistor
const int P2_RGB_B = 14; // GPIO14 with ~1kohm resistor

// Green LEDs for Player 1
const int P1_GREEN_1 = 12; // GPIO12
const int P1_GREEN_2 = 13; // GPIO13
const int P1_GREEN_3 = 4;  // GPIO4
const int P2_GREEN_1 = 0;  // GPIO0
const int P2_GREEN_2 = 2;  // GPIO2
const int P2_GREEN_3 = 15; // GPIO15

const int DEBOUNCE_TIME = 200;
const int SCORE_MAX = 192;
const int LED_MAX = SCORE_MAX / 3;
const int ANIMATION_DURATION = 20;

int P1_RGB_B_val = 0;
int P1_RGB_R_val = 0;
int P2_RGB_B_val = 0;
int P2_RGB_R_val = 0;
int C_RGB_R_val = 255;
int C_RGB_B_val = 255;
unsigned long previousMillisRGB = 0;
unsigned long previousMillisSwitch = 0;
long interval_RGB = random(8000, 15001);  // Random interval between 8 and 15 seconds
long interval_switch = random(8000, 15001);  // Random interval between 8 and 15 seconds
bool P1_RGB_matched = true;
bool P2_RGB_matched = true;
bool P1_W_matched = true;
bool P2_W_matched = true;

int P1_multiplier = 1;
int P2_multiplier = 1;

int scoreP1 = 0;
int scoreP2 = 0;
int lastStateP1 = HIGH;
int currentStateP1; 
int lastSteadyStateP1 = LOW; 
int lastFlickerableStateP1 = LOW; 
unsigned long lastDebounceTimeP1 = 0;  

bool animationInProgressP1 = false;
unsigned long animationStartTimeP1 = 0;
bool animationInProgressP2 = false;
unsigned long animationStartTimeP2 = 0;

int winner = 0;

int P1_SWITCH_val;
int P2_SWITCH_val;
int C_SWITCH_val = LOW;

class Debouncer {
public:
  Debouncer(int pin) : pin(pin) {}
  
  bool debounce() {
    int currentState = digitalRead(pin);
    
    int now = esp_timer_get_time();
    if (currentState != lastFlickerableState) {
      if (pin == P2_BUTTON) {

      }
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


Debouncer buttonP1(P1_BUTTON);
Debouncer buttonP2(P2_BUTTON);


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
      pins[0] = P1_GREEN_1;
      pins[1] = P1_GREEN_2;
      pins[2] = P1_GREEN_3;
  } else {
      pins[0] = P2_GREEN_1;
      pins[1] = P2_GREEN_2;
      pins[2] = P2_GREEN_3;
  }
  return pins;
}

int getPin(int player) {
  int score = player == 1 ? scoreP1 : scoreP2;
  int pin = -1;

  if (score == LED_MAX) {
    analogWrite(player == 1 ? P1_GREEN_1 : P2_GREEN_1, 255);
  }

  if (score > LED_MAX * 2) {
    pin = player == 1 ? P1_GREEN_3 : P2_GREEN_3;
    analogWrite(player == 1 ? P1_GREEN_2 : P2_GREEN_2, 255);
    analogWrite(player == 1 ? P1_GREEN_1 : P2_GREEN_1, 255);
  } else if (score > LED_MAX) {
    pin = player == 1 ? P1_GREEN_2 : P2_GREEN_2;
  } else {
    pin = player == 1 ? P1_GREEN_1 : P2_GREEN_1;
  }
  return pin;
}

void setCharge(int player) {
  int score = getScore(player);
  int pin = getPin(player);

  if (player == 1) {
    animationInProgressP1 = true;
    animationStartTimeP1 = esp_timer_get_time();
  } else {
    animationInProgressP2 = true;
    animationStartTimeP2 = esp_timer_get_time();
  }

  analogWrite(pin, 0);

  int fullScore = player == 1 ? scoreP1 : scoreP2;
  if (fullScore >= SCORE_MAX){
    winner = player;
  }
}

void flashLED(int player) {
  int pin = getPin(player);
  int score = getScore(player);

  if (player == 1 ? animationInProgressP1 : animationInProgressP2) {
    unsigned long currentTime = esp_timer_get_time();
    int time = (currentTime - (player == 1 ? animationStartTimeP1 : animationStartTimeP2)) / 1000;
    if (time > ANIMATION_DURATION) {
      if (player == 1) {
        animationInProgressP1 = false;
      } else {
        animationInProgressP2 = false;
      }
      analogWrite(pin, score * 4);
    }
  }
}

void handleButton(){
  if (buttonP2.debounce()) {
    Serial.print("P2 button ");
    Serial.println(scoreP2);
    scoreP2 = max(scoreP2 + P2_multiplier, 0);
    setCharge(2);
  } 
  
  if (buttonP1.debounce()) {
    Serial.print("P1 button ");
    Serial.println(scoreP1);
    scoreP1 = max(scoreP1 + P1_multiplier, 0);
    setCharge(1);
  } 

  flashLED(1);
  flashLED(2);
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

void handleJoystick(int player){
  static unsigned long lastUpdate1 = 0;
  static unsigned long lastUpdate2 = 0;
  unsigned long currentMillis = millis();

  if (player == 1 && currentMillis - lastUpdate1 >= 10) { // Adjust delay as needed
    int P1_VRX_val = analogRead(P1_VRX);
    int P1_VRY_val = analogRead(P1_VRY);

    if (P1_RGB_B_val != 0 && P1_VRX_val == 4095) { 
      P1_RGB_B_val -= 3;
      analogWrite(P1_RGB_B, P1_RGB_B_val);
    } else if (P1_RGB_B_val != 255 && P1_VRX_val == 0) {
      P1_RGB_B_val += 3;
      analogWrite(P1_RGB_B, P1_RGB_B_val);
    }

    if (P1_RGB_R_val != 0 && P1_VRY_val == 4095) { 
      P1_RGB_R_val -= 3;
      analogWrite(P1_RGB_R, P1_RGB_R_val);
    } else if (P1_RGB_R_val != 255 && P1_VRY_val == 0) {
      P1_RGB_R_val += 3;
      analogWrite(P1_RGB_R, P1_RGB_R_val);
    }
    lastUpdate1 = currentMillis;

  } else if (player == 2 && currentMillis - lastUpdate2 >= 10) { // Adjust delay as needed
    int P2_VRX_val = analogRead(P2_VRX);
    int P2_VRY_val = analogRead(P2_VRY);

    if (P2_RGB_B_val != 0 && P2_VRX_val == 4095) { 
      P2_RGB_B_val -= 3;
      analogWrite(P2_RGB_B, P2_RGB_B_val);
    } else if (P2_RGB_B_val != 255 && P2_VRX_val == 0) {
      P2_RGB_B_val += 3;
      analogWrite(P2_RGB_B, P2_RGB_B_val);
    }

    if (P2_RGB_R_val != 0 && P2_VRY_val == 4095) { 
      P2_RGB_R_val -= 3;
      analogWrite(P2_RGB_R, P2_RGB_R_val);
    } else if (P2_RGB_R_val != 255 && P2_VRY_val == 0) {
      P2_RGB_R_val += 3;
      analogWrite(P2_RGB_R, P2_RGB_R_val);
    }
    lastUpdate2 = currentMillis;
  }
}


void randomActions(unsigned long currentMillis){
    if (currentMillis - previousMillisRGB >= interval_RGB) {
    previousMillisRGB = currentMillis;

    // Update C_RGB_R_val
    int new_val = random(0, 256);
    while (abs(new_val - C_RGB_R_val) < 50) {  // Ensure new value is sufficiently different
      new_val = random(0, 256);
    }
    C_RGB_R_val = new_val;
    analogWrite(C_RGB_R, C_RGB_R_val);

    // Update C_RGB_B_val
    new_val = random(0, 256);
    while (abs(new_val - C_RGB_B_val) < 50) {  // Ensure new value is sufficiently different
      new_val = random(0, 256);
    }
    C_RGB_B_val = new_val;
    analogWrite(C_RGB_B, C_RGB_B_val);

    interval_RGB = random(8000, 15001);
  }

  if (currentMillis - previousMillisSwitch >= interval_switch) {
    if (C_SWITCH_val == LOW) {
      C_SWITCH_val = HIGH;
    } else {
      C_SWITCH_val = LOW;
    }
    digitalWrite(C_SWITCH, C_SWITCH_val);
  }
}

void checkMatches(){
  if (P1_SWITCH_val == C_SWITCH_val) {
    P1_W_matched = true;
  }
  else {
    P1_W_matched = false;
  }

  if (P2_SWITCH_val == C_SWITCH_val) {
    P2_W_matched = true;
  }
  else {
    P2_W_matched = false;
  }

  if (P1_RGB_R_val > (C_RGB_R_val + 49) || P1_RGB_R_val < (C_RGB_B_val - 49)) {
    P1_RGB_matched = false;
  }
  else {
    P1_RGB_matched = true;
  }

  if (P1_W_matched == false && P1_RGB_matched == false) {
    P1_multiplier = -2;
  }
  else if (P1_W_matched == false || P1_RGB_matched == false) {
    P1_multiplier = -1;
  }
  else {
    P1_multiplier = 1;
  }

  if (P2_RGB_R_val > (C_RGB_R_val + 49) || P2_RGB_R_val < (C_RGB_B_val - 49)) {
    P2_RGB_matched = false;
  }
  else {
    P2_RGB_matched = true;
  }

  if (P2_W_matched == false && P2_RGB_matched == false) {
    P2_multiplier = -2;
  }
  else if (P1_W_matched == false || P1_RGB_matched == false) {
    P2_multiplier = -1;
  }
  else {
    P2_multiplier = 1;
  }
  P1_multiplier = 1;
  P2_multiplier = 1;
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting up...");
  pinMode(P1_BUTTON, INPUT_PULLUP);
  pinMode(P2_BUTTON, INPUT_PULLUP);
  pinMode(P1_GREEN_1, OUTPUT);
  pinMode(P1_GREEN_2, OUTPUT);
  pinMode(P1_GREEN_2, OUTPUT);
  pinMode(C_RGB_R, OUTPUT);
  pinMode(C_RGB_B, OUTPUT);
  pinMode(P1_RGB_R, OUTPUT);
  pinMode(P1_RGB_B, OUTPUT);
  pinMode(C_SWITCH, OUTPUT);
  analogWrite(C_RGB_R, C_RGB_R_val);
  analogWrite(C_RGB_B, C_RGB_B_val);
  digitalWrite(C_SWITCH, C_SWITCH_val);
}

void loop() {
  // checkMatches();
  handleButton();
  flashLED(1);
  flashLED(2);
  // handleJoystick(1);
  // handleJoystick(2);
  // handleSwitch(1);
  // handleSwitch(2);
  // unsigned long currentMillis = millis();
  // randomActions(currentMillis);

  if (winner) {
    gameFinish();
  }
}
