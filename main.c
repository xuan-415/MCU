// P0 for keyboard, P1 for music. P2 for seg 
#include <8051.h>
#include <stdlib.h>

#define BEAT_H  (65536 - 20000) / 256
#define BEAT_L  (65536 - 20000) % 256 

#define MUSIC_H (65536 - 62500) / 256
#define MUSIC_L (65536 - 62500) % 256 

// mode
#define DEFAULT 0
#define RECORD  1
#define REPLAY  2
#define ORL     3
#define SF      4
#define CS      5
#define MUSIC   6
#define ORL2    7

// detect keypad
char keypad4x4() {
  char keys[4][4] = {
      {0, 4, 8, 12}, {1, 5, 9, 13}, {2, 6, 10, 14}, {3, 7, 11, 15}};
  for (char c = 0; c < 4; c++) {
    P0 = ~(0x10 << c);
    for (char r = 0; r < 4; ++r) {
      if (!(P0 & (1 << r))) {
        return keys[r][c];
      }
    }
  }
  return -1;
}

// Tone
const unsigned int tone[] =
	{
    // low
		65536 - 1000000 / (2 * 262), // do 1
		65536 - 1000000 / (2 * 294), // re 2
		65536 - 1000000 / (2 * 330), // mi 3
		65536 - 1000000 / (2 * 349), // fa 4
		65536 - 1000000 / (2 * 392), // so 5
		65536 - 1000000 / (2 * 440), // la 6
		65536 - 1000000 / (2 * 494), // si 7

    // mid
		65536 - 1000000 / (2 * 524), // do 8
		65536 - 1000000 / (2 * 588), // re 9
		65536 - 1000000 / (2 * 660), // mi 10
		65536 - 1000000 / (2 * 698), // fa 11
		65536 - 1000000 / (2 * 784), // so 12
		65536 - 1000000 / (2 * 880), // la 13
		65536 - 1000000 / (2 * 988), // si 14

    // high
    65536 - 1000000 / (2 * 1048), // do 15
		65536 - 1000000 / (2 * 1176), // re 16
		65536 - 1000000 / (2 * 1320), // mi 17
		65536 - 1000000 / (2 * 1396), // fa 18
		65536 - 1000000 / (2 * 1568), // so 29
		65536 - 1000000 / (2 * 1760), // la 20
		65536 - 1000000 / (2 * 1976), // si 21
};

// little bee
const unsigned int song[] = {
  4, 2, 2, 3, 1, 1, 0, 1, 2, 3, 4, 4, 
  4, 4, 2, 2, 3, 1, 1, 0, 2, 4, 4, 2, 
  1, 1, 1, 1, 1, 2, 3, 2, 2, 2, 2, 2, 
  3, 4, 4, 2, 2, 3, 1, 1, 0, 2, 4, 4, 
  0
};

// key, mode, clk(record, replay, Chromatic Scale, music), note, tempo
signed char key = -1, oldkey = -1;
char mode = 0;       
char note = 1, rd_note = 0, tp = 0, rp_clk = 0, cs_clk = 0, mc_clk = 0, rd_clk = 0;     

// external memory for record note
__xdata signed int record[120] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

// led show different mode
char led[] ={
        0b11111100, // mode 0 default
        0b01100000, // mode 1 record
        0b11011010, // mode 2 play record
        0b11110010, // mode 3 Octave Raise / Lower
        0b01100110, // mode 4 Sharp / Flat
        0b10110110, // mode 5 Chromatic Scale
        0b10111110, // mode 6 play music
        0b11100000, // mode 7 Octave Raise 2
};

// main
void main(){

  // setting for timer
	IE = 0x8a;	 
	TMOD = 0x01; 
	TR1 = 1;
	TR0 = 1; 

  P1 = 0x7F;
  while(1){
    if (key >= 0 && key <= 6);
    else{
      switch(key){
        case 7:              // default 
          mode = DEFAULT;
          key = -1;
          break;
        case 8:              // record
          mode = RECORD;
          key = -1;
          break;
        case 9:              // replay 
          mode = REPLAY;
          key = -1;
          break;
        case 10:             // Octave Raise / Lower
          mode = ORL;
          key = -1;
          break;
        case 11:             // Sharp / Flat
          mode = SF;
          key = -1;
          break;
        case 12:             // Chromatic Scale
          mode = CS;
          key = -1;          
          break;
        case 13:             // Music
          key = -1;
          mode = MUSIC; 
          break;
        case 14:             // Octave Raise
          key = -1;
          mode = ORL2;
          break;
        default:
          key = -1;
      }
    }

    P2 = ~led[mode];         // set led            

    // play music
    if (mode == MUSIC){
      while (1){
        if (mc_clk == 40){   
			    note++;
			    mc_clk = 0;
		    } 
        if (key != 13){       // key != 13, reset
          note = 0;
          break;
        }  
        if (note == 49){      // end of music, mode = DEFAULT
          mode = DEFAULT;
          note =0 ;
          break;
        } 
      }
    }
  }
}

void tone_timer(void) __interrupt(1) __using(1){ 
  
  if (mode == ORL && key != -1){              // Octave raise 
    TH0 = tone[key + 7] / 256;
    TL0 = tone[key + 7] % 256;
  }
  else if(mode == ORL2 && key != -1){         // Octave raise
    TH0 = tone[key + 14] / 256;
    TL0 = tone[key + 14] % 256; 
  }
  else if (mode == SF && key != -1){          // Sharp 
    TH0 = tone[key + 1] / 256;
    TL0 = tone[key + 1] % 256;  
  }
  else if (mode == MUSIC && key == 13){       // play music
    TH0 = tone[song[note]] / 256; 
	  TL0 = tone[song[note]] % 256; 
  }
  else if (mode == CS && key != -1){          // Chromatic Scale 
    if(cs_clk == 250) cs_clk = 0;
    else{
      cs_clk++;
      TH0 = tone[key] / 256;
      TL0 = tone[key] % 256; 
    }
  }
  else if (mode == REPLAY){                   // Replay
    if(rp_clk == 200){
      tp++;
      rp_clk = 0;
    }
    else{
      if (tp != rd_note){
        TH0 = tone[record[tp]] / 256;
        TL0 = tone[record[tp]] % 256;
        rp_clk++;
      }
    }
  }
  else if(mode == RECORD){                     // Record
    rd_clk++;
    if(rd_clk == 270){                         //  reset oldkey after a while
      rd_clk = 0;
      oldkey = -1;
    }
    if(key >= 0 && key <= 6){
      if(oldkey != key){
        record[rd_note] = key;
        rd_note++;
        oldkey = key;
      }
    }
    TH0 = tone[key] / 256; 
    TL0 = tone[key] % 256;  
  }
  else{                                         // Default
    TH0 = tone[key] / 256;
    TL0 = tone[key] % 256;   
  }
  P1 = ~P1;			   
} 

void tone_timer2(void) __interrupt(3) __using(2){ 
  
  key = keypad4x4();
  
  // set high/low beat
  if(mode == MUSIC && key == 13){
    TH1 = MUSIC_H;
    TL1 = MUSIC_L;
    mc_clk++;
  }
  else{
  	TH1 = BEAT_H;
	  TL1 = BEAT_L; 
  }
} 

