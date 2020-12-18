//
//  Spiral.cpp
//  GauntletII
//
//  Created by Robert Atkins on 26/09/13.
//  Copyright (c) 2013 Robert Atkins. All rights reserved.
//

#import "Effect.h"

class Spiral : public Effect {
    
public:
    
    Spiral(CRGB *leds, int width, int height ) : Effect(leds, width, height ) {
    }
    
    void start() {
        for (uint16_t time = 0, cycles = 0; cycles < 2048; time += 128, cycles++) {
            calcFrame(time);
        }
    }
//    sin(distance(x, y, (128 * sin(-t) + 128), (128 * cos(-t) + 128)) / 40.74)s
//     v = sin(10 * (x * sin(time / 2) + y * cos(time / 3)) + time)
    
    void calcFrame(int time) {
        static byte offset  = 0; // counter for radial color wave motion
        static int plasVector = 0; // counter for orbiting plasma center
         // Calculate current center of plasma pattern (can be offscreen)
//        int xOffset = cos8(plasVector / 256);
//        int yOffset = sin8(plasVector / 256);

        int xOffset = cos8(plasVector / 64);
        int yOffset = sin8(plasVector / 64);

//        int xOffset = (cos8(plasVector)-127)/2;
//        int yOffset = (sin8(plasVector)-127)/2;
      
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
              byte color = sin8(sqrt(sq(((float)x - 7.5) * 10 + xOffset - 127) + sq(((float)y - 2) * 10 + yOffset - 127)) + offset);
//                Serial.print(color);
//                Serial.println("");
                pixel(x, y) = CHSV(color, 255, 255);
            }
        }
//        LEDS.show();
        offset++; // wraps at 255 for sin8
        plasVector += 16; // using an int for slower orbit (wraps at 65536)
        FastLED.show();
        FastLED.delay(1000/120);
    }
    
//    uint8_t sin8(uint16_t x) {
//        return (sin16(x) >> 8) + 128;
//    }
//    
//    uint8_t cos8(uint16_t x) {
//        return (cos16(x) >> 8) + 128;
//    }

};
