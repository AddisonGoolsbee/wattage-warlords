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
- add switch: if controller switch != user switch, decrease score instead of increasing it
- some sort of negative feedback for bad switch and button press
- controller switch randomly changes every so often
- add joystick: if controller color ~!= user color, decrease score
- map joystick axes to red/blue and green
- figure out threshholds for color correct and fine tune a lot
- randomly change the controller joystick color
- add two users

b sometimes when switching to the next led, the previous one turns off

* pwb on leds
* start mode: leds are off, middle color leds are white. Both players hold button to start the game, at which the controller leds count down 3 2 1
* ability to restart game after it's ended/at any point, without using reset button on esp32

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

bool animationInProgress = false;
unsigned long animationStartTime = 0;

int winner = 0;

int P1_SWITCH_val;
int P2_SWITCH_val;
int C_SWITCH_val = LOW;

void handleButton(int player){
  if (player == 1){
     currentStateP1 = digitalRead(P1_BUTTON);

    if (currentStateP1 != lastFlickerableStateP1) {
      lastDebounceTimeP1 = esp_timer_get_time();
      lastFlickerableStateP1 = currentStateP1;
    }

    if ((esp_timer_get_time() - lastDebounceTimeP1) > DEBOUNCE_TIME) {
      if(lastSteadyStateP1 == HIGH && currentStateP1 == LOW) {
        Serial.print("P1 press ");
        Serial.println(scoreP1);
        scoreP1 += P1_multiplier;
        setCharge(1);
      }
      lastSteadyStateP1 = currentStateP1;
    }
    lastStateP1 = currentStateP1;
  } else {
    currentStateP2 = digitalRead(P2_BUTTON);

    if (currentStateP2 != lastFlickerableStateP2) {
      lastDebounceTimeP2 = esp_timer_get_time();
      lastFlickerableStateP2 = currentStateP2;
    }

    if ((esp_timer_get_time() - lastDebounceTimeP2) / 1000 > DEBOUNCE_TIME) {
      if(lastSteadyStateP2 == HIGH && currentStateP2 == LOW) {
        Serial.print("P2 press");
        Serial.println(scoreP2);
        scoreP2 += P2_multiplier;
      }
      lastSteadyStateP2 = currentStateP2;
    }
    lastStateP2 = currentStateP2;
  }
}

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

  if (score > LED_MAX * 2) {
    score = score - (LED_MAX * 2);
    pin = player == 1 ? P1_GREEN_3 : P2_GREEN_3;
  } else if (score > LED_MAX) {
    score = score - LED_MAX;
    pin = player == 1 ? P1_GREEN_2 : P1_GREEN_2;
  } else {
    pin = player == 1 ? P1_GREEN_1 : P2_GREEN_1;
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

int getSwitch(int player){
  int switch_pin = player == 1 ? P1_SWITCH : P2_SWITCH;
  int switch_pin_val;

  // Flips mode to input, reads the stte
  pinMode(switch_pin , INPUT_PULLUP);
  switch_pin_val = digitalRead(switch_pin); // Read the switch status

  pinMode(switch_pin, OUTPUT);
  digitalWrite(switch_pin, HIGH); // Turn on the LED

  return switch_pin_val;
  
}

void handleJoystick(int player){
  if (player == 1) {
    int P1_VRX_val = analogRead(P1_VRX);
    int P1_VRY_val = analogRead(P1_VRY);

    if (P1_RGB_B_val != 0 && P1_VRX_val == 4095) { 
        P1_RGB_B_val -= 1;  // remove 1 from P1_RGB_B_val
      }
    else if (P1_RGB_B_val != 255 && P1_VRX_val == 0) {
        P1_RGB_B_val += 1;  // add 1 to P1_RGB_B_val
      }

    if (P1_RGB_R_val != 0 && P1_VRY_val == 4095) { 
        P1_RGB_R_val -= 1;  // remove 1 from P1_RGB_R_val
      }
    else if (P1_RGB_R_val != 255 && P1_VRY_val == 0) {
        P1_RGB_R_val += 1;  // add 1 to P1_RGB_R_val
      }
    analogWrite(P1_RGB_R, P1_RGB_R_val);
    analogWrite(P1_RGB_B, P1_RGB_B_val);
  }
  else if (player == 2) {
    int P2_VRX_val = analogRead(P2_VRX);
    int P2_VRY_val = analogRead(P2_VRY);

    if (P2_RGB_B_val != 0 && P2_VRX_val == 4095) { 
        P2_RGB_B_val -= 1;  // remove 1 from P1_RGB_B_val
      }
    else if (P2_RGB_B_val != 255 && P2_VRX_val == 0) {
        P2_RGB_B_val += 1;  // add 1 to P1_RGB_B_val
      }

    if (P2_RGB_R_val != 0 && P2_VRY_val == 4095) { 
        P2_RGB_R_val -= 1;  // remove 1 from P1_RGB_R_val
      }
    else if (P2_RGB_R_val != 255 && P2_VRY_val == 0) {
        P2_RGB_R_val += 1;  // add 1 to P1_RGB_R_val
      }
    analogWrite(P2_RGB_R, P2_RGB_R_val);
    analogWrite(P2_RGB_B, P2_RGB_B_val);
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
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting up...");
  pinMode(P1_BUTTON, INPUT_PULLUP);
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
}

void loop() {
  checkMatches();
  handleButton(1);
  handleButton(2);
  flashLED(1);
  flashLED(2);
  handleJoystick(1);
  handleJoystick(2);
  unsigned long currentMillis = millis();
  randomActions(currentMillis);

  // print out the value you read:
  Serial.println(P1_SWITCH_val);

  if (winner) {
    gameFinish();
  }
}
