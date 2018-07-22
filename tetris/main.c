/* The initial goal of this program was to create a "tetris" engine with no rotation, a friend got this as a coding take home
 * exercise and I thought it sounded fun.
 * Initially, I was printing to stdout to display the game and this got cumbersome fast.
 * I was familiar with the idea of the ncurses library and took this as an opportunity to learn it.
 * This represents by far the most coding I've done in 8 years or so (perl), or 10 years since the last
 * time I did anything in C (back in school).
 * The only code copied from a website verbatim is the rng initializer though this is probably idiomatic.
 * Writing this also told me that rand() % 7 will always return 0! https://stackoverflow.com/questions/7866754/why-does-rand-7-always-return-0
 * This code has no license because I frankly don't know anything about them.
 * I'm planning on coming back and fixing rotation and line removal.
 * -matt holland 7/20/18
 */

#include <stdlib.h>  // rand(), srand()
#include <time.h>    // time()
#include <unistd.h>  // usleep()
#include <ncurses.h> // ncurses

#define WIDTH 10
#define HEIGHT 20 //visible playing rows + two invisible rows at the top for rotating an initially placed line
#define DELAY 3000


/* enums */
enum state{empty,temp,full,wall};          //empty = no block present, temp = shape filling space but not commited, block is present, wall = invisible wall/ceiling
typedef enum {line,square,T,J,L,S,Z} shapeType; //enum for the different shapes ("tetronimo" is too cumbersome to type)
typedef enum {left,right,down} direction;
typedef enum {rot0,rot90,rot180,rot270} rotation;

/* structs */

typedef struct {
        shapeType type;  //which shape it is
        //each shape has an "anchor" which is the upper left position in its 4x4 frame
        unsigned char row; //row coordinate of the anchor
        unsigned char col; //col coordinate of the anchor
        rotation rot; 
} shape; //holds state of current active shape


/* global vars */

//global so intialized for me to 0
//we have two "invisible" rows at the top to allow for rotating a line from its initial position
//we have two columns on the left and one on the right that will always be full
//this lets us simplify collision detection due to the varying dead space in the 4x4 grid depending on shape
char field[WIDTH+3][HEIGHT+2];

shape curShape = {Z,0,3,rot0};	//initial anchor position and rotation
								//we'll randomize the initial shape in placeShape

/* function declarations */
int moveShape(direction dir);
void clearShape(void);
void setShape(int);
void placeShape(void);
void updateDisplay(WINDOW *win);
int lineCheck();


//following array contains all rotations of each shape in a 4x4 matrix

//first subscript gives the tetromino
//second subscript gives the rotation
//third subscript gives the row
//fourth subscript gives the col
const int shapes[7][4][4][4] = {
//line
{ { {0,0,0,0},
	{0,0,0,0},
	{1,1,1,1},
	{0,0,0,0} },
	
  { {0,0,1,0},
	{0,0,1,0},
	{0,0,1,0},
	{0,0,1,0} },

  { {0,0,0,0},
	{0,0,0,0},
	{1,1,1,1},
	{0,0,0,0} },

  { {0,0,1,0},
	{0,0,1,0},
	{0,0,1,0},
	{0,0,1,0} } },
//square
{ { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0} },
	
  { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0} },

  { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0} },

  { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0} } },
//T
{ { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,1},
	{0,0,1,0} },
	
  { {0,0,0,0},
	{0,0,1,0},
	{0,1,1,0},
	{0,0,1,0} },

  { {0,0,0,0},
	{0,0,1,0},
	{0,1,1,1},
	{0,0,0,0} },

  { {0,0,0,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,1,0} } },
//J
{ { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,1},
	{0,0,0,1} },
	
  { {0,0,0,0},
	{0,0,1,0},
	{0,0,1,0},
	{0,1,1,0} },

  { {0,0,0,0},
	{0,1,0,0},
	{0,1,1,1},
	{0,0,0,0} },

  { {0,0,0,0},
	{0,0,1,1},
	{0,0,1,0},
	{0,0,1,0} } },
//L
{ { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,1},
	{0,1,0,0} },
	
  { {0,0,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,1,0} },

  { {0,0,0,0},
	{0,0,0,1},
	{0,1,1,1},
	{0,0,0,0} },

  { {0,0,0,0},
	{0,0,1,0},
	{0,0,1,0},
	{0,0,1,1} } },
