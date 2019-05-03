//
// chat_client.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
//#include <stdlib.h>
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"
#include <ncurses.h>
#include <time.h>
#include "dictionary.cpp"
#include <string>
#include <algorithm>

using asio::ip::tcp;
using namespace std;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
  chat_client(asio::io_context &io_context,
              const tcp::resolver::results_type &endpoints)
      : io_context_(io_context),
        socket_(io_context)
  {
    chat_window = NULL;
    type_window = NULL;
    do_connect(endpoints);
    std::strcpy(chatroom_name, "Lobby");
    for (int i = 0; i < 50; i++)
    {
      block_list[i] = NULL;
    }

    initscr();
    cbreak();
    noecho();
    time_t my_time = time(NULL);
    char times[30];
    std::strcpy(times, ctime(&my_time));
    times[29] = '\0';
    char title[] = "Superchat v1.0";
    string get_username = "Please input the username: ";
    WINDOW *login_window = newwin(LINES, COLS, 0, 0);
    //wrefresh(login_window);
    //move(starty+1, startx+1);
    //wprintw(login_window, "apple\n");
    //mvwprintw(login_window, 2, 1, "banana\n");

    mvwprintw(login_window, 1, 1, "%s", title);
    mvwprintw(login_window, 2, 1, "%s", times);
    mvwprintw(login_window, 4, 1, "%s", get_username.c_str());
    box(login_window, 0, 0);
    wrefresh(login_window);
    //sleep(2);
    //c.getstring
    char *username_str = getstring(login_window, 4, 27, COLS - 0.2 * COLS);
    //username_str.copy(username, username_str.size() + 1);
    //username[username_str.size()] = '\0';
    std::strncpy(username, username_str, 16);
    username[16] = '\0';
    free(username_str);

    delwin(login_window);
    refresh();
    endwin();
  }

  //Write takes a message and if it's the first message it calls do_write
  //to write the message which in turn goes through and writes all the
  //messages currently waiting to be processed
  void write(const chat_message &msg)
  {
    asio::post(io_context_,
               [this, msg]() {
                 bool write_in_progress = !write_msgs_.empty();
                 write_msgs_.push_back(msg);
                 if (!write_in_progress)
                 {
                   do_write();
                 }
               });
  }

  void close()
  {
    asio::post(io_context_, [this]() { socket_.close(); });
  }

  void set_chatname(char *name)
  {
    std::strcpy(chatroom_name, name);
  }

  char *get_username()
  {
    return username;
  }

  char *get_server_response()
  {
    return chatroom_name;
  }

  void block(char *name)
  {
    //printf("At least we're trying to block %s.\n", name);
    for (int i = 0; i < 50; i++)
    {
      if (block_list[i] == NULL)
      {
        block_list[i] = (char *)malloc((std::strlen(name) + 1) * sizeof(char));
        std::strcpy(block_list[i], name);
        //printf("You are now blocking %s.\n", block_list[i]);
        break;
      }
    }
  }

  void unblock(char *name)
  {
    for (int i = 0; i < 50; i++)
    {
      if (block_list[i] != NULL && std::strcmp(block_list[i], name) == 0)
      {
        //printf("You are no longer blocking %s.\n", block_list[i]);
        free(block_list[i]);
        break;
      }
    }
  }

  int blocked(char *name)
  {
    //printf("At least we're cheacking to see if %s is being blocked\n", name);
    for (int i = 0; i < 50; i++)
    {
      if (block_list[i] != NULL)
      {
        if (std::strcmp(block_list[i], name) == 0)
        {
          //printf("blocked a message from %s.\n", name);
          return 1;
        }
      }
    }
    return 0;
  }

  //void suggest

  void delete_type_window()
  {
    //mvwprintw(type_window, 0, 0, "                                                                                                                                 ");
    //wprintw(type_window, "                                                                                                                                 ");
    //wprintw(type_window, "                                                                                                                                 ");
    //wprintw(type_window, "                                                                                                                                 ");
    //box(type_window, ' ', ' ');
    //wrefresh(type_window);
    delwin(type_window);
  }

  void delete_chat_window()
  {
    delwin(chat_window);
  }

  void create_type_window(int h, int w, int y, int x)
  {
    type_window = newwin(h, w, y, x);
    scrollok(type_window, TRUE);
    //idlok(chat_window, TRUE);
    box(type_window, 0, 0);
    //wmove(type_window, 1, 1);
    wrefresh(type_window);
  }

  void create_chat_window(int h, int w, int y, int x, char *title, char *time)
  {
    chat_window = newwin(h, w, y, x);
    scrollok(chat_window, TRUE);
    //idlok(chat_window, TRUE);
    wprintw(chat_window, "\n");
    wprintw(chat_window, "|%s\n", title);
    wprintw(chat_window, "|%s\n", time);
    box(chat_window, 0, 0);
    wrefresh(chat_window);
  }

  WINDOW *get_type_window()
  {
    return type_window;
  }
  WINDOW *get_chat_window()
  {
    return chat_window;
  }

  void refresh_data()
  {
    box(chat_window, 0, 0);
    box(type_window, 0, 0);
    wrefresh(chat_window);
    wrefresh(type_window);
  }

  char *getstring(WINDOW *win, int y, int x, int print) //CHANGE
  {
    char *input = (char *)malloc(200 * sizeof(char));
    memset(input, '\0', 200 * sizeof(char));
    int i = 0;
    // let the terminal do the line editing
    //nocbreak();
    noecho();
    keypad(win, TRUE);

    // this reads from buffer after <ENTER>, not "raw"
    // so any backspacing etc. has already been taken care of
    wmove(win, y, x + 1);
    int ch = wgetch(win);

    while (ch != '\n')
    {
      if (i == 0 && ch == KEY_BACKSPACE)
      {
      }
      else if (i > 0 && ch == KEY_BACKSPACE)
      {
        //exit(1);
        mvwdelch(win, y, i + x);
        mvwdelch(win, y, print - 2);
        wmove(win, y, i + x);
        box(win, 0, 0);
        //wdelch(win);
        wrefresh(win);
        i--;
        input[i] = '\0';
      }
      /*else if(ch == )
      {

      }*/
      else if (i >= print - 2)
      {
        //I want to keep the text in type_window from deleting the boundry
        //But it doesn't seem to work
        /*wdelch(win);
        mvwdelch(win,1,48);
        mvwprintw(win,1,48,"%c", '|');
        wrefresh(win);*/
      }
      else
      {
        wprintw(win, "%c", (char)ch);
        wrefresh(win);
        input[i] = (char)ch;
        i++;
      }
      ch = wgetch(win);
    }
    cbreak();
    echo();
    wrefresh(win);
    // restore your cbreak / echo settings here
    return input;
  }

  bool checkforWeird(string temp)
  {
    unsigned int i=0;
    for (i = 0; i < temp.length(); i++)
    {
      char c1 = temp[i];
      //char c2 = temp[i + 1];
      if (c1 == '^')
      {
        return 0;
      }
    }
    return 1;
  }

  std::string trim(const std::string &s)
  {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start))
    {
      start++;
    }

    auto end = s.end();
    do
    {
      end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
  }

