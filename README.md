# CSE-3310-SuperChat Group 7 <br />
# Alex Eseyin, David Miller, Henry Le, Nihar Gupte <br />

Superchat created for CSE 3310 - Spring 2019 <br />

Uses basic ASIO networking protocol and ncurses for GUI <br />
<br />
to compile: <br />
make <br />
<br />
to run: <br />
./chat_server 9000 in one command window <br />
./chat_client 127.0.0.1 9000 in separate command window <br />
<br />
Help menu for superchat: <br />
the avaliable commands: <br />
        /c <room name> creates a room called <room name> <br />
        /e <room name> enters a room called <room name> <br />
        /d <room name> deletes a room called <room name> <br />
        /b <user name> blocks the user called <user name> <br />
        /u <user name> unblocks the user called <user name> <br />
        /h displays this help menu <br />
        /q exits the program <br />
        to enter lobby type /e Lobby <br />
        Chatrooms can only be deleted if there are no active users in it <br />
<br />
