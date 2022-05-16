/* 
Frank Bender
Prizm software testing
*/

#ifndef PRINTCOLOR_H_ 
#define PRINTCOLOR_H_

#include <stdio.h>

void red();
void blue();
void yellow();
void green();
void cyan();
void purple();
void magenta();
void black();
void white();
void grey();
void lred();
void lgreen();
void lyellow();
void lblue();
void lpurple();
void lmagenta();
void lcyan();
void lwhite();

void redbg();
void bluebg();
void yellowbg();
void greenbg();
void cyanbg();
void purplebg();
void magentabg();
void blackbg();
void whitebg();
void greybg();
void lredbg();
void lgreenbg();
void lyellowbg();
void lbluebg();
void lpurplebg();
void lmagentabg();
void lcyanbg();
void lwhitebg();

void bold();
void underscore();
void blink();
void reverse();
void conceal();
void clearcolor();

void br();

#define RED(a) red(); printf(a); clearcolor();
#define BLU(a) blue(); printf(a); clearcolor();
#define YEL(a) yellow(); printf(a); clearcolor();
#define MAG(a) magenta(); printf(a); clearcolor();
#define GRE(a) green(); printf(a); clearcolor();
#define CYA(a) cyan(); printf(a); clearcolor();
#define WHI(a) white(); printf(a); clearcolor();
#define GRY(a) grey(); printf(a); clearcolor();
#define BLA(a) black(); printf(a); clearcolor();

#define BRED(a) red(); bold(); printf(a); clearcolor();
#define BBLU(a) blue(); bold(); printf(a); clearcolor();
#define BYEL(a) yellow(); bold(); printf(a); clearcolor();
#define BMAG(a) magenta(); bold(); printf(a); clearcolor();
#define BGRE(a) green(); bold(); printf(a); clearcolor();
#define BCYA(a) cyan(); bold(); printf(a); clearcolor();
#define BWHI(a) white(); bold(); printf(a); clearcolor();
#define BGRY(a) grey(); bold(); printf(a); clearcolor();
#define BBLA(a) black(); bold(); printf(a); clearcolor();

#define REDV(a) red(); a; clearcolor();
#define BLUV(a) blue(); a; clearcolor();
#define YELV(a) yellow(); a; clearcolor();
#define MAGV(a) magenta(); a; clearcolor();
#define GREV(a) green(); a; clearcolor();
#define CYAV(a) cyan(); a; clearcolor();
#define WHIV(a) white(); a; clearcolor();
#define GRYV(a) grey(); a; clearcolor();
#define BLAV(a) black(); a; clearcolor();

#define BREDV(a) red(); bold(); a; clearcolor();
#define BBLUV(a) blue(); bold(); a; clearcolor();
#define BYELV(a) yellow(); bold(); a; clearcolor();
#define BMAGV(a) magenta(); bold(); a; clearcolor();
#define BGREV(a) green(); bold(); a; clearcolor();
#define BCYAV(a) cyan(); bold(); a; clearcolor();
#define BWHIV(a) white(); bold(); a; clearcolor();
#define BGRYV(a) grey(); bold(); a; clearcolor();
#define BBLAV(a) black(); bold(); a; clearcolor();

void example();

#endif
