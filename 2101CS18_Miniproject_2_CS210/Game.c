
/* set a single pixel on the screen at x,y
 * x in [0,319], y in [0,239], and colour in [0,65535]
 */

#include <stdio.h>
#include<stdlib.h>
#include <math.h>
#include <string.h>
#define PUSHBUTTONS ((volatile long *) 0xFF200050)
#define RLEDs ((volatile long *) 0xFF200000)
#define HEX3_HEX0_BASE 0xFF200020

	
int blocks[320][240];int c2;

void write_pixel(int x, int y, short colour) {
  volatile short *vga_addr=(volatile short*)(0x08000000 + (y<<10) + (x<<1));
  *vga_addr=colour;
}

short get_color(int x, int y)
{
	volatile short *vga_addr=(volatile short*)(0x08000000 + (y<<10) + (x<<1));
	return *vga_addr;
}

/* use write_pixel to set entire screen to black (does not clear the character buffer) */


/* write a single character to the character buffer at x,y
 * x in [0,79], y in [0,59]
 */
void ball_transit(int x1, int y1, int x2, int y2, int r, short c)
{
	int xi, yi;
	
	for(xi = x1 - r; xi <= x1 + r; xi ++)
	{
		for(yi = y1 - r; yi <= y1 + r; yi ++)
		{
			if((xi - x1) * (xi - x1) + (yi - y1) * (yi - y1) <= r * r)
			{
			if((xi - x2) * (xi - x2) + (yi - y2) * (yi - y2) > r * r)
			{
				write_pixel(xi, yi, 0);
			}
			}
		}
	}
	
	for(xi = x2 - r; xi <= x2 + r; xi ++)
	{
		for(yi = y2 - r; yi <= y2 + r; yi ++)
		{
			if((xi - x2) * (xi - x2) + (yi - y2) * (yi - y2) <= r * r)
			{
				write_pixel(xi, yi, c);
			}
		}
	}
}



int slide_transit(int xs,int slidecol,int f)
{
	if((xs==5 && f==0) || (xs==275 && f==1))
	{
		return 0;
	}

    int buff = 5;
	if(f==1)
	{		
    if(314-xs-40<5)
    {
      buff=314-xs-40;
    }
    for(int i=xs;i<xs+buff;i++)
    {
      for(int j=230;j<=235;j++)
      {
        write_pixel(i, j, 0);
      }
    }
	for(int i=xs+buff;i<=xs+40+buff;i++)
	{
		for(int j=230;j<=235;j++)
		{
			write_pixel(i, j, slidecol);
		}
	}
	}

	if(f==0)
	{
    if(xs<5)
    {
      buff=xs;
    }
    for(int i=xs+40-buff+1;i<=xs+40;i++)
    {
      for(int j=230;j<=235;j++)
      {
        write_pixel(i, j, 0);
      }
    }
	for(int i=xs-buff;i<=xs+40-buff;i++)
	{
		for(int j=230;j<=235;j++)
		{
			write_pixel(i, j, slidecol);
		}
	}
	}
  return buff;
}

/* write a single character to the character buffer at x,y
 * x in [0,79], y in [0,59]
 */
void write_char(int x, int y, char c) {
  // VGA character buffer
  volatile char * character_buffer = (char *) (0x09000000 + (y<<7) + x);
  *character_buffer = c;
}

void clear_screen() {
  int x, y;
  for (x = 0; x < 320; x++) {
    for (y = 0; y < 240; y++) {
	  write_pixel(x,y,0);
		if(x < 80 && y < 60)
		{
			write_char(x, y, 0);
		}
	}
  }
}

void clear_game()
{
	int x, y;
  for (x = 5; x < 315; x++) {
    for (y = 10; y < 236; y++) {
	  write_pixel(x,y,0);
	}
  }

   for (x = 0; x < 80; x ++)
   {
      for(y = 0; y < 60; y ++)
      {
          write_char(x, y, 0);
      }
   }
}

int within(int x, int y)
{
	if(x >= 5 || x <= 314)
	{
		if(y >= 10 || y <= 234)
			return 1;
	}
	else return 0;
}

void erase_block(int cx, int cy)
{
	int b = blocks[cx][cy];
	write_pixel(cx, cy, 0);
	blocks[cx][cy] = 0;
	for(int i = 0; i < 10000; i ++);
	if(within(cx + 1, cy) && blocks[cx + 1][cy] == b)
	{
		erase_block(cx + 1, cy);
	}
	if(within(cx, cy + 1) && blocks[cx][cy + 1] == b)
	{
		erase_block(cx, cy + 1);
	}
	if(within(cx - 1, cy) && blocks[cx - 1][cy] == b)
	{
		erase_block(cx - 1, cy);
	}
	if(within(cx, cy - 1) && blocks[cx][cy - 1] == b)
	{
		erase_block(cx, cy - 1);
	}
}