private:
  //This connects the client with the server and then attemps to
  //Start reading messages from the chatroom
  void do_connect(const tcp::resolver::results_type &endpoints)
  {
    asio::async_connect(socket_, endpoints,
                        [this](std::error_code ec, tcp::endpoint) {
                          if (!ec)
                          {
                            while (chat_window == NULL || type_window == NULL)
                            {
                            }
                            do_read_header();
                          }
                        });
  }

  //do_read_header and do_read_body call each other
  //creating an infinite cycle that breaks if the message
  //decode returns false (maybe it failed)
  void do_read_header()
  {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.data(), chat_message::header_length),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec && read_msg_.decode_header() && read_msg_.decode_command() == 0)
                       {
                         //printf("...%s...\n",read_msg_.decode_username());
                         if (blocked(read_msg_.decode_username()) == 1)
                         {
                           //usleep(1000);
                           do_read_body();
                         }
                         else
                         {
                           char name[20] = {'\0'};
                           std::strncpy(name, read_msg_.decode_username(), 16);
                           wprintw(chat_window, "|%s: ", name);
                           //std::cout.write(read_msg_.decode_username(), 10);
                           //std::cout.write(": ", 2);
                           do_read_body();
                         }
                       }
                       else if (!ec && read_msg_.decode_header() && read_msg_.decode_command() == 3)
                       {
                         //printf("%s: ",read_msg_.decode_username());
                         //std::cout.write(read_msg_.decode_username(), 10);
                         //std::cout.write(": ", 2);
                         //printf("Caught the error, returning to %s.\n", read_msg_.decode_chatname_old());
                         if (std::strcmp(chatroom_name, read_msg_.decode_chatname_new()) != 0)
                         {
                           std::strcpy(chatroom_name, read_msg_.decode_chatname_old());
                           do_read_body();
                         }
                         else
                         {
                           do_read_header();
                         }
                       }

                       else
                       {
                         socket_.close();
                       }
                     });
  }

  void do_read_body()
  {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.body(), read_msg_.body_length()),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec && !blocked(read_msg_.decode_username()) && read_msg_.decode_command() == 0)
                       {
                         char input[500] = {'\0'};
                         std::strncpy(input, read_msg_.body(), read_msg_.body_length());
                         //temp2 = temp2.substr(0, read_msg_.body_length());
                         //wmove(chat_window, 0, 1);
                         wprintw(chat_window, "%s\n", input);
                         //wmove(type_window, 1, 1);
                         box(chat_window, 0, 0);
                         //kounter++;
                         wrefresh(chat_window);
                         wrefresh(type_window);

                         do_read_header();
                       }
                       else if (!ec)
                       {
                         do_read_header();
                       }
                       else
                       {
                         socket_.close();
                       }
                     });
  }

  //This writes all the messages sent to other users over the server, and
  //dequeues each message as it goes until there are no messages left in the queue
  void do_write()
  {
    asio::async_write(socket_,
                      asio::buffer(write_msgs_.front().data(),
                                   write_msgs_.front().length()),
                      [this](std::error_code ec, std::size_t /*length*/) {
                        if (!ec)
                        {
                          write_msgs_.pop_front();
                          if (!write_msgs_.empty())
                          {
                            do_write();
                          }
                        }
                        else
                        {
                          socket_.close();
                        }
                      });
  }

