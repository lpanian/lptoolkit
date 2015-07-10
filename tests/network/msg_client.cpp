#include <memory>
#include <list>
#include <iostream>
#include "toolkit/network.hh"
#include "testmsg.hh"

class ClientProcessor : public lptk::MessageProcessor
{
public:
    // using lptk::MessageProcessor::MessageProcessor; // not supported in vs2013
    ClientProcessor(lptk::Socket&& socket,
        uint32_t maxMessageSize = 1024,
        uint32_t incomingBufferSize = (1 << 12),
        lptk::MemPoolId poolId = lptk::MEMPOOL_Network)
        : MessageProcessor(std::move(socket), maxMessageSize, incomingBufferSize, poolId)
    { }
protected:
    void HandleMessage(uint32_t typeId, const void* data, uint32_t dataSize) override {
        switch(typeId)
        {
        case MSG_Echo:
            {
                auto msg = Cast<EchoMessage>(data, dataSize);
                if(!msg) break;

                char maxBuf[256];
                strncpy(maxBuf, msg->m_buf, sizeof(maxBuf));
                maxBuf[255] = '\0';
                std::cout << "Client Echo From Server: " << maxBuf << std::endl;
            }
            break;
        default: 
            std::cerr << "Unrecognized message type " << typeId << std::endl;
            break;
        }
    }
};


int main(int argc, char **argv)
{
    lptk::NetworkInit();

    if(argc != 3) {
        std::cerr << "Usage: " <<
            argv[0] << " host port" << std::endl;
        return 1;
    }

    lptk::Socket sock = lptk::ClientConnect(argv[1], argv[2], lptk::SOCKETF_Stream); 
    if(!sock.Valid()) {
        std::cerr << " error connecting to " << argv[1] << ':' << argv[2] << std::endl;
        return 1;
    }
        
    std::cout << "connected" << std::endl;

    bool done = false;
    ClientProcessor proc(std::move(sock)); 

    EchoMessage hello;
    strcpy(hello.m_buf, "Hello, world!");
    proc.SendMessage(MSG_Echo, &hello, sizeof(hello));

    while(!done) {
        auto status = proc.Update();
        if(status < 0) {
            if(status == lptk::MessageProcessor::UPDATE_Error) {
                std::cout << "update error" << std::endl;
            } else if (status == lptk::MessageProcessor::UPDATE_Closed) {
                std::cout << "server closed connection" << std::endl;
            }
            done = true;
        }
        proc.Process();
        
        if(!proc.Valid()) {
            done = true;
        }
    }

    return 0;
}
