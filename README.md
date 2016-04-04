# DialUp
Laser communication at dial up speeds - Accepting customers now!

## Setup
In a meteor-client-x folder edit `chat.js` on line 17 to the serialport that the Arduino is connected to.

To start the first client, run `meteor` on the command line. The webpage will be served at `http://localhost:3000/`.

To run a second client on the same machine a port has to be chosen when starting meteor `meteor --port <port>`, which will then be served at `http://localhost:<port>/`.
