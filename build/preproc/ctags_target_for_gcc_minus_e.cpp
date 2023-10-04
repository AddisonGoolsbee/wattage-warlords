# 1 "/Users/addisongoolsbee/Desktop/class/CPSC-334/interactiveDevices/led-game/led-game.ino"
// #define BUTTON_PIN 33 
// #define SWITCH_PIN 25

// #define VRX_PIN  36 // ESP32 pin GPIO36 (ADC0) connected to VRX pin
// #define VRY_PIN  39 // ESP32 pin GPIO39 (ADC0) connected to VRY pin

// int valueX = 0; // to store the X-axis value
// int valueY = 0; // to store the Y-axis value


// void setup() {
//   Serial.begin(9600);
//   // initialize the pushbutton pin as an pull-up input
//   pinMode(BUTTON_PIN, INPUT_PULLUP);
//   pinMode(SWITCH_PIN, INPUT_PULLUP);

// }


// void loop() {
//   Serial.print("Button: ");
//   Serial.print(digitalRead(BUTTON_PIN));
//   Serial.print("    Switch: ");
//   Serial.println(digitalRead(SWITCH_PIN));

//   valueX = analogRead(VRX_PIN);
//   valueY = analogRead(VRY_PIN);
//   Serial.print("x = ");
//   Serial.print(valueX);
//   Serial.print(", y = ");
//   Serial.println(valueY);
//   delay(200);
// }

/*
* start mode: leds are off, middle color leds are white. Both players hold button to start the game, at which the controller leds count down 3 2 1
- pins start at 0
- just one player to start
- pushing the button makes the counter increment
- pusing the button flashes the led that's incrementing
- counter translates to front 3 leds being lit
* when counter gets to certain threshhold, game is over. turn leds off, Flash winner leds and then reset
- add switch: if controller switch != user switch, decrease score instead of increasing it
- some sort of negative feedback for bad switch and button press
- controller switch randomly changes every so often
- add joystick: if controller color ~!= user color, decrease score
- map joystick axes to red/blue and green
- figure out threshholds for color correct and fine tune a lot
- randomly change the controller joystick color
- add two users
*/

int ledPin = 32; // The LED is connected to digital pin 9
int brightness = 0; // Initialize the brightness value
int fadeAmount = 5; // Rate of brightness change

void setup() {
  Serial.begin(9600);
  Serial.println("Hi");
  pinMode(ledPin, 0x03); // Set the LED pin as an output
}

void loop() {
  Serial.println("Button: ");

  analogWrite(ledPin, brightness); // Set the LED brightness

  brightness = brightness + fadeAmount; // Increase brightness

  // If brightness reaches the maximum (255), change direction
  if (brightness == 255 || brightness == 0) {
    fadeAmount = -fadeAmount; // Change direction
    delay(500); // Delay for a moment at the maximum and minimum brightness
  }

  delay(30); // Adjust the delay to control the speed of the oscillation
}
