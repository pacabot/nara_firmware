/**************************************************************************/
/*!
 @file     tone.h
 @author  BM (PACABOT)
 @date
 @version  0.0
 */
/**************************************************************************/
#ifndef __TONE_H__
#define __TONE_H__

#define c	261
#define d   294
#define e   329
#define f   349
#define g   391
#define gS  415
#define a   440
#define aS  455
#define b   466
#define cH  523
#define cSH 554
#define dH  587
#define dSH 622
#define eH  659
#define fH  698
#define fSH 740
#define gH  784
#define gSH 830
#define aH  880

#define C3 	(131 * 2)	// DO
#define C3H (139 * 2)	// DO#
#define D3 	(147 * 2)	// RE
#define D3H (156 * 2)	// RE#
#define E3 	(165 * 2)	// MI
#define F3 	(175 * 2)	// FA
#define F3H (185 * 2)	// FA#
#define G3 	(196 * 2)	// SOL
#define G3H (208 * 2)	// SOL#
#define A3 	(220 * 2)	// LA
#define A3H (233 * 2)	// LA#
#define B3 	(247 * 2)	// SI

#define C4  (131 * 4)	// DO
#define C4H (139 * 4)	// DO#
#define D4  (147 * 4)	// RE
#define D4H (156 * 4)	// RE#
#define E4  (165 * 4)	// MI
#define F4  (175 * 4)	// FA
#define F4H (185 * 4)	// FA#
#define G4  (196 * 4)	// SOL
#define G4H (208 * 4)	// SOL#
#define A4  (220 * 4)	// LA
#define A4H (233 * 4)	// LA#
#define B4  (247 * 4)	// SI

#define C5  (131 * 8) // DO
#define C5H (139 * 8) // DO#
#define D5  (147 * 8) // RE
#define D5H (156 * 8) // RE#
#define E5  (165 * 8) // MI
#define F5  (175 * 8) // FA
#define F5H (185 * 8) // FA#
#define G5  (196 * 8) // SOL
#define G5H (208 * 8) // SOL#
#define A5  (220 * 8) // LA
#define A5H (233 * 8) // LA#
#define B5  (247 * 8) // SI

#define C6  (131 * 16) // DO
#define C6H (139 * 16) // DO#
#define D6  (147 * 16) // RE
#define D6H (156 * 16) // RE#
#define E6  (165 * 16) // MI
#define F6  (175 * 16) // FA
#define F6H (185 * 16) // FA#
#define G6  (196 * 16) // SOL
#define G6H (208 * 16) // SOL#
#define A6  (220 * 16) // LA
#define A6H (233 * 16) // LA#
#define B6  (247 * 16) // SI

#define C7  (131 * 32) // DO
#define C7H (139 * 32) // DO#
#define D7  (147 * 32) // RE
#define D7H (156 * 32) // RE#
#define E7  (165 * 32) // MI
#define F7  (175 * 32) // FA
#define F7H (185 * 32) // FA#
#define G7  (196 * 32) // SOL
#define G7H (208 * 32) // SOL#
#define A7  (220 * 32) // LA
#define A7H (233 * 32) // LA#
#define B7  (247 * 32) // SI

// ------------------------------------------------------------------------------------------------------------------------
// tonesplayer
// note[size]     : tableau de fréquence (notation américaine A = LA, B= SI, C= DO,... A3 = LA 440Hz)
// duration[size) : tableau de durée (nombre de double croche) 1=double croche, 2= croche, 4=noire, 8=blanche, 16=ronde...)
// size           : nombre de note dans le morceau
// tempo		  : 60= 60 noire à la minute
// ------------------------------------------------------------------------------------------------------------------------
void tonesplayer(int *note, int *duration, int size, int tempo);

void toneInit(void);
void toneItMode(int note, int duration);
void tone_IT(void);
void toneStart(int note);
void toneStop(void);
void tone(int note, int duration);
void toneSetVolulme(int volume);
void welcomeTone(int volume);
void toneTest(void);

#endif