private:
  asio::io_context &io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  char username[17];
  WINDOW *chat_window; //CHANGE
  WINDOW *type_window; //CHANGE
  char chatroom_name[20];
  char *block_list[50];
};

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }

    asio::io_context io_context;
    //Some magic to connect to the server
    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);
    std::thread t([&io_context]() { io_context.run(); });
    //std::cin.getline evaluates to true in a boolean context as long as there's no error
    //(bad bit and fail bit aren't set to true) so this loops forever getting messages
    //from std::cin

    //CHANGE from here
    char line[chat_message::max_body_length + 1];
    char title[] = "Superchat v1.0";

    int starty = 0;
    int startx = 0;
    //clock my_clock();
    time_t my_time = time(NULL);
    char times[30];
    std::strcpy(times, ctime(&my_time));
    times[29] = '\0';

    char chatroom_name[20] = "Lobby";
    char username[16];
    std::strncpy(username, c.get_username(), 15);
    username[15] = '\0';
    initscr();
    cbreak();
    refresh();
    startx = COLS;
    starty = LINES;
    int room_width = 0.2 * COLS;
    int height1 = starty - 5;
    int width1 = startx - room_width;
    int height2 = 5;
    int width2 = startx - room_width;
    startx = 0;
    starty = 0;

    WINDOW *room_window = newwin(height1 + height2, room_width, starty, startx + width1);
    box(room_window, 0, 0);
    wrefresh(room_window);

    char dictionary[] = "dictionary.txt";
    Dictionary dict(dictionary);

    std::strcpy(times, ctime(&my_time));
    //system("clear");
    c.create_chat_window(height1, width1, starty, startx, title, times);

    //mheight1 = height1; //CHANGE
    //mwidth1 = width1; //CHANGE
    //mstartx = startx; //CHANGE
    //mstarty = starty; //CHANGE
    //mtime = times; //CHANGE

    c.create_type_window(height2, width2, starty + height1, startx);

    //kounter = 3;
    chat_message msg;
    msg.set_crn(0);
    msg.set_nrn(0);
    msg.set_cmd(1);
    msg.encode_header();
    c.write(msg);

    while (true)
    {
      c.delete_type_window();
      c.create_type_window(height2, width2, starty + height1, startx);
      string temp = c.getstring(c.get_type_window(), 1, 0, COLS - 0.2 * COLS);
      temp.copy(line, temp.size() + 1);
      line[temp.size()] = '\0';

      //==================================================================================================================================
      if (std::strlen(line) == 0)
      {
        continue;
      }
      else if (std::strlen(line) >= 4 && line[0] == '/' && line[1] == 'e')
      {
        //c.change_room(1);
        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        c.delete_chat_window();
        std::strcpy(times, ctime(&my_time));
        c.create_chat_window(height1, width1, starty, startx, title, times);

        char *token = strtok(line, " ");
        token = strtok(NULL, " ");
        chat_message msg;
        msg.set_chatname_current(chatroom_name);
        strcpy(chatroom_name, token);
        msg.set_chatname_new(chatroom_name);
        msg.set_cmd(1);
        msg.encode_header();
        c.set_chatname(chatroom_name);
        c.write(msg);
      }
      else if (std::strlen(line) >= 4 && line[0] == '/' && line[1] == 'c')
      {
        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        c.delete_chat_window();
        std::strcpy(times, ctime(&my_time));
        c.create_chat_window(height1, width1, starty, startx, title, times);

        char *token = strtok(line, " ");
        token = strtok(NULL, " ");
        chat_message msg;
        msg.body_length(0);
        msg.set_chatname_current(chatroom_name);
        strcpy(chatroom_name, token);
        msg.set_chatname_new(chatroom_name);
        msg.set_cmd(2);
        msg.encode_header();
        c.set_chatname(chatroom_name);
        c.write(msg);
      }
      else if (std::strlen(line) >= 4 && line[0] == '/' && line[1] == 'd')
      {
        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        char *token = strtok(line, " ");
        token = strtok(NULL, " ");
        char input[20];
        std::strcpy(input, token);
        chat_message msg;
        msg.body_length(0);
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(input);
        msg.set_cmd(3);
        msg.encode_header();
        c.set_chatname(chatroom_name);
        c.write(msg);
      }
      else if (std::strlen(line) >= 4 && line[0] == '/' && line[1] == 'b')
      {
        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        char *token = strtok(line, " ");
        token = strtok(NULL, " ");
        char input[std::strlen(token) + 1];
        std::strcpy(input, token);
        //printf("blocking: %s.\n", input);
        c.block(input);
      }
      else if (std::strlen(line) >= 4 && line[0] == '/' && line[1] == 'u')
      {
        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        char *token = strtok(line, " ");
        token = strtok(NULL, " ");
        char input[20];
        std::strcpy(input, token);
        //printf("unblocking: %s.\n", input);
        c.unblock(input);
      }
      else if (std::strlen(line) == 2 && line[0] == '/' && line[1] == 'h')
      {
        c.delete_type_window();
        c.delete_chat_window();
        WINDOW *help = newwin(11, 100, 0, 0);
        wprintw(help, "\n");
        wprintw(help, "|This is the help menu, here are the avaliable commands:\n");
        wprintw(help, "|%-25s(creates a room called <room name>)\n", "/c <room name>");
        wprintw(help, "|%-25s(enters a room called <room name>)\n", "/e <room name>");
        wprintw(help, "|%-25s(deletes a room called <room name>)\n", "/d <room name>");
        wprintw(help, "|%-25s(blocks the user called <user name>)\n", "/b <user name>");
        wprintw(help, "|%-25s(unblocks the user called <user name>)\n", "/u <user name>");
        wprintw(help, "|%-25s(displays this help menu)\n", "/h");
        wprintw(help, "|%-25s(exits the program)\n", "/q");
        wprintw(help, "|Press any key to continue...");
        box(help, 0, 0);
        wrefresh(help);
        wgetch(help);
        delwin(help);
        //c.delete_chat_window();
        std::strcpy(times, ctime(&my_time));
        c.create_chat_window(height1, width1, starty, startx, title, times);
        c.create_type_window(height2, width2, starty + height1, startx);
        chat_message msg;
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(chatroom_name);
        msg.set_cmd(1);
        msg.encode_header();
        c.set_chatname(chatroom_name);
        c.write(msg);
      }
      else if (std::strlen(line) == 2 && line[0] == '/' && line[1] == 'q')
      {
        break;
      }
      else
      {
        //c.delete_type_window();
        //c.create_type_window(height2, width2, starty + height1, startx);
        chat_message msg;
        //char answer[4];

        //char suggestion[500];
        char *temp;
        temp = dict.spellcheck(line);
        string temp2(temp);
        //string suggestion = temp2;
        string temp3(line);
        //std::strcpy(suggestion, temp);
        free(temp);
        //if (std::strcmp(line, suggestion) != 0)
        if (temp2.compare(temp3) != 0 && c.checkforWeird(temp2))
        {
          //c.suggest_spelling(&line, suggestion);
          wprintw(c.get_type_window(), "\n");
          temp2 = temp2.substr(temp2.find(" "));
          temp2 = c.trim(temp2);
          if (temp2.compare(temp3) == 0)
          {
          }
          else
          {
            wprintw(c.get_type_window(), "|did you mean: %s\n", temp2.c_str());
            wprintw(c.get_type_window(), "|type 'y' for corrected line or type 'n' for original line.");
            int ch = wgetch(c.get_type_window());
            if (ch == 'y' || ch == 'Y')
            {
              //std::strcpy(line, suggestion);
              temp3 = temp2;
              //temp3=temp3.substr(temp3.find(" "));
              strcpy(line, temp3.c_str());
            }
          }
        }

        c.delete_type_window();
        c.create_type_window(height2, width2, starty + height1, startx);
        //printf("?\n");
        msg.set_username(username);
        msg.set_cmd(0);
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(chatroom_name);
        msg.body_length(std::strlen(line));
        std::memcpy(msg.body(), line, msg.body_length());
        msg.encode_header(); //neat idea
        c.write(msg);
      }
      //my_time = time(NULL);
      //std::strcpy(times, ctime(&my_time));
      c.refresh_data();
      usleep(500); //wait half a second for the server to process a message sent
      if (strcmp(chatroom_name, c.get_server_response()) != 0)
      {
        strcpy(chatroom_name, c.get_server_response());
        chat_message msg;
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(chatroom_name);
        msg.set_cmd(1);
        msg.encode_header();
        c.write(msg);
      }
      c.refresh_data();

      //==================================================================================================================================
    }

    endwin();
    c.close();
    t.join();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
