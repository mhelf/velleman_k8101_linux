#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ncurses.h>
#include "usb_interface.h"
#include "game.h"




static playerobject *player;
static objectlist *gameobs;
static sharedobject *shared;


static int fd;

int main()
{

	

	start();
	clear_all();
	backlight_on(120);
	contrast(0x10);

	//Setup shared memory
	if((fd = shm_open("shared", O_CREAT | O_RDWR, 0666))==-1)
	{
		printf("Failed to open shared memory\n");
		exit(1);
	}
		
	ftruncate(fd, sizeof(sharedobject));	
	if((shared = mmap(NULL, sizeof(sharedobject), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED)
	{
		printf("Failed to mmap\n");
		exit(1);
	}

	pid_t loop;
	shared->button_pressed = false;
	shared->state = READY;
	init();
	
	//Fork loop process
	if ((loop = fork()) > 0) 
	{
		//Game-loop process
		while(shared->state != GAMEOVER)
		{

			while(shared->state == READY)
			{

				
				
				
				clear_all();
				//Testing of other drawing functions
				//draw_pixel(10,20);
				//draw_pixel(12,10);
				//draw_pixel(13,10);
				//draw_pixel(14,10);
				//draw_line(14,10,126,50);

				//Draw Menu
				draw_text("TUNNEL FLIGHT\0",30,10,128,TEXT_SIZE_BIG);
				draw_text("Arrow Up -> controlls\0",28,25,128,TEXT_SIZE_SMALL);
				draw_text("SPACE to start\0",42,40,128,TEXT_SIZE_SMALL);
				draw_text("E to end\0",52,50,128,TEXT_SIZE_SMALL);
				sleep(1);
			}
			//Game Loop
			while(shared->state == RUNNING)
			{
				update();
				render();		
				usleep(200*1000);
			}
		}

		
		clear_all();
		draw_text("GAME OVER :D\0",40,30,128,TEXT_SIZE_BIG);

		clean();
		

		return EXIT_SUCCESS;
	}
	else if(loop < 0)
	{
		printf("Failed to fork game loop\n");
		exit(1);
	}
	else
	{

		//Input process
		shared->button_pressed = false; 
		int ch;
    		initscr();
		cbreak();
    		noecho();
    		nodelay(stdscr, TRUE);
		keypad(stdscr, TRUE);

		while(1)
		{	
			if(shared->state == READY)
			{
				while(1)
				{
					ch = getch();
					if(ch == ' ')
					{
						shared->state = RUNNING;
						break;
					}
					else if(ch == 'e')
					{
						shared->state = GAMEOVER;
						break;
					}
				}
			}
	
			if((ch = getch())==ERR)
			{
				shared->button_pressed = false;
			}
			else
			{	
				if(ch == KEY_UP)
				{
					shared->button_pressed = true;
				}
			}
			
			usleep(20*1000);
		
			if(shared->state == GAMEOVER)
				break;
		}

		refresh();

		endwin();

		if (close(fd) == -1) 
		{
			printf("Close failed\n");
			exit(1);
		}

		
		

	}
	clear_all();
	draw_text("GAME OVER :D\0",40,30,128,TEXT_SIZE_BIG);
	sound(2);
	end();
	return 0;
}



void update()
{	
	update_player();
	update_objects();

	if(collision_detection())
	{
		shared->state = GAMEOVER;
		sound(3);
	}
	
}

void update_player(){

	if(shared->button_pressed)
	{


		if(player->gravity > -MAX_GRAVITY)
		{
			player->gravity -= 2;
		}
		else
		{
			player->gravity = -MAX_GRAVITY;
		}
		

	}
	else
	{	

		if(player->gravity < MAX_GRAVITY)
		{
			player->gravity += 1;
		}
		else
		{
			player->gravity = MAX_GRAVITY;
		}
		
	}
	
	player->y += player->gravity;
	if(player->y <= 1)
	{
		player->y = 1;
	}
	else if(player->y+player->height >= LEVEL_HEIGHT)
	{
		player->y = LEVEL_HEIGHT-player->height-1;
	}

		

	

	

}

void update_objects()
{
	gameobject *tmp;
	int i;
	for(i = 0;i<=gameobs->count-1;i++)
	{
		
		if((tmp = gameobs->list[i]) != NULL)
		{


			
			if(tmp->x <= 1)
			{
				tmp->x = 1;
				tmp->display_width -= SCROLL_SPEED;

				if(tmp->display_width <= 1)
				{
					bool is_top = tmp->is_top;
					free(gameobs->list[i]);
					create_new_ob(is_top,i,LEVEL_WIDTH-1);
				}
			}
			else
			{
				tmp->x -= SCROLL_SPEED;
			}

		}
	}

}

void create_new_ob(bool is_top,int index,unsigned char x){

	unsigned char y;
	unsigned char width;
	unsigned char height;

	height= rand() %(MAX_HEIGHT-MIN_HEIGHT+1)+MIN_HEIGHT;
	width= rand() %(MAX_WIDTH-MIN_WIDTH+1)+MIN_WIDTH;
	
	
	if(is_top)
	{
		y = 1;
	}
	else
	{
		y= LEVEL_HEIGHT - height;
	}

	gameobs->list[index] = malloc(sizeof(gameobject));
	gameobs->list[index]->display_width = width;
	gameobs->list[index]->display_height = height;
	gameobs->list[index]->width = width;
	gameobs->list[index]->height = height;
	gameobs->list[index]->x = x;
	gameobs->list[index]->y = y;
	gameobs->list[index]->is_top = is_top;
}



bool collision_detection()
{
	
	gameobject *tmp;
	int i;
	for(i = 0;i<gameobs->count;i++)
	{
		if((tmp = gameobs->list[i]) != NULL)
		{
			if(tmp->is_top)
			{
				if(player->x+player->width > tmp->x && player->x < tmp->x+tmp->width)
				{
					if(player->y < tmp->y+tmp->height)
					{	
						return true;
					}
				}

			}
			else
			{
				if(player->x+player->width > tmp->x && player->x < tmp->x+tmp->width)
				{
				
					

					if(player->y+player->height > tmp->y)
					{	
						return true;
					}
				}
			}
		}
			
				


	}
	return false;

}

void render()
{	
	clear_all();
	gameobject *tmp;
	int i;
	for(i = 0;i<gameobs->count;i++)
	{
		if((tmp = gameobs->list[i]) != NULL)
		{
			draw_rectangle(tmp->x,tmp->y,tmp->display_width,tmp->display_height);
		}
	}
	draw_rectangle(player->x,player->y,player->width,player->height);
}

void init()
{
	player = malloc(sizeof(playerobject));
	player->x = 15;
	player->y = 1;
	player->width = 15;
	player->height = 5;
	player->gravity = -2;

	gameobs = malloc(sizeof(objectlist));
	gameobs->list = malloc(sizeof(gameobject*)*MAX_OBJECTS_ON_SCREEN);
	gameobs->count = MAX_OBJECTS_ON_SCREEN;

	create_new_ob(true,1,LEVEL_WIDTH); 
	create_new_ob(false,0,LEVEL_WIDTH-OBJECT_SPACING*2); 
}

void clean()
{
	free(player);
	if(gameobs->list[0] != NULL) free(gameobs->list[0]);
	if(gameobs->list[1] != NULL) free(gameobs->list[1]);
	free(gameobs->list);
}

