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
  chat_client(asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
    std::strcpy(chatroom_name, "Lobby");
    for(int i=0; i<50; i++)
    {
      block_list[i] = NULL;
    }
  }

  //Write takes a message and if it's the first message it calls do_write
  //to write the message which in turn goes through and writes all the 
  //messages currently waiting to be processed
  void write(const chat_message& msg)
  {
    asio::post(io_context_,
        [this, msg]()
        {
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

  void set_chatname(char* name)
  {
    std::strcpy(chatroom_name, name);
  }

  char* get_server_response()
  {
    return chatroom_name;
  }

  void block(char* name)
  {
    //printf("At least we're trying to block %s.\n", name);
    for(int i=0; i<50; i++)
    {
      if(block_list[i] == NULL)
      {
        block_list[i] = (char*)malloc((std::strlen(name)+1)*sizeof(char));
        std::strcpy(block_list[i], name);
        printf("You are now blocking %s.\n", block_list[i]);
        break;
      }
    }
  }

  void unblock(char* name)
  {
    for(int i=0; i<50; i++)
    {
      if(block_list[i] != NULL && std::strcmp(block_list[i], name) == 0)
      {
        printf("You are no longer blocking %s.\n", block_list[i]);
        free(block_list[i]);
        break;
      }
    }
  }

  int blocked(char* name)
  {
    //printf("At least we're cheacking to see if %s is being blocked\n", name);
    for(int i=0; i<50; i++)
    {
      if(block_list[i] != NULL)
      {
        if(std::strcmp(block_list[i], name) == 0)
        {
          //printf("blocked a message from %s.\n", name);
          return 1;
        }
      }
    }
    return 0;
  }  

private:
  //This connects the client with the server and then attemps to
  //Start reading messages from the chatroom
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    asio::async_connect(socket_, endpoints,
        [this](std::error_code ec, tcp::endpoint)
        {
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
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header() && read_msg_.decode_command() == 0)
          {
            //printf("...%s...\n",read_msg_.decode_username());
            if(blocked(read_msg_.decode_username()) == 1)
            {
              //usleep(1000);
              do_read_body();
            }
            else
            {
              printf("%s: ",read_msg_.decode_username());
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
            if(std::strcmp(chatroom_name, read_msg_.decode_chatname_new()) != 0)
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
        [this](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && !blocked(read_msg_.decode_username()))
          {
            std::cout.write(read_msg_.body(), read_msg_.body_length());
            std::cout << "\n";
            do_read_header();
          }
          else if(!ec)
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
        [this](std::error_code ec, std::size_t /*length*/)
        {
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
  asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  char chatroom_name[20];
  char* block_list[50];
};

int main(int argc, char* argv[])
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

    std::thread t([&io_context](){ io_context.run(); });
    //std::cin.getline evaluates to true in a boolean context as long as there's no error
    //(bad bit and fail bit aren't set to true) so this loops forever getting messages
    //from std::cin
    char line[chat_message::max_body_length + 1];
    int chat_room_number = 0;
    char chatroom_name[20] = "Lobby";
    string return_str="";
    char username[11] = {'\0'};
    {
      View ncurses;
      return_str=ncurses.getUsername();
    }
    return_str.copy(username,return_str.size()+1);
    username[return_str.size()]='\0';

    
    //std::cout << "Enter a username " << std::endl;
    //std::cin.getline(username, 11);
    
    while (std::cin.getline(line, chat_message::max_body_length + 1))
    {
      //Create a message that's a command to change rooms and send it to the server
      //strcpy(chatroom_name, c.get_server_response());
     
      if(line[0] == '/' && line[1] == 'e')
      {
         //c.change_room(1);
         char* token = strtok(line, " ");
         token = strtok(NULL, " ");
         chat_message msg;
	 //msg.set_crn(chat_room_number);
         msg.set_chatname_current(chatroom_name);
         strcpy(chatroom_name, token);
         //printf("token: ...%s...%s...\n", token, chatroom_name);
         msg.set_chatname_new(chatroom_name);
      	 //msg.set_nrn(chat_room_number);
         msg.set_cmd(1);
   	 msg.encode_header();
         c.set_chatname(chatroom_name);
         //printf("...%s...\n", msg.data());
	 c.write(msg);
         //strcpy(chatroom_name, c.get_server_response());
      }
      else if(line[0] == '/' && line[1] == 'c')
      {
         char* token = strtok(line, " ");
         token = strtok(NULL, " ");
         //printf("chatroom name: %s\n", token);
         chat_message msg;
         msg.body_length(0);
         msg.set_chatname_current(chatroom_name);
         strcpy(chatroom_name, token);
         msg.set_chatname_new(chatroom_name);
      	 //msg.set_crn(chat_room_number);
         msg.set_cmd(2);
   	 msg.encode_header();
         c.set_chatname(chatroom_name);
         //printf("This is the full message data: %s\n", msg.data());
	 c.write(msg);
         //strcpy(chatroom_name, c.get_server_response());
      }
      else if(line[0] == '/' && line[1] == 'd')
      {
         char* token = strtok(line, " ");
         token = strtok(NULL, " ");
         char input[20];
         std::strcpy(input, token);
         //printf("chatroom name: %s\n", token);
         chat_message msg;
         msg.body_length(0);
         msg.set_chatname_current(chatroom_name);
         msg.set_chatname_new(input);
      	 //msg.set_crn(chat_room_number);
         msg.set_cmd(3);
   	 msg.encode_header();
         c.set_chatname(chatroom_name);
         //printf("This is the full message data: %s\n", msg.data());
	 c.write(msg);
         //strcpy(chatroom_name, c.get_server_response());
      }
      else if(line[0] == '/' && line[1] == 'b')
      {
         char* token = strtok(line, " ");
         token = strtok(NULL, " ");
         char input[std::strlen(token)+1];
         std::strcpy(input, token);
         //printf("blocking: %s.\n", input);
         c.block(input);
      }
      else if(line[0] == '/' && line[1] == 'u')
      {
         char* token = strtok(line, " ");
         token = strtok(NULL, " ");
         char input[20];
         std::strcpy(input, token);
         //printf("unblocking: %s.\n", input);
         c.unblock(input);
      }
      else if(line[0] == '/' && line[1] == 'h')
      {
         printf("This is the help menu, here are the avaliable commands:\n");
         printf("%-25s(creates a room called <room name>)\n", "/c <room name>");
         printf("%-25s(enters a room called <room name>)\n", "/e <room name>");
         printf("%-25s(deletes a room called <room name>)\n", "/d <room name>");
         printf("%-25s(blocks the user called <user name>)\n", "/b <user name>");
         printf("%-25s(unblocks the user called <user name>)\n", "/u <user name>");
         printf("%-25s(displays this help menu)\n", "/h");
      }
      else
      {
        chat_message msg;
        msg.set_username(username);
        msg.set_cmd(0);
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(chatroom_name);
        //msg.set_crn(chat_room_number);
        msg.body_length(std::strlen(line));
        std::memcpy(msg.body(), line, msg.body_length());
        msg.encode_header(); //neat idea
        c.write(msg);
      }
       
      usleep(500); //wait half a second for the server to process a message sent
      if(strcmp(chatroom_name, c.get_server_response()) != 0)
      {
        strcpy(chatroom_name, c.get_server_response());
        chat_message msg;
	//msg.set_crn(chat_room_number);
        msg.set_chatname_current(chatroom_name);
        msg.set_chatname_new(chatroom_name);
      	//msg.set_nrn(chat_room_number);
        msg.set_cmd(1);
   	msg.encode_header();
        //c.set_chatname(chatroom_name);
	c.write(msg);
      }

    }

    c.close();
    t.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
