/* 
Frank Bender
Prizm software testing
*/

#ifndef PRINTCOLOR_H_ 
#define PRINTCOLOR_H_

#include <stdio.h>

void red() { printf("\033[0;31m"); }
void blue() { printf("\033[0;34m"); }
void yellow() { printf("\033[0;33m"); }
void green() { printf("\033[0;32m"); }
void cyan() { printf("\033[0;36m"); }
void purple() { printf("\033[0;35m"); }
void magenta() { printf("\033[0;35m"); }
void black() { printf("\033[0;30m"); }
void white() { printf("\033[0;37m"); }
void grey() { printf("\033[0;90m"); }
void lred() { printf("\033[0;91m"); }
void lgreen() { printf("\033[0;92m"); }
void lyellow() { printf("\033[0;93m"); }
void lblue() { printf("\033[0;94m"); }
void lpurple() { printf("\033[0;95m"); }
void lmagenta() { printf("\033[0;95m"); }
void lcyan() { printf("\033[0;96m"); }
void lwhite() { printf("\033[0;97m"); }

void redbg() { printf("\033[0;41m"); }
void bluebg() { printf("\033[0;44m"); }
void yellowbg() { printf("\033[0;43m"); }
void greenbg() { printf("\033[0;42m"); }
void cyanbg() { printf("\033[0;46m"); }
void purplebg() { printf("\033[0;45m"); }
void magentabg() { printf("\033[0;45m"); }
void blackbg() { printf("\033[0;40m"); }
void whitebg() { printf("\033[0;47m"); }
void greybg() { printf("\033[0;100m"); }
void lredbg() { printf("\033[0;101m"); }
void lgreenbg() { printf("\033[0;102m"); }
void lyellowbg() { printf("\033[0;103m"); }
void lbluebg() { printf("\033[0;104m"); }
void lpurplebg() { printf("\033[0;105m"); }
void lmagentabg() { printf("\033[0;105m"); }
void lcyanbg() { printf("\033[0;106m"); }
void lwhitebg() { printf("\033[0;107m"); }

void bold() { printf("\033[1m"); }
void underscore() { printf("\033[4m"); }
void blink() { printf("\033[5m"); }
void reverse() { printf("\033[7m"); }
void conceal() { printf("\033[8m"); }
void clearcolor() { printf("\033[0m"); }

void br() { printf("\n"); }

void example() { printf("Holy guacamole\n"); }
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