int collides(double x, double y, int r, int c)
{
	for(int i = x - r; i <= x + r; i ++)
	{
		for (int j = y - r; j <= y + r; j ++)
		{
			if((i - x) * (i - x) + (j - y) * (j - y) > r * r) continue;
			if(get_color(i, j) != c && get_color(i, j) != 0) return 1;
		}
	}
	return 0;
}

double * collide_type(double x, double y, int r, int c)
{
	double dmin = r * r * 8;
	double * ans = (double *) malloc(2 * sizeof(double));
	for(int i = x - r; i <= x + r; i ++)
	{
		for (int j = y - r; j <= y + r; j ++)
		{
			if((i - x) * (i - x) + (j - y) * (j - y) > r * r) continue;
			if(get_color(i, j) == c || get_color(i, j) == 0) continue;
			if(dmin > (i - x) * (i - x) + (j - y) * (j - y))
			{
				dmin = (i - x) * (i - x) + (j - y) * (j - y);
				ans[0] = i;
				ans[1] = j;
			}
		}
	}
	return ans;
}

int score = 0;

void display_score()
{
	volatile int *hex3_hex0_ptr = (int *)HEX3_HEX0_BASE; // Pointer to HEX3_HEX0
	
	int digit_values[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
	int ones = score % 10; // Extract the ones digit
    int tens = (score / 10) % 10; // Extract the tens digit
    int hundreds = (score / 100) % 10; // Extract the hundreds digit
    int thousands = (score / 1000) % 10; // Extract the thousands digit

    // Set the values of the individual digits on the 7-segment displays
    *hex3_hex0_ptr = (digit_values[thousands] << 24) | (digit_values[hundreds] << 16) | (digit_values[tens] << 8) | digit_values[ones];
}

void display_num(int x)
{
	volatile int *hex3_hex0_ptr = (int *)HEX3_HEX0_BASE; // Pointer to HEX3_HEX0
	
	int digit_values[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
	int ones = x % 10; // Extract the ones digit
    int tens = (x / 10) % 10; // Extract the tens digit
    int hundreds = (x / 100) % 10; // Extract the hundreds digit
    int thousands = (x / 1000) % 10; // Extract the thousands digit

    // Set the values of the individual digits on the 7-segment displays
    *hex3_hex0_ptr = (digit_values[thousands] << 24) | (digit_values[hundreds] << 16) | (digit_values[tens] << 8) | digit_values[ones];
}

arr_blocks[500][2];

void actual_print(ai,bi,id)
{
	for(int i = ai; i <= ai+20; i++)
	{
		for(int j = bi; j <= bi+10; j++)
		{
			blocks[i][j] = id;
			write_pixel(i, j, c2);
		}
	}
	arr_blocks[id][0]=ai;
	arr_blocks[id][1]=bi;
}

void print_blocks()
{
	actual_print(50,30,1);
	actual_print(50,42,2);
	actual_print(50,54,3);
	actual_print(50,78,5);
	actual_print(50,90,4);
	actual_print(72,18,8);
	actual_print(72,30,6);	
	actual_print(72,42,7);	
	actual_print(72,66,9);
	actual_print(72,78,10);
  
  
	actual_print(240,78,11);	
	actual_print(262,102,12);	
	actual_print(284,78,13);	
	actual_print(240,90,14);	
	actual_print(262,114,15);	
	actual_print(284,90,16);
	actual_print(262,90,17);	

  // 17 blocks complete


	actual_print(150,170,18);	
	actual_print(190,210,19);	
	actual_print(230,18,20);	
	actual_print(270,18,21);	
	actual_print(170,30,22);	
	actual_print(210,30,23);	
	actual_print(250,30,24);	
	actual_print(290,30,25);	

  // 25 blocks

	actual_print(130,70,26);	
	actual_print(152,70,27);	
	actual_print(174,70,28);
	actual_print(130,82,29);
	actual_print(174,82,30);
	actual_print(130,94,31);
	actual_print(152,94,32);
	actual_print(174,94,33);

  // 33

	actual_print(20,150,34);
	actual_print(20,170,35);
	actual_print(20,190,36);
	actual_print(20,210,37);

  //37

	actual_print(190,150,38);
	actual_print(190,162,39);
	actual_print(190,174,40);
	actual_print(190,186,41);
	actual_print(190,198,42);
	actual_print(212,174,43);
	actual_print(234,150,44);
	actual_print(234,162,45);
	actual_print(234,174,46);
	actual_print(234,186,47);
	actual_print(234,198,48);

  // 48

	actual_print(260,150,49);
	actual_print(282,150,50);
	actual_print(271,162,51);
	actual_print(271,174,52);
	actual_print(271,186,53);
	actual_print(260,198,54);
	actual_print(282,198,55);
}

int coun=56;

void print_small_blocks(ai,bi)
{
   for(int i = ai+5; i <= ai+15; i++)
	{
		for(int j = bi+3; j <= bi+7; j++)
		{
			blocks[i][j] = coun;
			write_pixel(i, j, c2);
		}
	}
	coun++;
}

void play_game () 
{
   clear_screen();
	
	for (int i = 0; i < 320; i++)
	{
		for(int j = 0; j < 240; j++)
		{
			if(i < 5 || i > 314 || j < 10 || j > 235)
			{
				write_pixel(i, j, 0xffff + 1000000);
			}
		}
	}
   long PBval;
	
	short cb = 0xffff;
	
	int g=1;
	short c1 = cb + 300000; c2 = cb + 600000;
	double x = 120, y = 190;
	int slidecol = c1 +6000;
	double xs =140;
	int r = 3;
	
	double x1 = 75, y1 = 130;
	
	for(int i = 0; i < 320; i ++)
	{
		for(int j = 0; j < 240; j ++)
		{
			blocks[i][j] = 0;
		}
	}
	
  print_blocks(c2);

	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j <= i; j ++)
		{
			write_pixel(x1 + 50 - j, y1 + i, c1);
		}
	}

  for(int i = 140; i <= 180; i++) // slide
	{
		for(int j=230;j<=235;j++)
		{
           write_pixel(i, j, slidecol);
		}
	}
	
	double delay = 10000;
	
	double dirx = 2, diry = 3;
	
	while(1)
	{
		display_score();
    	PBval = *PUSHBUTTONS;
		double nx = x + dirx;
		double ny = y + diry;

    if ((PBval & 0x01) && (PBval & 0x02)) // Check if button is pressed
    {
      // xs+=slide_transit(xs,slidecol,1);
    }
    else if(!((PBval & 0x01) || (PBval & 0x02)))
    {

    }
    else if (PBval & 0x01)
    {
      xs+=slide_transit(xs,slidecol,1);
      
    }
    else if(PBval & 0x02)
    {
    xs-=slide_transit(xs,slidecol,0);
    
    }
    else
    {

    }
		
    if(nx - r < 0 || nx + r > 320)
    {
      if(nx - r < 0) nx = r;
      else nx = 319 - r;
      
      dirx = -dirx;
    }
    if(ny - r < 0 )
    {
      if(ny - r < 0) ny = r;
      else ny = 239 - r;
      
      diry = -diry;
    }
    if(ny + r > 232)
    {
      clear_game();
      break;
    }
      
    if(collides(nx, ny, r, cb))
    {	
      int onx = nx, ony = ny;
      while(collides(nx, ny, r, cb))
      {
        onx = nx;
        ony = ny;
        nx += (dirx == 0 ? 0 : (dirx > 0 ? -1 : 1));
        ny += (diry == 0 ? 0 : (diry > 0 ? -1 : 1));
      }
      
      double * coll = collide_type(onx, ony, r, cb);
      double cx = coll[0];
      double cy = coll[1];
      
      if(cx == onx)
      {
        diry = -diry;
      }
      else if(cy == ony)
      {
        dirx = -dirx;
      }
      else
      {
        if(dirx > 0 && cx > nx || dirx < 0 && cx < nx)
        dirx = -dirx;
        if(diry > 0 && cy > ny || diry < 0 && cy < ny)
        diry = -diry;
      }
      
      if(blocks[(int)cx][(int)cy])
      {

		if(blocks[(int)cx][(int)cy]<=55)
		{
			score += 100;
			print_small_blocks(arr_blocks[blocks[(int)cx][(int)cy]][0],arr_blocks[blocks[(int)cx][(int)cy]][1]);
		}
		else
		{
			score += 50;
		}
		erase_block(cx, cy);
      }
    }
    
    ball_transit((int)(x), (int)(y), (int)nx, (int)ny, r, cb);
    
    x = nx;
    y = ny;
  
    for(int i = 0; i < delay; i++);
	}
	
	char *mess1 = "Game Over!";
	
	for (int i = 0; i < 10; i++)
	{
		write_char (35 + i, 30, mess1[i]);
	}
	
	char *mess2 = "Press any key to try again.";
		
	for (int i = 0; i < 27; i++)
	{
		write_char (25 + i, 45, mess2[i]);
	}
	
	while(1)
	{
		if(*PUSHBUTTONS > 0)
		{
			while(1)
			{
				if(*PUSHBUTTONS == 0)
				{
					return;
				}
			}
		}
	}
}

int main()
{
	clear_screen();
	
	int best_score = 0;
	while(1)
	{
		char *mess = "Press any key to play.";
		
		for (int i = 0; i < 22; i++)
		{
			write_char (25 + i, 40, mess[i]);
		}
		display_num(best_score);
		if(*PUSHBUTTONS > 0)
		{
			while(1)
			{
				if(*PUSHBUTTONS == 0)
				{
					score = 0;
					play_game();
					if(score > best_score)
						best_score = score;
					clear_game();
					break;
				}
			}
		}
	}
}
