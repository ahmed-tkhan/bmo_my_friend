#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <Arduino.h>

// Image data from Good Display sample code (Ap_29demo.h)
// Used for 4-level grayscale display test

// Image 1 (5000 bytes)
const unsigned char gImage_1[] PROGMEM = {
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
// ... (rest of image data)
};

// Image 2 (5000 bytes)
const unsigned char gImage_2[] PROGMEM = {
0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
// ... (rest of image data)
};

#endif // IMAGE_DATA_H
