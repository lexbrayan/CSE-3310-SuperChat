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
#include <deque>
#include <iostream>
#include <thread>
#include "asio.hpp"
#include "chat_message.hpp"
#include <ncurses.h>
#include <time.h>
#include <string>

using asio::ip::tcp;
using namespace std;

typedef std::deque<chat_message> chat_message_queue;

WINDOW *chat_window; //CHANGE
WINDOW *type_window; //CHANGE
int kounter = 3;     //CHANGE
int mheight1, mwidth1, mstarty, mstartx; //CHANGE
string mtime; //CHANGE

class chat_client
{
public:
  chat_client(asio::io_context &io_context,
              const tcp::resolver::results_type &endpoints)
      : io_context_(io_context),
        socket_(io_context)
  {
    do_connect(endpoints);
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

  void makeWindowagain() //CHANGE
  {
    string title = "SuperChat v1.0";
    chat_window = newwin(mheight1, mwidth1, mstarty, mstartx);
    box(chat_window, 0, 0);
    scrollok(chat_window, TRUE);
    mvprintw(mstarty + 1, mstartx + 1, "%s", title.c_str());
    mvprintw(mstarty + 2, mstartx + 1, "%s", mtime.c_str());
    move(mstarty + 16, mstartx + 1);
    wrefresh(chat_window);
  }

  std::string getstring() //CHANGE
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

private:
  //This connects the client with the server and then attemps to
  //Start reading messages from the chatroom
  void do_connect(const tcp::resolver::results_type &endpoints)
  {
    asio::async_connect(socket_, endpoints,
                        [this](std::error_code ec, tcp::endpoint) {
                          if (!ec)
                          {
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
                       if (!ec && read_msg_.decode_header())
                       {
                         do_read_body();
                       }
                       else
                       {
                         socket_.close();
                       }
                     });
  }

  void do_read_body() //CHANGE
  {
    asio::async_read(socket_,
                     asio::buffer(read_msg_.body(), read_msg_.body_length()),
                     [this](std::error_code ec, std::size_t /*length*/) {
                       if (!ec)
                       {
                         if (kounter > 13)
                         {
                           makeWindowagain();
                           kounter = 3;
                         }

                         //std::cout.write(read_msg_.body(), read_msg_.body_length());
                         //std::cout << "\n";

                         string temp2 = read_msg_.body();
                         temp2 = temp2.substr(0, read_msg_.body_length());

                         mvwprintw(chat_window, kounter, 1, temp2.c_str());
                         kounter++;
                         wrefresh(chat_window);

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
    int chat_room_number = 0;

    initscr();
    cbreak();

    int height = 10;
    int width = 50;
    int startx = (COLS - width) / 2;
    int starty = (LINES - height) / 2;
    time_t my_time = time(NULL);
    string time = ctime(&my_time);

    string title = "Superchat v1.0";
    string get_username = "Please input the username: ";
    char username[16];
    WINDOW *login_window = newwin(height, width, starty, startx);
    box(login_window, 0, 0);
    refresh();
    mvprintw(starty + 1, startx + 1, "%s", title.c_str());
    mvprintw(starty + 2, startx + 1, "%s", time.c_str());
    mvprintw(starty + 4, startx + 1, "%s", get_username.c_str());
    wrefresh(login_window);

    string username_str = c.getstring();
    username_str.copy(username, username_str.size() + 1);
    username[username_str.size()] = '\0';

    delwin(login_window);
    refresh();
    endwin();

    initscr();
    cbreak();
    int height1 = 15;
    int width1 = 50;
    int height2 = 5;
    int width2 = 50;

    chat_window = newwin(height1, width1, starty, startx);
    box(chat_window, 0, 0);
    scrollok(chat_window, TRUE);
    time = ctime(&my_time);
    mvprintw(starty + 1, startx + 1, "%s", title.c_str());
    mvprintw(starty + 2, startx + 1, "%s", time.c_str());
    mheight1 = height1; //CHANGE
    mwidth1 = width1; //CHANGE
    mstartx = startx; //CHANGE
    mstarty = starty; //CHANGE
    mtime = time; //CHANGE

    type_window = newwin(height2, width2, starty + 14, startx);
    box(type_window, 0, 0);
    move(starty + 16, startx + 1);
    wrefresh(chat_window);
    wrefresh(type_window);

    kounter = 3;
    chat_message msg;
    msg.set_crn(0);
    msg.set_nrn(0);
    msg.set_cmd(1);
    msg.encode_header();
    c.write(msg);

    while (true)
    {
      delwin(type_window);
      type_window = newwin(height2, width2, starty + 14, startx);
      box(type_window, 0, 0);
      move(starty + 16, startx + 1);
      wrefresh(type_window);
      string temp = c.getstring();
      temp.copy(line, temp.size() + 1);
      line[temp.size()] = '\0';

      if (line[0] == '/' && line[1] >= 48 && line[1] <= 57)
      {
        kounter = 3;
        delwin(chat_window);
        chat_window = newwin(height1, width1, starty, startx);
        box(chat_window, 0, 0);
        time = ctime(&my_time);
        mvprintw(starty + 1, startx + 1, "%s", title.c_str());
        mvprintw(starty + 2, startx + 1, "%s", time.c_str());
        wrefresh(chat_window);
        chat_message msg;
        msg.set_crn(chat_room_number);
        chat_room_number = (int)(line[1] - '0');
        msg.set_nrn(chat_room_number);
        msg.set_cmd(1);
        msg.encode_header();
        c.write(msg);
      }
      else if (line[0] == '/' && line[1] == 'e' && line[2] == 'x' && line[3] == 'i' && line[4] == 't')
      {
        break; //CHANGE
      }
      else
      {
        chat_message msg;
        msg.set_username(username);
        msg.set_cmd(0);
        msg.set_crn(chat_room_number);
        msg.body_length(std::strlen(line));
        std::memcpy(msg.body(), line, msg.body_length());
        msg.encode_header(); //neat idea
        c.write(msg);
      }
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