//S
{ { {0,0,0,0},
	{0,0,0,0},
	{0,0,1,1},
	{0,1,1,0} },
	
  { {0,0,0,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,0,1} },

  { {0,0,0,0},
	{0,0,0,0},
	{0,0,1,1},
	{0,1,1,0} },

  { {0,0,0,0},
	{0,0,1,0},
	{0,0,1,1},
	{0,0,0,1} } },
//Z
{ { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,0,1,1} },
	
  { {0,0,0,0},
	{0,0,0,1},
	{0,0,1,1},
	{0,0,1,0} },

  { {0,0,0,0},
	{0,0,0,0},
	{0,1,1,0},
	{0,0,1,1} },

  { {0,0,0,0},
	{0,0,0,1},
	{0,0,1,1},
	{0,0,1,0} } }

};




//moves shape in the field, does collision detection, updates field if no collision, returns 1 if new location is fixed
//returns 0 if block is still free, 1 if block has been placed, 2 if there was a collision
int moveShape(direction dir) {
	//mvprintw(0,0,"enter moveShape                    ");
	//getch();
	int collision = 0;
	switch (dir) {
		case (left) :
			//mvprintw(0,0,"enter case left                    ");
			//getch();
			//wall check 
			//flat line
			if (curShape.col == 2)
				if (curShape.type == line && (curShape.rot == rot0 || curShape.rot == rot180))
					collision = 1;
			//rotated line
			if (curShape.col == 0) 
				if (curShape.type == line && (curShape.rot == rot90 || curShape.rot == rot270))
					collision = 1;
			//all other shapes
			if (curShape.col == 1 && curShape.type != line)
					collision = 1;
		
			if(collision)
				return 2;
			
			//block check
			for (int j = 0; j < 4; j++) {
				for (int i = 0 ; i < 4; i++) {
					//check for collision on left column moved once left
					//element by element 
					if (field[curShape.row+j][curShape.col+i] == empty) 
						continue;
					else if (field[curShape.row+j][curShape.col+i] == temp) {
						//check if piece left of it is full
						if(field[curShape.row+j][curShape.col+i-1] == full) {
							collision = 1;
						}
					} else if (field[curShape.row+j][curShape.col+i] == full) {
						//check if piece right of it is full
						if(field[curShape.row+j][curShape.col+i+1] == temp) {
							collision = 1;
						}
					}
				}
			}
			if (collision) {
				return 2;
			}
			if (!collision) {
				//mvprintw(0,0,"no collision was detected              ");
				//getch();
				clearShape();
				//mvprintw(0,0,"exited clearShape             ");
				//getch();
				//mvprintw(0,0,"updating curShape.col, now it's %d", curShape.col);
				//getch();
				curShape.col = curShape.col - 1;
				//mvprintw(0,0,"after it's %d                ", curShape.col);
				//getch();
				setShape(temp);
			}
			break;
		case (right) :
			//mvprintw(0,0,"enter case right                 ");
			//getch();
			//wall check 
			switch (curShape.col) {
				case (8) :
					if (curShape.type == line && (curShape.rot == rot0 || curShape.rot == rot180))
						collision = 1;
					else if (curShape.type == Z || curShape.type == S)
						collision = 1;
					else if (curShape.rot != rot90 && ((curShape.type == T) || (curShape.type == J) || (curShape.type == L)))
						collision = 1;
					break;
				case(9) :
					if (curShape.type == line && (curShape.rot == rot90 || curShape.rot == rot270))
						collision = 1;
					else if (curShape.type == square)
						collision = 1;
					else if (curShape.rot == rot90 && ((curShape.type == J) || (curShape.type == L) || (curShape.type == T)))
						collision = 1;
					break;
			}	
			if (collision)
				return 2;


			//block check
			for (int j = 0; j < 4; j++) {
				for (int i = 0 ; i < 4; i++) {
					//check for collision on right moved once right
					//element by element 
					if (field[curShape.row+j][curShape.col+i] == empty) 
						continue;
					else if (field[curShape.row+j][curShape.col+i] == temp) {
						//check if piece left of it is full
						if(field[curShape.row+j][curShape.col+i+1] == full) {
							collision = 1;
						}
					} else if (field[curShape.row+j][curShape.col+i] == full) {
						//check if piece right of it is full
						if(field[curShape.row+j][curShape.col+i-1] == temp) {
							collision = 1;
						}
					}
				}
			}
			if (collision) {
				return 2;
			}

			if (!collision) {
				//mvprintw(0,0,"no collision was detected              ");
				//getch();
				clearShape();
				//mvprintw(0,0,"exited clearShape             ");
				//getch();
				//mvprintw(0,0,"updating curShape.col, now it's %d", curShape.col);
				//getch();
				curShape.col = curShape.col + 1;
				//mvprintw(0,0,"after it's %d                ", curShape.col);
				//getch();
				setShape(temp);
			}
			break;
		case (down) :
			//mvprintw(0,0,"enter case downi                         ");
			//getch();
			//mvprintw(0,0,"curShape.row = %d                 ", curShape.row);
			//getch();
			//floor check 
			switch (curShape.row) {
				case (HEIGHT-2) : //frame adjacent to the floor
					//mvprintw(0,0,"In case HEIGHT-2                 ");
					//getch();
					//check if there is room to move the piece down 1 (assuming no placed pieces)
					if (curShape.type == line && (curShape.rot == rot0 || curShape.rot == rot180))
						break; //leave collision = 0
					else if (curShape.rot == rot180 && ((curShape.type == T) || (curShape.type == J) || (curShape.type == L)))
						break; //leave collision = 0
					else { 
						setShape(full);
						return 1; //we're on the floor
					}
					break;
				case(HEIGHT-1) :
					//mvprintw(0,0,"In case HEIGHT-1                 ");
					//getch();
					//if we're here by now then the piece was already on the floor when we hit down so
					setShape(full);
					return 1;
			}	
			//block check
			for (int j = 0; j < 4; j++) {
				for (int i = 0 ; i < 4; i++) {
					//check for collision on down moved once down
					//element by element 
					if (field[curShape.row+j][curShape.col+i] == empty) 
						continue;
					else if (field[curShape.row+j][curShape.col+i] == temp) {
						//check if piece down from it is full
						if(field[curShape.row+j+1][curShape.col+i] == full) {
							setShape(full);
							return 1; //place the shape
						}
					} else if (field[curShape.row+j][curShape.col+i] == full) {
						//check if piece above it is full
						if(field[curShape.row+j-1][curShape.col+i] == temp) {
							collision = 1;
						}
					}
				}
			}
			if (!collision) {
				mvprintw(0,0,"no collision was detected              ");
				//getch();
				mvprintw(0,0,"just before clearshape       ");
				//getch();
				clearShape();
				//mvprintw(0,0,"exited clearShape             ");
				//getch();
				//mvprintw(0,0,"updating curShape.row, now it's %d", curShape.row+1);
				//getch();
				curShape.row = curShape.row + 1;
				//mvprintw(0,0,"after it's %d                ", curShape.row);
				//getch();
				setShape(temp);
			}
		default :			
			break;
   }
   return 0;
 }

