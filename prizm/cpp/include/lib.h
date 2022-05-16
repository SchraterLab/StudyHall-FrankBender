/* 
Frank Bender
Prizm software testing (Python port)
5-5-2022: May make a c only version of this file to support portability 
*/

#ifndef LIB_H_
#define LIB_H_

#include <stdio.h>
#include "printcolor.h"

extern "C" void wonder() {
    blue();
    printf("Wonderful!\n");
    clearcolor();
}

#endif