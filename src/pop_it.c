/* By Tasos Sahanidis <code@tasossah.com>, 2024                                        */
/* Based on the cc65 2600 sample by Florent Flament (contact@florentflament.com), 2017 */
#include <atari2600.h>
#include <stdlib.h>

// PAL Timings
// Roughly computed based on Stella Programmer's guide (Steve Wright)
// scanlines count per section.
#define VBLANK_TIM64 57 // 48 lines * 76 cycles/line / 64 cycles/tick
#define KERNAL_T1024 17 // 228 lines * 76 cycles/line / 1024 cycles/tick
#define OVERSCAN_TIM64 42 // 36 lines * 76 cycles/line / 64 cycles/tick

#define VPOS_MIDDLE 60
#define MISSILEHEIGHT 8
#define BOTTOM_SCANLINE 220
#define FRIENDLY_MISSILE_POS_ADD 33
#define ENEMY_MISSILE_POS_ADD 57

#define PFSIZE (sizeof(playfield) / sizeof(*playfield))
// Left to right - mirrored
static const unsigned char playfield[][3] = {
	     // XXXX
	{ 0b11111111, 0b11111111, 0b11111111 },
	{ 0b11111111, 0b11111111, 0b11111111 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b00101111, 0b00000000, 0b00000000 },
	{ 0b00011111, 0b00000000, 0b00000000 },
	{ 0b11111111, 0b11111111, 0b11111111 },
	{ 0b11111111, 0b11111111, 0b11111111 },
	{ 0b11111111, 0b11111111, 0b11111111 },
	{ 0b11111111, 0b11111111, 0b11111111 },

};

#define PLAYERHEIGHT 7

static const unsigned char playerdata[][PLAYERHEIGHT] = {
#define TARGET_START 0
	{
		0b00001000,
		0b00010100,
		0b00100010,
		0b01001001,
		0b00100010,
		0b00010100,
		0b00001000,
	},
#define SMILE_START PLAYERHEIGHT
//static const unsigned char smile[][7] = {
	{
		0b00000000,
		0b00100010,
		0b00000000,
		0b00001000,
		0b00000000,
		0b00111110,
		0b01100011,
	},
	{
		0b00000000,
		0b00100010,
		0b00000000,
		0b00001000,
		0b00000000,
		0b01111111,
		0b00000000,
	},
	{
		0b00000000,
		0b00100010,
		0b00000000,
		0b01001001,
		0b01100011,
		0b00110110,
		0b00011100,
	},
};

// Avoid the stack as it makes code super slow
// Shove everything into the zeropage for now
#pragma bss-name (push, "ZEROPAGE", "zeropage")

static unsigned char color;
static unsigned char vpos;
static unsigned char missilepos;
static unsigned char sound_timer;
static unsigned char scanline;

// Use indexing instead of pointers as those are 16 bit and are slower
static unsigned char player;
static unsigned char current_playfield;
static unsigned char current_player_line;

static unsigned char swcha;
static unsigned char pfp0col;
static unsigned char pm1col;
static unsigned char pm0col;
static unsigned char score;
static unsigned char letgoofbutton; // this is the opposite of what the name implies
static unsigned char friendlymissilepos;
static unsigned char counter;
static unsigned char counter2;

static unsigned char collision_enable;
static unsigned char diff;
static unsigned char player_plus_diff;
static unsigned char difficulty_frame_counter;
static unsigned char difficulty_frame_counter2;

#pragma bss-name (pop)

// Awful awful awful, no time...
static inline void doLose(void) {
	unsigned char k;
	TIA.audc1 = 12;
	TIA.audv1 = 15;
	TIA.audf1 = 0b01110;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	}
	TIA.audf1 = 0b11111;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	
	}
	TIA.audv1 = 0;
}

// Awful awful awful awful awful, no time...
static inline void doWin(void) {
	unsigned char k;
	TIA.audc1 = 12;
	TIA.audv1 = 15;
	TIA.audf1 = 0b11111;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	}
	TIA.audv1 = 0;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	}
	TIA.audv1 = 15;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	}
	TIA.audf1 = 0b01110;
	for(k = 0; k != 255; k++) {
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
		TIA.wsync = 0;
	}
	TIA.audv1 = 0;
}

#define MISSILE_POS_ADD(x, y) \
			x += y; \
			if(x > BOTTOM_SCANLINE) \
				x = x - BOTTOM_SCANLINE