//checks for lines formed by the recently placed block
//to be called after moveShape returns 1 and before calling addShape
//returns # lines cleared
int lineCheck() {
	for (int j = 0; j < 4; j++) {
		for (int i = 2; i < 12; i++) {
		//TODO: this	
		}
	}

}


//paints the field with 4x4 grid (should i call this a sprite? hmm) as specified in curShape
//assumes collision detection has already been done
void setShape(int type) {
	//mvprintw(0,0,"entering setShape               ");
	//getch();
	for (int j = 0; j < 4; j++) {
		for (int i = 0; i < 4; i++) {
			if (curShape.col > 0) { 
				if (field[curShape.row+j][curShape.col+i] != wall) {//don't accidentally clear the wall
					//mvprintw(0,0,"setting field[%d][%d] to %d", curShape.row+j,curShape.col+i,shapes[curShape.type][curShape.rot][j][i]);
					if (shapes[curShape.type][curShape.rot][j][i] == 1)	{
						switch (type) {
							case (temp) :
								field[curShape.row+j][curShape.col+i] = temp;
								break;
							case (full) :
								field[curShape.row+j][curShape.col+i] = full;
								break;
							default :
								break;
						}
					}
					else if ( field[curShape.row+j][curShape.col+i] == full) {
						; //leave as is
					} else {
						field[curShape.row+j][curShape.col+i] = empty; //redundant but whatever
					}
				}
			}
		}
	}
	//mvprintw(0,0,"leaving setShape               ");
	//getch();
}
//marks the 4x4 grid the shape currently occupies as empty
void clearShape() {	
	//mvprintw(0,0,"enter clearShape                ");
	for (int j = 0; j < 4 ; j++) {
		for (int i = 0; i < 4; i++){
			if(field[curShape.row+i][curShape.col+j] == wall || field[curShape.row+i][curShape.col+j] == full) {
				continue;
			}
			else
				field[curShape.row+i][curShape.col+j] = empty;
		}
	}
	//mvprintw(0,0,"leave clearShape                ");
}

