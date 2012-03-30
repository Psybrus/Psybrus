Running the STUN server:
------------------------

You need to have 2 IP addresses next to each other to run the STUN server.
./server -h primary-ip -a secondary-ip -b



STUN server version 0.97
Usage: 
 ./server [-v] [-h] [-h IP_Address] [-a IP_Address] [-p port] [-o port] [-m mediaport]
 
 If the IP addresses of your NIC are 10.0.1.150 and 10.0.1.151, run this program with
    ./server -v  -h 10.0.1.150 -a 10.0.1.151
 STUN servers need two IP addresses and two ports, these can be specified with:
  -h sets the primary IP
  -a sets the secondary IP
  -p sets the primary port and defaults to 3478
  -o sets the secondary port and defaults to 3479
  -b makes the program run in the backgroud
  -m sets up a STERN server starting at port m
  -v runs in verbose mode

