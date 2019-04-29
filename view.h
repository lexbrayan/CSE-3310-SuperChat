#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <panel.h>

using namespace std;

class View
{
  private:
    const int BSP = KEY_LEFT;      // Backspace
    const int ExitFKey = KEY_F(6); // F6 Function Key for Cancel

  public:
    string getUsername()
    {
        WINDOW *my_win;
        int startx, starty, width, height;

        initscr();            /* Start curses mode 		*/
        cbreak();             /* Line buffering disabled, Pass on
					 * everty thing to me 		*/
        keypad(stdscr, TRUE); /* I need that nifty F1 	*/

        height = 10;
        width = 50;
        starty = (LINES - height) / 2; /* Calculating for a center placement */
        startx = (COLS - width) / 2;   /* of the window		*/
        //printw("Press F1 to exit");

        refresh();
        my_win = create_newwin(height, width, starty, startx);
        //printw("enter username:");

        string username,password;
        char mesg[] = "Enter a username: ";
        //getmaxyx(stdscr, row, col);
        mvprintw(starty + 1, startx + 1, "%s", mesg);
        username = getstring();
        return username;
    }

    string getPassword()
    {
	int startx, starty, height, width;
	height = 10;
	width = 50;
	starty = (LINES - height)/2; //calculating for center placement
	startx = (COLS - width)/2; //of the window
	
	string password;
	char ch;
	const char RETURN = 10;
	char mesg[] = "Enter password: ";
	mvprintw(starty + 2, startx + 1, "%s", mesg);
	
	noecho();
	while((ch = getch()) != RETURN)
	{
	    password+=ch;
	}
	
	endwin();
	return password;
    }	

    std::string getstring()
    {
        std::string input;

        // let the terminal do the line editing
        nocbreak();
        echo();

        // this reads from buffer after <ENTER>, not "raw"
        // so any backspacing etc. has already been taken care of
        int ch = getch();

        while (ch != '\n')
        {
            input.push_back(ch);
            ch = getch();
        }

        // restore your cbreak / echo settings here

        return input;
    }

    string incorrectPassword()
    {
        WINDOW *my_win;
        int startx, starty, width, height;
        int ch;

        initscr();            /* Start curses mode              */
        cbreak();             /* Line buffering disabled, Pass on
                                         * everty thing to me           */
        keypad(stdscr, TRUE); /* I need that nifty F1   */

        height = 10;
        width = 50;
        starty = (LINES - height) / 2; /* Calculating for a center placement */
        startx = (COLS - width) / 2;   /* of the window         */

        refresh();
        my_win = create_newwin(height, width, starty, startx);

        string username, password;
        char title[]= "Superchat v1.0";
        char mesg0[] = "Incorrect Password for the given username";
        char mesg00[] = "New username will be created for";
        char mesg000[] = "further invalid inputs";
        char mesg[] = "Enter a username: ";
        int row, col;
        initscr();
        mvprintw(starty + 2, startx + 1, title);
        mvprintw(starty + 3, startx + 1, "%s", mesg0);
        mvprintw(starty + 4, startx + 1, "%s", mesg00);
        mvprintw(starty + 5, startx + 1, "%s", mesg000);
        mvprintw(starty + 7, startx + 1, "%s", mesg);
        username = getstring();
        char mesg2[] = "Enter password: ";
        mvprintw(starty + 8, startx + 1, "%s", mesg2);
        password = getstring();

        refresh();

        endwin(); /* End curses mode              */
        string master_str = username + "`" + password;
        return master_str;
    }

    int login()
    {
        char username[11];
        char mesg[] = "Enter a username: ";
        int row, col;
        initscr();
        getmaxyx(stdscr, row, col);
        mvprintw(row / 2, (col - strlen(mesg) / 2), "%s", mesg);
        getstr(username);
        getch();
        refresh();
        endwin();

        return 0;
    }

    WINDOW *create_newwin(int height, int width, int starty, int startx)
    {
        WINDOW *local_win;

        local_win = newwin(height, width, starty, startx);
        box(local_win, 0, 0); /* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
        wrefresh(local_win);  /* Show that box 		*/

        return local_win;
    }

    void destroy_win(WINDOW *local_win)
    {
        /* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
        wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        /* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
        wrefresh(local_win);
        delwin(local_win);
    }
};
#endif
