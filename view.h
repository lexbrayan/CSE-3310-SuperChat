#ifndef VIEW_H
#define VIEW_H

#include <ncurses.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <panel.h>
#include <time.h>

using namespace std;

class View
{
  private:
    const int BSP = KEY_LEFT;      // Backspace
    const int ExitFKey = KEY_F(6); // F6 Function Key for Cancel

  public:
    // WINDOW *my_win;
    WINDOW *message_window;
    WINDOW *message_type;
    int master_startx,master_starty;
    int x_type,y_type;
    int x_window,y_window;
    int i;
    time_t my_time=time(NULL);
    int counter=3;
    string chatHistory[10][10];
    /*for(int i=0;i<10;i++)
    {
        chatHistory[i][0]="Superchat v1.0";
        chatHistory[i][1]=ctime(&my_time);
        //"string time = ctime(&my_time);
    }*/
        

    string getUsername()
    {
        wclear(message_type);
        wclear(message_window);

        WINDOW *my_win;
        int startx, starty, width, height;
        int ch;

        initscr();            /* Start curses mode 		*/
        cbreak();             /* Line buffering disabled, Pass on
					 * everty thing to me 		*/
        //keypad(stdscr, TRUE); /* I need that nifty F1 	*/

        height = 10;
        width = 50;
        starty = (LINES - height) / 2; /* Calculating for a center placement */
        startx = (COLS - width) / 2;   /* of the window		*/
        //printw("Press F1 to exit");

        refresh();
        my_win = create_newwin(height, width, starty, startx);
        //printw("enter username:");

        string username, password;
        string time = ctime(&my_time);
        char title[]= "Superchat v1.0";
        char mesg[] = "Enter a username: ";
        
        initscr();
        //getmaxyx(stdscr, row, col);
        mvprintw(starty + 1, startx + 1, time.c_str());
        mvprintw(starty + 2, startx + 1, title);
        mvprintw(starty + 3, startx + 1, "%s", mesg);
        username = getstring();
        char mesg2[] = "Enter password: ";
        mvprintw(starty + 4, startx + 1, "%s", mesg2);
        password = getstring();

        

        //getch();
        refresh();

        /*
        while ((ch = getch()) != KEY_F(1))
        {
            switch (ch)
            {
            case KEY_LEFT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, --startx);
                break;
            case KEY_RIGHT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, ++startx);
                break;
            case KEY_UP:
                destroy_win(my_win);
                my_win = create_newwin(height, width, --starty, startx);
                break;
            case KEY_DOWN:
                destroy_win(my_win);
                my_win = create_newwin(height, width, ++starty, startx);
                break;
            }
        }*/

        //while(1);
        endwin(); /* End curses mode		  */
        string master_str = username + "`" + password;
        return master_str;
    }

    string incorrectPassword()
    {
        wclear(message_type);
        wclear(message_window);
        WINDOW *my_win;
        int startx, starty, width, height;
        int ch;

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

        string username, password;
        string time = ctime(&my_time);
        char title[]= "Superchat v1.0";
        char mesg0[] = "Incorrect Password for the given username";
        char mesg00[] = "New username will be created for";
        char mesg000[] = "further invalid inputs";
        char mesg[] = "Enter a username: ";
        int row, col;
        initscr();
        //getmaxyx(stdscr, row, col);
        mvprintw(starty + 1, startx + 1, time.c_str());
        mvprintw(starty + 2, startx + 1, title);
        mvprintw(starty + 3, startx + 1, "%s", mesg0);
        mvprintw(starty + 4, startx + 1, "%s", mesg00);
        mvprintw(starty + 5, startx + 1, "%s", mesg000);
        mvprintw(starty + 7, startx + 1, "%s", mesg);
        username = getstring();
        char mesg2[] = "Enter password: ";
        mvprintw(starty + 8, startx + 1, "%s", mesg2);
        password = getstring();

        //getch();
        refresh();

        /*
        while ((ch = getch()) != KEY_F(1))
        {
            switch (ch)
            {
            case KEY_LEFT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, --startx);
                break;
            case KEY_RIGHT:
                destroy_win(my_win);
                my_win = create_newwin(height, width, starty, ++startx);
                break;
            case KEY_UP:
                destroy_win(my_win);
                my_win = create_newwin(height, width, --starty, startx);
                break;
            case KEY_DOWN:
                destroy_win(my_win);
                my_win = create_newwin(height, width, ++starty, startx);
                break;
            }
        }*/

        //while(1);
        endwin(); /* End curses mode		  */
        string master_str = username + "`" + password;
        return master_str;
    }

    void buildChatScreen()
    {
        //WINDOW *message_window;
        //WINDOW *message_type;
        int startx, starty, width1, height1,width2, height2;
        int ch;
        //refresh();
        initscr();            /* Start curses mode 		*/
        cbreak();             /* Line buffering disabled, Pass on
					 * everty thing to me 		*/
        //keypad(stdscr, TRUE); /* I need that nifty F1 	*/

        height1 = 15;
        width1 = 50;
        height2=5;
        width2=50;
        starty = (LINES - height1) / 2; /* Calculating for a center placement */
        startx = (COLS - width1) / 2;   /* of the window		*/
        //printw("Press F1 to exit");

        
        message_window = create_newwin(height1, width1, starty, startx);
        string time = ctime(&my_time);
        char title[]= "Superchat v1.0";
        mvprintw(starty + 1, startx + 1, time.c_str());
        mvprintw(starty + 2, startx + 1, title);
        x_window=startx+1;
        y_window=starty+3;
        message_type = create_newwin(height2, width2, starty + 14, startx);

        master_startx=startx;
        master_starty=starty;

        

        wrefresh(message_window);
        wrefresh(message_type);
        //int c=getch();
        endwin();
        //wgetch(message_window);
        //wgetch(message_type);
    }

    string getMessage(char *username)
    {      
        //refresh();
        int height2=5;
        int width2=50;
        //wclear(message_type); 
        int starty=master_starty;
        int startx=master_startx;
        x_type=startx;
        y_type=starty+14;
        message_type = create_newwin(height2, width2, y_type,x_type);       
        
        //printf("%d %d",starty,startx);
        //string message="";
        char mesg[11];
        
        //message="";
        //char mesg[] = "Enter a username: ";
        initscr();
        //getmaxyx(stdscr, row, col);
        //mvprintw( starty + 16, startx + 1, username);
        //mvprintw( starty+16, strlen(username) + 1 + startx, "%s", ": ");
        move(starty+16,startx+1);
        master_starty=starty;
        master_startx=startx;
        wrefresh(message_type);
        getstr(mesg);
        string message(mesg);
        //message = getstring();
        
        //clrtoeol();
        //wclear(message_type);
        
        
       

        //getch();
        //refresh();

        
        endwin(); /* End curses mode		  */
        return message;
    }

    void changeCursor(char* username, string body)
    {
        int height1 = 15;
        int width1 = 50;
        
        int starty = (LINES - height1) / 2; /* Calculating for a center placement */
        int startx = (COLS - width1) / 2;   /* of the window		*/
        message_window = create_newwin(height1, width1, starty, startx);
        string time = ctime(&my_time);
        char title[]= "Superchat v1.0";
        mvprintw(starty + 1, startx + 1, time.c_str());
        mvprintw(starty + 2, startx + 1, title);
        move(starty+counter,startx+1);
        counter++;
        string master(username);
        //string master2(body);
        master=master+": "+body;
        printw(master.c_str()); //this can be replaced by actual message.
        //printlw(": ");
        //printlw(body);
        wrefresh(message_window);
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