int main(void) {
reset:
	friendlymissilepos = 20;
	letgoofbutton = 1;
	score = 0;
	pm0col = 0;
	pm1col = 0;
	pfp0col = 0;
	//swcha = 0xFF;
	player = TARGET_START;
	sound_timer = 0;
	vpos = VPOS_MIDDLE;
	TIA.colup0 = color = 0x55;
	TIA.colup1 = 0xbb;
	missilepos = 12;
	counter = 0;
	collision_enable = 0;
	current_player_line = 0;
	TIA.resp0 = 0;
	TIA.resm0 = 0;
	TIA.resm1 = 0;

	RIOT.swacnt = 0;
	TIA.audf0 = 0;
	TIA.nusiz0 = 0b00010101;
	TIA.nusiz1 = 0b00110000;
	TIA.ctrlpf = 1;
	TIA.colubk = 0x00;
	TIA.colupf = 0x88;
	TIA.audc0 = 3;

	while(1) {
		// Check if we need to reset
		if(!(RIOT.swchb & 1))
			goto reset;

		if(RIOT.swchb & 0b10000000) {
			difficulty_frame_counter = 35;
			difficulty_frame_counter2 = 45;
		} else {
			difficulty_frame_counter = 100;
			difficulty_frame_counter2 = 120;
		}

		if(pm0col) {
			doLose();
			goto reset;
		}

		// Vertical Sync signal
		TIA.vsync = 0x02;
		TIA.wsync = 0x00;
		TIA.wsync = 0x00;
		TIA.wsync = 0x00;
		TIA.vsync = 0x00;

		// Vertical Blank timer setting
		RIOT.tim64t = VBLANK_TIM64;

		// Doing frame computation during blank
		if(++counter > difficulty_frame_counter) {
			counter = 0;
			MISSILE_POS_ADD(friendlymissilepos, FRIENDLY_MISSILE_POS_ADD);
		}
		if(++counter2 > difficulty_frame_counter2) {
			counter2 = 0;
			MISSILE_POS_ADD(missilepos, ENEMY_MISSILE_POS_ADD);
		}

		swcha = RIOT.swcha;

		if(sound_timer) {
			TIA.audv0 = 15;
			TIA.audf0 = 0x1f - sound_timer;
			// Skip black, white, and cyan
			do {
				color++;
			} while((color & 0xE) == 0 || (color & 0xE) == 0xE || (color * 0xE) == 0xA);
			TIA.colup0 = color;
			sound_timer--;
		} else {
			TIA.audv0 = 0;
			player = TARGET_START;
		}

		TIA.hmm1 = 0b10110000;
		TIA.hmm0 = 0b00010000;

		if(TIA.inpt5 & 0x80) {
			letgoofbutton = 1;
		} else {
			if(pm1col && letgoofbutton) {
				if(!sound_timer)
					sound_timer = 0x1f;

				score++;
				MISSILE_POS_ADD(friendlymissilepos, FRIENDLY_MISSILE_POS_ADD);
				TIA.resm1 = 0;
				if(score < 4)
					player = SMILE_START;
				else if(score < 7)
					player = SMILE_START + PLAYERHEIGHT;
				else if(score < 10)
					player = SMILE_START + PLAYERHEIGHT*2;
				else {
					doWin();
					goto reset;
				}
				letgoofbutton = 0;
			}
		}
		
		// Down
		if(!(swcha & 2)) {
			collision_enable = 1; // Bad code... Same thing is copied for every direction
			if(!pfp0col || vpos < VPOS_MIDDLE)
				vpos += 2;
		} else if(!(swcha & 1)) { // Up
			collision_enable = 1;
			if(!pfp0col || vpos > VPOS_MIDDLE)
				vpos -= 2;
		}

		if(!(swcha & 8)) {
			collision_enable = 1;
			TIA.hmp0 = 0b11100000;
		} else if (!(swcha & 4)) {
			collision_enable = 1;
			TIA.hmp0 = 0b00100000;
		} else {
			TIA.hmp0 = 0;
		}

		// Wait for end of Vertical Blank
		while (RIOT.timint == 0);
		TIA.wsync = 0x00;
		 // Turn on the beam
		TIA.vblank = 0x00;

		// Prepare for rendering
		current_playfield = 0;
		current_player_line = 0;

		// Display frame
		RIOT.t1024t = KERNAL_T1024;

		scanline = 0;
		TIA.wsync = 0;
		TIA.hmove = 0;

		do {
			TIA.wsync = 0;
			if(scanline == 0 || scanline == BOTTOM_SCANLINE - 10) {
				TIA.pf0 = 0xFF;
				TIA.pf1 = 0xFF;
				TIA.pf2 = 0xFF;
			} else if (scanline == 10) {
				TIA.pf1 = 0;
				TIA.pf2 = 0;
			}
			if(scanline & 1) {
//				if(scanline & 2) {
					if(scanline > vpos && current_player_line < PLAYERHEIGHT) {
						diff = current_player_line + player;
						TIA.grp0 = ((unsigned char*)playerdata)[diff];
						current_player_line++;
					} else {
						TIA.grp0 = 0;
					}
//				}
			} else {
			
				if(scanline & 2)
				{
					diff = scanline - missilepos;
					if(diff < MISSILEHEIGHT)
						TIA.enam0 = 2;
					else
						TIA.enam0 = 0;
				} else {
					diff = scanline - friendlymissilepos;
					if(diff < MISSILEHEIGHT)
						TIA.enam1 = 2;
					else
						TIA.enam1 = 0;
				}
			}
			scanline++;
		} while(RIOT.timint == 0);
		TIA.wsync = 0;
		// Turn off the beam
		TIA.vblank = 0x02;

		// Overscan
		RIOT.tim64t = OVERSCAN_TIM64;
		if(collision_enable) {
			pfp0col = TIA.cxp0fb & 0x80;
			pm1col = TIA.cxm1p & 0x80;
			pm0col = TIA.cxm0p & 0x40;
		} else {
			pfp0col = pm1col = pm0col = 0;
		}
		TIA.cxclr = 0;

		while (RIOT.timint == 0);
	}
	return 0;
}
