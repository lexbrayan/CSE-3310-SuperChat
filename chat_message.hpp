//
// chat_message.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ncurses.h"
/*
 * The way a message works is it has a pointer to
 * the beginning of the data the message contains
 * as well as the length of the actual message and the
 * length of the header which currently only has the size
 * of the message
 *
 * for example:
 * if the message is "Hello World!"
 * 
 * the message data will be: 
 * 12Hello World!
 *
 * where 12 is the number of bytes the message contains
 *
 * This header is what we will use to store data about the message
 * like the user who sent the message and the chatroom it should be 
 * sent to, and any other information directly associated with a message
 *
 *
 */
 
/*
 * The header of the message will contain the size of the message (4 bytes)
 * 					  the chatroom number (4 bytes)
 *                                        the user who sent the message (20 bytes)
 *
 *
 *
 */
class chat_message
{
public:
  enum { header_length = 72 };
  enum { max_body_length = 512 };

  chat_message()
  {
    body_length_ = 0;
    chat_room_number = 0;
    new_room_number = 0;
    command = 0;
    //chatname_old;
  //  chatname_new;
//    username;

  }

  const char* data() const
  {
    return data_;
  }

  char* data()
  {
    return data_;
  }

  std::size_t length() const
  {
    return header_length + body_length_;
  }

  const char* body() const
  {
    return data_ + header_length;
  }

  char* body()
  {
    return data_ + header_length;
  }

  std::size_t body_length() const
  {
    return body_length_;
  }

  void body_length(std::size_t new_length)
  {
    body_length_ = new_length;
    if (body_length_ > max_body_length)
      body_length_ = max_body_length;
  }

  void set_crn(int crn)
  {
    chat_room_number = crn;
  }

  void set_nrn(int nrn)
  {
    new_room_number = nrn;
  }

  void set_cmd(int cmd)
  {
    command = cmd;
  }

  void set_username(char* un)
  {
    std::strncpy(username, un, 16);
  }

  void set_chatname_current(char* cn)
  {
    std::strncpy(chatname_old, cn, 20);
  }

  void set_chatname_new(char* cn)
  {
    std::strncpy(chatname_new, cn, 20);
  }

  //Gets the chatroom number of a message
  int decode_crn()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    header[8] = '\0';
    int crn_offset = 4;
    int crn = std::atoi(header + crn_offset);
    
    return crn;
  }
  
  int decode_command()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    //So it only reads the body size part of the header
    header[12] = '\0';
    return std::atoi(header+8);

  }

  bool decode_header()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    //So it only reads the body size part of the header
    header[4] = '\0';

    body_length_ = std::atoi(header);
    if (body_length_ > max_body_length)
    {
      body_length_ = 0;
      return false;
    }
    return true;
  }

  int decode_nrn()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    //So it only reads the body size part of the header
    header[16] = '\0';
    return std::atoi(header+12);
  }

  char* decode_username()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    for(int i=16; i<32; i++)
    {
      if(header[i] == ' ')
      {
        header[i] = '\0';
      }
    }
    //So it only reads the body size part of the header
    header[32] = '\0';
    return (header+16);
  }

  char* decode_chatname_old()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    //So it only reads the body size part of the header
    for(int i=32; i<52; i++)
    {
      if(header[i] == ' ')
      {
        header[i] = '\0';
      }
    }
    header[52] = '\0';
    return (header+32);
  }

  char* decode_chatname_new()
  {
    char header[header_length + 1] = "";
    std::strncat(header, data_, header_length);
    //So it only reads the body size part of the header
    for(int i=52; i<72; i++)
    {
      if(header[i] == ' ')
      {
        header[i] = '\0';
      }
    }
    header[72] = '\0';
    return (header+52);
  }

  void encode_header()
  {
    char header[header_length + 1] = "";
    std::sprintf(header, "%4d%4d%4d%4d%-16s%-20s%-20s", static_cast<int>(body_length_), chat_room_number, command, new_room_number, username, chatname_old, chatname_new);
    std::memcpy(data_, header, header_length);
  }

private:
  int chat_room_number;
  int new_room_number;
  int command;
  char username[17];
  char chatname_old[21];
  char chatname_new[21];
  char data_[header_length + max_body_length];
  std::size_t body_length_;

};

#endif // CHAT_MESSAGE_HPP
