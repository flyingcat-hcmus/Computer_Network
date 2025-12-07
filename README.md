# CP

COMPILE = g++ -std=c++17 -I./ -I./asio/include server.cpp -o server.exe -lmswsock  -lws2_32 -lgdi32 -luser32
