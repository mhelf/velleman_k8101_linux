
#ifndef GAME_H
#define GAME_H
#define GAMEOVER 2
#define RUNNING 1
#define READY 0

#define LEVEL_WIDTH 127
#define LEVEL_HEIGHT 63
#define MAX_OBJECTS_ON_SCREEN 2

#define MAX_GRAVITY 4
#define SCROLL_SPEED 2


#define MIN_WIDTH 6
#define MAX_WIDTH 12
#define MIN_HEIGHT 20
#define MAX_HEIGHT 40

#define OBJECT_SPACING \
LEVEL_HEIGHT/MAX_OBJECTS_ON_SCREEN

/*
 * Struct holds data for the playerobject position, scale and gravity.
 */
typedef struct {
	char x;
	char y;
	char width;
	char height;
	char gravity;
}playerobject;


/*
 * Struct holds data for a gameobject position, scale, displayed scale and if it is on top or bottom of the level.
 */
typedef struct {
	char x;
	char y;
	char width;
	char height;
	char display_width;
	char display_height;
	bool is_top;
}gameobject;

/*
 * Struct holds a list with all the gameobjects 
 */
typedef struct {
	gameobject **list;
	char count;
}objectlist;

/*
 * Struct hold data for sharedmemory which is used for communication between input and main process of the game. 
 */
typedef struct {
	int button_pressed;
	int state;
}sharedobject;

/*
 * Initializes the objects.
 */
void init();

/*
 * Clean up the objects and free allocated memory.
 */
void clean();

/*
 * Renders the level to the screen.
 */
void render();

/*
 * Updates the state of all game objects and player and checks for collision and Game over.
 */
void update();

/*
 * Updates the player called in update.
 */
void update_player();

/*
 * Updates the gameobjects called in update.
 */
void update_objects();

/*
 * Checks wether the player is colliding with an gameobject or not. If yes the game is over.
 */
bool collision_detection();

/*
 * Creates a single game object and adds it to the list.
 */
void create_new_ob(bool is_top,int index, unsigned char x);

#endif /* GAME_H */
