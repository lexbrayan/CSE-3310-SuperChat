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
#include "view.h"

using asio::ip::tcp;

typedef std::deque<chat_message> chat_message_queue;

class chat_client
{
public:
 
  string print_str;
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

                         //UN COMMENT THE FOLLOWING LINE for proper code
                         //printf("%s: ", read_msg_.decode_username());
                         //print_str=read_msg_.decode_username();
                         //ncurses.printHeader(read_msg_.decode_username());
                         do_read_body();
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
                       if (!ec)
                       {
                         //ncurses.printBody(read_msg_.body());
                         //string temp=read_msg_.body();
                         //print_str=print_str+" "+temp;
                         //printf("%s\n",print_str);
                         //UN COMMENT THE FOLLOWING LINE for proper code
                         //std::cout.write(read_msg_.body(), read_msg_.body_length());
                         //std::cout << "\n";
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
    char line[chat_message::max_body_length + 1];
    int chat_room_number = 0;

    //starting working from here
    string return_str = "";
    char password[11] = {'\0'};
    char username[11] = {'\0'};

    View ncurses;
    return_str = ncurses.getUsername();

    string return_str1 = return_str.substr(0, return_str.find("`"));
    string return_str2 = return_str.substr(return_str.find("`"));
    return_str1.copy(username, return_str1.size() + 1);
    username[return_str1.size()] = '\0';
    return_str2.copy(password, return_str2.size() + 1);
    password[return_str2.size()] = '\0';

    int size_of_below_array = 1;
    string master_usernames_of_this_user[50];
    string master_passwords_of_this_user[50];
    master_passwords_of_this_user[0] = return_str2;
    master_usernames_of_this_user[0] = return_str1;

    //std::cout << "Enter a username " << std::endl;
    //std::cin.getline(username, 11);

    {
    }

    string return_mssg = "";
    ncurses.buildChatScreen();

    //while (std::cin.getline(line, chat_message::max_body_length + 1))
    while (true)
    {
      return_mssg = ncurses.getMessage(username);

      return_mssg.copy(line, return_mssg.size() + 1);
      line[return_mssg.size()] = '\0';

      //ncurses.displayMessage(line);
      //Create a message that's a command to change rooms and send it to the server
      if (line[0] == '/' && line[1] >= 48 && line[1] <= 57)
      {
        //c.change_room(1);
        chat_message msg;
        msg.set_crn(chat_room_number);
        chat_room_number = (int)(line[1] - '0');
        msg.set_nrn(chat_room_number);
        msg.set_cmd(1);
        msg.encode_header();
        c.write(msg);
        //ncurses.displayMessage(username,msg.data_);
      }
      else if (line[0] == '/' && line[1] == 'e' && line[2] == 'x' && line[3] == 'i' && line[4] == 't')
      {

        bool flag = 0;
        return_str = ncurses.getUsername();
        char password_input[11] = {'\0'};
        char username_input[11] = {'\0'};
        string return_str1 = return_str.substr(0, return_str.find("`"));
        string return_str2 = return_str.substr(return_str.find("`"));
        return_str1.copy(username_input, return_str1.size() + 1);
        username_input[return_str1.size()] = '\0';
        return_str2.copy(password_input, return_str2.size() + 1);
        password_input[return_str2.size()] = '\0';

        for (int i = 0; i < size_of_below_array; i++)
        {
          if (return_str2 == master_passwords_of_this_user[i] && return_str1 == master_usernames_of_this_user[i])
          {
            chat_message msg;
            msg.set_crn(0);
            msg.set_nrn(0);
            msg.set_cmd(1);
            msg.encode_header();
            c.write(msg);
            ncurses.buildChatScreen();
            flag = 1;
          }
        }
        if (flag == 0)
        {
          return_str = ncurses.incorrectPassword();
          size_of_below_array++;
          char password_input[11] = {'\0'};
          char username_input[11] = {'\0'};
          string return_str1 = return_str.substr(0, return_str.find("`"));
          string return_str2 = return_str.substr(return_str.find("`"));
          return_str1.copy(username_input, return_str1.size() + 1);
          username_input[return_str1.size()] = '\0';
          return_str2.copy(password_input, return_str2.size() + 1);
          password_input[return_str2.size()] = '\0';
          master_passwords_of_this_user[size_of_below_array - 1] = return_str2;
          master_usernames_of_this_user[size_of_below_array - 1] = return_str1;
          chat_message msg;
          msg.set_username(username_input);

          return_str1.copy(username, return_str1.size() + 1);
          username[return_str1.size()] = '\0';
          return_str2.copy(password, return_str2.size() + 1);
          password[return_str2.size()] = '\0';

          msg.set_cmd(0);
          msg.set_crn(0);
          msg.body_length(std::strlen(line));
          std::memcpy(msg.body(), line, msg.body_length());
          msg.encode_header(); //neat idea
          ncurses.buildChatScreen();
          //c.write(msg);
        }
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
        ncurses.changeCursor(username,return_mssg);
        c.write(msg);        //THIS COMMAND SENDS message to server
        //ncurses.printSentMessage(c.print_str);
        //ncurses.displayMessage(username,msg.data());
      }
    }

    c.close();
    t.join();
  }
  catch (std::exception &e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
