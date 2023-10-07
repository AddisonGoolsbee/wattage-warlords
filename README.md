## Devlopment Setup

To get the device running on VSCode, you'll need to configure a couple of Arduino tools
- Download the official Microsoft Arduino extension in vscode, and click accept when it asks you to switch from the deprecated version
- Type CMD+Shift+P and type `C/C++: Select a Confuguration`, then choose `Arduino`
- Type CMD+Shift+P and type `Arduino: Select Serial Port`, then choose the port the device is connected to
- Upload code to the Arduino with CMD+Shift+P and `Arduino: Upload`

# Overview

[GitHub - AddisonGoolsbee/wattage-warlords: CPSC 334 interactive devices unit project](https://github.com/AddisonGoolsbee/wattage-warlords)

**Wattage Warriors** is a physical, electronic game set in a post-apocalyptic world where players (warlords) compete to be the first to charge up their batteries. After all, in the age of apocalypse, energy is king.

You and your enemy have taken your squabble to the wattage war room, and only through careful observation and unfathomable grit will you make it out alive, batteries charged. Brows will fill with sweat, betrayals will be had, and in the end, only one will claim all the charge. The time is now to fight for your faction. There is no time to wait!

# How to Play

The goal of **Wattage Warriors** is to light up all three of your green LEDs before your opponent does. This is accomplished primarily through button mashing. However, as the environment changes, indicated by the two middle LEDs, you must adjust your environment to match, both by ensuring your switch state stays the same, and that your color LED roughly matches the environment’s. In addition, there is a reset button.

Insert video here

# Design

One fateful night, two budding warlords, Omega Smelter and Epsilon Smelter-Smelter, sat in the wattage war room and settled on a compromise. To put a stop to the needless post-apocalyptic bloodshed in the name of energy, a competition was schemed…

When designing **Wattage Warlords**, we wanted to make a highly interactive, highly competitive game, and we thought the best way to start was with a good old-fashioned button masher. Output was a concern to us. We wanted an entirely closed system, and while we weren’t able to overcome the power source from a computer, we decided to challenge ourselves to make the only feedback be in the form of LEDs. Thinking some more, we wanted to find ways to make the button mashing more interesting—things that happen while you’re button mashing that require your attention. We designed an easier distraction—the switch, and a more difficult distraction—the joystick, which mapped to a color.

A player wins Wattage Warlords by filling three 64kWh batteries from the central power source before their opponent. A kWh of energy is harnessed by pressing the button. However, spamming the button is not a viable strategy. Every few seconds, the status of the orange “AC” LED toggles, shifting the energy source from DC to AC, then back to DC when the light toggles off again. If a player tries to harness energy while the source is on DC but they are on AC, they will damage their own equipment and lose 1 kWh from their battery.

Similarly, the power source temperature varies wildly throughout the game, indicated by a magenta LED in the center of the board. Players must match their battery temperature (indicated by their own magenta LED) to the power source’s temperature when harnessing energy from it, or risk damaging their equipment and losing 1 kWh from their battery. The heat source can be adjusted by moving the joystick to the right, and the coolant can be adjusted by moving the joystick up. In practice, this means that moving the joystick to the right increases the amount of red emitted while moving the joystick up increases the amount of blue emitted. The goal is to match the central LED in both red and blue emissions. An aside: we initially were going to do red/green, but decided to refrain from disadvantaging red/green colorblind warlords.

Three green LEDs / battery capacity for each player reveal the player’s battery capacity, where the first battery is fully charged before moving to the second.

# Electronics

The electronics for this thing were a nightmare

We used nearly every pin possible on the ESP32. We connected three RGB LEDs, 3 yellow LEDs, 6 green LEDs, 3 buttons, two switches, and 2 analog sticks, totaling to 26 separate pin connections after many optimizations. Many times we would find after an hour of debugging, that a certain GPIO pin didn’t actually support input, or that one pin’s output conflated another (particularly with the joysticks). Most wires had to go from female to male, which meant that we (since there weren’t any m-f wires in the CEID), had to combine f-f to m-m wires for most connections we made. Additionally, because **Wattage Warlords** is such a violent game, we had to make sure everything was secured, meaning many of our enormous number of connections had to be soldered. There has to be a better way to organize wires; we spent some time thinking about it but weren’t able to figure any system out. I wonder how on Earth one would deal with larger contraptions.

We made several iterations of diagrams to help us keep track of which pins connected to what, and how the circuit actually looked. This is the final version we created:

# Code

https://github.com/AddisonGoolsbee/wattage-warlords

Our code is about 500 lines long, and while there isn’t anything particularly special about it, it was particularly challenging to implement asynchronous events like animations, while keeping the rest of the program flowing. Additionally, we ran into latency problems at multiple junctures.

Ultimately what made the code so difficult is that when debugging, we would have a very hard time discerning if the problem was with the code, with the hardware, or with both.

One takeaway from the code, and something that we only started to do partway through, was that object-oriented programming is actually really useful! If only we had started with it, then all of our edge cases and debugging of the two players could’ve been so much easier.

Because of the many segmented aspects of the game, we were able to effectively divide up our work and 

# Enclosure

Lesson #2: use cardboard

Aside from the sheer volume of material and the constant unplugging of wires and breaking of pieces and failure of materials in the ceid, the hardest part of making the enclosure was getting all the buttons and lights out on the top of the enclosure, so stable that vigorous button mashing would not break the system. We settled on a haphazard system of drilling moles into the top of just the right size, cutting wood platforms, and hot gluing the wood platforms to the sensor and to the inner wall of the enclosure. Many failures later, and I mean ****many****, we finished. Even though the least happened with this part, this probably took the most time of any other section. The end result is a device that looks unbelievably clean, however. It really distinguishes the amature warlord from the experienced one

# Conclusion

The project took a total of around 20-25 hours, and that’s not including the fact that there were *two* of us. In hindsight, we now know that leaning so heavily on the hardware side is NOT the right choice to make if you think your time is valuable. We were plagued by constant hardware, software-hardware, and enclosure bugs, near-fatal catastrophes, and suffered too much head trauma from banging against walls and tables. One particular constraint we faced was that we approached the physical limit of how many inputs/outputs we had to the ESP32 board, such that we started having to use more and more questionable pins, and had to sacrifice features we wanted implemented.

While we’re very happy with the result, if we were to start this project again, we would definitely choose something more on the software side of things. I personally cannot wait to challenge my friends to a fine gentlemanly game of **Wattage Warlords**, but at what cost?
