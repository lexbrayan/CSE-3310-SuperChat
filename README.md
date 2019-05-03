# CSE-3310-SuperChat Group 7
# Alex Eseyin, David Miller, Henry Le, Nihar Gupte

Superchat created for CSE 3310 - Spring 2019

Uses basic ASIO networking protocol and ncurses for GUI

to compile:
make

to run:
./chat_server 9000 in one command window
./chat_client 127.0.0.1 9000 in separate command window

Help menu for superchat:
the avaliable commands:
        /c <room name> creates a room called <room name>
        /e <room name> enters a room called <room name>
        /d <room name> deletes a room called <room name>
        /b <user name> blocks the user called <user name>
        /u <user name> unblocks the user called <user name>
        /h displays this help menu
        /q exits the program
