# Sokket
A program that transfer information between a client and a server. Sokket can transmit unencrypted files and messages by TCP. It currently only works on Windows, but a Unix port is coming soon.
# How to build
````
$ git clone https://github.com/umloucobr/sokket
$ mkdir build
$ cd build
$ cmake ../cmake-example
$ make
````
# How to open
Open the server, then the client. They are both capable of sending and receiving information.
````
$ sokket [mode] [port] [address]
````

There are 3 modes:
- -a: Automatic Mode. It opens the program as a client, and if no server is found it automatically switches to a server.
- -c: Client mode.
- -s: Server mode.

The default values are:

 - sokket::config::port = 27015.
 - sokket::config::address = localhost.

You can also open the file without arguments, entering Automatic Mode by default.
# How to use
When connected you will see a ">" symbol, meaning that you are able to send and receive information. It defaults to message mode, so you can type anything and hit enter. When receiving a message, the size in bytes will appear by it's side.
To send files, use:
````
> :f [name of the file for the receiver] [path of file to send] 
```` 
Remember to add the correct file extension for both of those.
If you receive a file the message "File written to path" will appear.
# Other information
The program first sends the total size of the information being sent, and in the case of files, it's name.
If the information is too big the program will divide it to fit in the maximum value of the socket, which defaults to 1400. It can be modified by changing:
````
extern const int sokket::config::bufferSize{};
````
This will require recompiling the program.
 