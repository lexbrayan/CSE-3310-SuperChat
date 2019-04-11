//
// chat_server.cpp
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
#include <list>
#include <memory>
#include <set>
#include <utility>
#include "asio.hpp"
#include "chat_message.hpp"

using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
  void join(chat_participant_ptr participant, int chat_room_number)
  {
    participants_[chat_room_number].insert(participant);
    for (auto msg: recent_msgs_[chat_room_number])
    {
      participant->deliver(msg);
      printf("Delivering msg:\n%s\nto chatroom number %d\n", msg.body(), chat_room_number);
    }
  }

  void leave(chat_participant_ptr participant, int chat_room_number)
  {
    participants_[chat_room_number].erase(participant);
  }

  void change_room(chat_participant_ptr participant, int old_room_number, int new_room_number)
  {
    participants_[old_room_number].erase(participant);
    printf("Successfully changed rooms!\n");
    join(participant, new_room_number);
  }


  //If the message has an error in it there's no way of knowing which chatroom to leave
  //So we attempt to leave all of them
  void leave_all(chat_participant_ptr participant)
  {
    for(int i=0; i<10; i++)
    {
      participants_[i].erase(participant);
    }
  }

  void deliver(const chat_message& msg, int chat_room_number)
  {
    recent_msgs_[chat_room_number].push_back(msg);
    while (recent_msgs_[chat_room_number].size() > max_recent_msgs)
      recent_msgs_[chat_room_number].pop_front();

    for (auto participant: participants_[chat_room_number])
    {
      participant->deliver(msg);
      printf("Delivering msg:\n%s\nto chatroom number %d\n", msg.body(), chat_room_number);
    }
  }

private:
  std::set<chat_participant_ptr> participants_[10];
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_[10];
};

//----------------------------------------------------------------------

class chat_session
  : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
public:
  chat_session(tcp::socket socket, chat_room& room)
    : socket_(std::move(socket)), room_(room)
  {
  }
  //Enter the chatroom (in essence)
  void start()
  {
    room_.join(shared_from_this(), 0);
    do_read_header();
  }

  //Change room
  /*void change_room(int from, int to)
  {
    room_.leave(shared_from_this(), from);
    room_.join(shared_from_this(), to);
    do_read_header();
  }*/

  void deliver(const chat_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
      do_write();
    }
  }
//
//For the async_read and async_write, they are simply
//
//
//
//
private:
  void do_read_header()
  {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header() && read_msg_.decode_command() == 0)
          {
            do_read_body();
          }
	  else if(!ec && read_msg_.decode_header() && read_msg_.decode_command() == 1)
	  {
            printf("Attempting to changed rooms from %d to %d...\n", read_msg_.decode_crn(), read_msg_.decode_nrn());
	    room_.change_room(shared_from_this(), read_msg_.decode_crn(), read_msg_.decode_nrn()); 
            printf("reading header in new room.\n");
            do_read_header();
            /* std::atoi(read_msg_.body())*/
	    //printf("******%d*******\n", std::atoi(read_msg_.body())); 
	  }
          else
          {
            room_.leave_all(shared_from_this());
          }
        });
  }

  void do_read_body()
  {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            room_.deliver(read_msg_, read_msg_.decode_crn());
            do_read_header();
          }
          else
          {
            room_.leave_all(shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
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
            room_.leave_all(shared_from_this());
          }
        });
  }

  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
  chat_participant_ptr participant;
};

//----------------------------------------------------------------------

class chat_server
{
public:
  chat_server(asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            //This piece of code seems important...
	    //This is how a user is added to a chatroom in the server
	    std::make_shared<chat_session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  chat_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    asio::io_context io_context;

    std::list<chat_server> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
