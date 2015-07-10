#include <iostream>
#include <cstring>
#include "toolkit/network.hh"

int main(int argc, char **argv)
{
    lptk::NetworkInit();
    if(argc != 3) {
        std::cerr << "Usage: " <<
            argv[0] << " host port" << std::endl;
        return 1;
    }

    lptk::Socket clientCx = lptk::ClientConnect(argv[1], argv[2], lptk::SOCKETF_Stream);
    if(!clientCx.Valid())
    {
        std::cerr << " error connecting to " << argv[1] << ':' << argv[2] << std::endl;
        return 1;
    }
        
    std::cout << "connected" << std::endl;

    const char* payload = "Hello, world!";
    int64_t payloadSize = strlen(payload) + 1;
    auto numWritten = clientCx.Write(payload, static_cast<size_t>(payloadSize));
    while(payloadSize > 0 && numWritten >= 0) {
        payloadSize -= numWritten;
        payload += numWritten;
        if(payloadSize > 0) 
            numWritten = clientCx.Write(payload, static_cast<size_t>(payloadSize));
    }

    if(numWritten < 0) {
        std::cerr << " network write error" << std::endl;
        return 1;
    }
    std::cout << "payload sent, waiting for response" << std::endl;

    char responseBuffer[3];
    int64_t responseSize = sizeof(responseBuffer);

    char* writeBuf = responseBuffer;
    auto numRead = clientCx.Read(writeBuf, static_cast<size_t>(responseSize));
    while(numRead > 0 && responseSize > 0) {
        responseSize -= numRead;
        writeBuf += numRead;
        if(responseSize > 0) 
            numRead = clientCx.Read(writeBuf, static_cast<size_t>(responseSize));
    }

    responseBuffer[2] = '\0';

    std::cout << "Received response: \"" << responseBuffer << "\", exiting." << std::endl;
    return 0;
}