void placeShape() {
//TODO: for now i'm just going to assume WIDTH = 10, can make this smarter later
//TODO: need to fix for collision on initial placement
	curShape.type = rand() % 7; //set the type to rand from 0 to 6
	curShape.row = 0; //initial shape frame always anchors
	curShape.col = 5; //to these coords
	curShape.rot= rot0; //always start in initial rotation
	for (int j = 0 ; j < 4 ; j++)
		for (int i = 0 ; i < 4 ; i++)
			field[j][i+5] = shapes[curShape.type][rot0][j][i];
   return;
}


//this function takes the cell information from the global field array and updates the window 
void updateDisplay(WINDOW *win) {
	//TODO: use attributes instead of characters to make the shapes/backgrounds
	for (int j = 2; j < HEIGHT+2; j++) { //we don't display the invisible top two rows
		for (int i = 2; i < WIDTH+2; i++) {
    		if (field[j][i] == empty)
    			mvwaddch(win,j-2,i-2,'.');	
    		else if (field[j][i] == temp)
    			mvwaddch(win,j-2,i-2,'*');	
    		else if (field[j][i] == full)
    			mvwaddch(win,j-2,i-2,'X');	
    	}
    }
	wrefresh(win);
    return;
}

int main(int argc, char *argv[]) {
	
	int max_y = 0, max_x = 0;
	time_t t;

	//init random number generator
	srand((unsigned) time(&t));
	rand(); //the first rand() seems to always be 0, so i use a throwaway call to rand() following the advice given in
			//https://stackoverflow.com/questions/7866754/why-does-rand-7-always-return-0
			//random numbers seems like a large enough topic to keep me busy forever but this is just
			//a dumb game so if it feels random enough its good enough

	//init ncurses stuff
	initscr();				//initializes the stdscr and starts ncurses up
	noecho();				//don't echo keypresses
	curs_set(FALSE);		//make the curor invisible
	cbreak();				//pass all keys one at a time, no line buffering, ctrl characters retain their meaning (seems like default behaviour anyway, but calling specifically)
	keypad(stdscr, TRUE);	//enable arrow keys

	//get the size of the open terminal window
	getmaxyx(stdscr, max_y, max_x);

	//create an outline for the window, borders are "inside" so need two extra rows/columns
	WINDOW *winbox = newwin(HEIGHT+2,WIDTH+2,max_y/2-HEIGHT/2,max_x/2-WIDTH/2);
	refresh(); //tricky, after creating a top level window, you have to refresh to the display
	box(winbox,0,0);
	wrefresh(winbox); //similarly, after changing a window, you have to refresh it

	//setup the window that will contain the playing field, which sits inside the border window (to keep zero indexing)
	//derwin is a derived window, meaning the coord system here is relative to the parent window
	WINDOW *win = derwin(winbox,HEIGHT,WIDTH,1,1);
	wrefresh(win);//apprently don't have to refresh() with windows created with derwin
	

	//init tetris display
	for (int j = 2 ; j < HEIGHT+2 ; j++)
		field[j][0] = field[j][1] = field[j][12] = wall; //set up the imaginary walls to make collision detection simpler
	updateDisplay(win);
	mvprintw(0,0,"Press any key to begin game");
	refresh();
	getch();	
	//mvprintw(0,0,"");

	placeShape();
	updateDisplay(win);
	
	while(1) {
		getmaxyx(stdscr, max_y, max_x);	//update the screen extents in case it got resized
		//TODO: stuff pertaining to resized window
		
		switch(getch()) { //get a keypress and take action
			case(KEY_LEFT) :
				//left key actions
				//mvprintw(0,0,"left key was pressed");
				//getch();
				if (moveShape(left) == 2) {
					; //couldn't move, do nothing
				}
				break;
			case(KEY_RIGHT) :
				//right key actions
				//mvprintw(0,0,"right key was pressed");
				moveShape(right);
				break;
			case(KEY_DOWN) :
				//down key actions
				//mvprintw(0,0,"down key was pressed");
				if(moveShape(down) == 1) {
					placeShape();
					updateDisplay(win);
					//check for line
					lineCheck();
				}
					
					
				break;
			default :
				//mvprintw(0,0,"some other key was pressed");
				break;
		}	
		//mvprintw(0,0,"back outside loop switch, about to updatedisplay        ");
		
		updateDisplay(win);	
		//refresh();
		usleep(DELAY);
	}
	//clean up functions that will be called if we ever deside to catch ctrl-c
	delwin(win);
	delwin(winbox);
	endwin();
	return 0;
}

