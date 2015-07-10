#include <iostream>
#include "toolkit/network.hh"

int main(int argc, char **argv)
{
    lptk::NetworkInit();
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " port" << std::endl;
        return 1;
    }

    lptk::ServerConnection serverCx(lptk::SOCKETF_Stream);
    if(!serverCx.Listen(argv[1])) {
        std::cerr << "error listening on port " << argv[1] << std::endl;
        return 1;
    }

    std::cout << "listening" << std::endl;

    bool done = false;
    while(!done) 
    {
        lptk::Socket socket = serverCx.Accept();
        if(socket.Valid()) {
            char buf[256] = {};
            socket.Read(buf, sizeof(buf) - 1);
            std::cout << "read: " << buf << std::endl;

            socket.Write("OK", 3);
            done = true;
        }
    }
    return 0;
}


