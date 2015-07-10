#include <memory>
#include <list>
#include <iostream>
#include "toolkit/network.hh"
#include "testmsg.hh"

class ServerProcessor : public lptk::MessageProcessor
{
public:
    ServerProcessor(lptk::Socket&& socket,
        uint32_t maxMessageSize = 1024,
        uint32_t incomingBufferSize = (1 << 12),
        lptk::MemPoolId poolId = lptk::MEMPOOL_Network)
        : MessageProcessor(std::move(socket), maxMessageSize, incomingBufferSize, poolId)
    {}
    //using lptk::MessageProcessor::MessageProcessor; // not supported in vs2013
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
                std::cout << "Server Echo: " << maxBuf << std::endl;

                EchoMessage reply;
                strcpy(reply.m_buf, "OK");
                SendMessage(MSG_Echo, &reply, sizeof(reply));
            }
            break;
        default: 
            std::cerr << "Unrecognized message type " << typeId << std::endl;
            break;
        }
    }
};

int main(int argc, char** argv)
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
    std::list<std::unique_ptr<lptk::MessageProcessor>> processors;
    while(!done) {
        auto iter = processors.begin();
        while(iter != processors.end()) {
            auto cur = iter;
            ++iter;

            if(!(*cur)->Valid()) {
                std::cout << "removing client" << std::endl;
                processors.erase(cur);
            }
        }
        
        lptk::Socket socket = serverCx.Accept();
        if(socket.Valid()) {
            std::cout << "New client accepted" << std::endl;
            processors.push_back(make_unique<ServerProcessor>(std::move(socket)));
        }

        for(auto& proc : processors) {
            auto status = proc->Update();
            if(status < 0) {
                if(status == lptk::MessageProcessor::UPDATE_Error) {
                    std::cout << "update error" << std::endl;
                    proc->Close();
                } else if (status == lptk::MessageProcessor::UPDATE_Closed) {
                    std::cout << "server closed connection" << std::endl;
                }
            } 
        }
        
        for(auto& proc : processors) {
            proc->Process();
        }
    }
    return 0;
}

