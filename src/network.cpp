#include "toolkit/network.hh"
#include <cstdio>
#include <cstring>

#include "toolkit/mathcommon.hh"

#ifdef LINUX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#endif

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

namespace lptk
{
#if defined(LINUX)
    static const Socket::SocketType kBadSocket = -1;
#elif defined(WINDOWS)
    static const Socket::SocketType kBadSocket = INVALID_SOCKET;
#endif

    void NetworkInit()
    {
#if defined(WINDOWS)
        WSADATA wsaData;
        int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (err != 0)
        {
            fprintf(stderr, "Failed to initialize WSA\n");
            return;
        }
        else
        {
            fprintf(stderr, "WSA init: %s\n", wsaData.szDescription);
        }
#endif
    }

    ////////////////////////////////////////////////////////////////////////////////
    Socket::Socket()
        : m_socket(kBadSocket)
    {
    }
        
    Socket::Socket(SocketType s)
        : m_socket(s)
    {
    }

    Socket::Socket(int family, int type, int protocol)
        : m_socket(socket(family, type, protocol))
    {
//#if defined(LINUX)
//        int const flags = fcntl(m_socket, F_GETFL, 0);
//        fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);
//#elif defined(WINDOWS)
//        u_long enable = 1;
//        ioctlsocket(m_socket, FIONBIO, &enable);
//#endif
    }
    
    Socket::Socket(Socket &&other)
        : m_socket(other.m_socket)
    {
        other.m_socket = kBadSocket;
    }
        
    Socket& Socket::operator=(Socket&& other)
    {
        Close();
        m_socket = other.m_socket;
        other.m_socket = kBadSocket;
        return *this;
    }

    Socket::~Socket() 
    {
        Close();
    }

    bool Socket::Valid() const
    {
        return m_socket != kBadSocket;
    }
        
    int64_t Socket::Write(const void* buf, size_t count) const
    {
        #if defined(LINUX)
            return send(m_socket, buf, count, 0);
        #elif defined(WINDOWS)
            return int64_t(send(m_socket, reinterpret_cast<const char*>(buf), 
                int(count), 0));
        #endif
    }

    int64_t Socket::Read(void* buf, size_t count) const
    {
        #if defined(LINUX)
            return recv(m_socket, buf, count, MSG_DONTWAIT);
        #elif defined(WINDOWS)
            return int64_t(recv(m_socket, reinterpret_cast<char*>(buf), 
                int(count), 0));
        #endif
    }

    void Socket::Close() 
    {
        if(Valid()) {
            shutdown(m_socket, 
#if defined(LINUX)
                SHUT_WR
#elif defined(WINDOWS)
                SD_BOTH
#endif
                );

#if defined(LINUX)
            close(m_socket);
#elif defined(WINDOWS)
            closesocket(m_socket);
#endif
            m_socket = kBadSocket;
        }
    }

    void PrintLastNetworkError(const char* prm)
    {
#if defined(LINUX)
        const char* err = strerror(errno);
#elif defined(WINDOWS)
        char msgBuf[1024];
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            WSAGetLastError(),
            0,
            msgBuf,
            ARRAY_SIZE(msgBuf),
            NULL);
        const char* err = msgBuf;
#endif
        fprintf(stderr, "network error (%s): %s\n", prm ? prm : "", err);
    }

    ////////////////////////////////////////////////////////////////////////////////
    template<class Fn>
    bool GetAddrInfo(const char* hostname, const char* service, uint32_t flags, 
        Fn&& onFindHost)
    {
        addrinfo hints ;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = (flags & SOCKETF_Datagram) ? 
            SOCK_DGRAM : SOCK_STREAM;
        hints.ai_flags = hostname == nullptr ? AI_PASSIVE : 0;
        
        addrinfo *result = nullptr;
        int status = getaddrinfo(hostname, service, &hints, &result);
        if(status) {
            fprintf(stderr, "network: getaddrinfo error: %s\n", gai_strerror(status));
            return false;
        }
        
        const auto freeOnExit = at_scope_exit([&]{ freeaddrinfo(result); });
        
        addrinfo* cur = nullptr;
        for(cur = result; cur; cur = cur->ai_next) {
            void* addr = nullptr;
            switch(cur->ai_family) 
            {
            case AF_INET:
                addr = &reinterpret_cast<sockaddr_in*>(cur->ai_addr)->sin_addr;
                break;
            case AF_INET6:
                addr = &reinterpret_cast<sockaddr_in6*>(cur->ai_addr)->sin6_addr;
                break;
            default:
                break;
            }

            if(addr) {
                char addrBuf[256];
                inet_ntop(cur->ai_family, addr, addrBuf, sizeof(addrBuf));
                addrBuf[255] = '\0';
                fprintf(stderr, "network: Trying %s...\n", addrBuf);
            }
      
            if(onFindHost(cur))
                return true;
        }
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    ServerConnection::ServerConnection(uint32_t flags)
        : m_flags(flags)
        , m_socket()
    {
    }

    bool ServerConnection::Listen(const char* service)
    {
        return GetAddrInfo("localhost", service, m_flags, [&](addrinfo* addrInfo) {
            m_socket = Socket(addrInfo->ai_family, 
                addrInfo->ai_socktype, 
                addrInfo->ai_protocol);
            
            if(!m_socket.Valid()) {
                fprintf(stderr, "network: Socket create failed.\n");
                return false;
            }

#if defined(LINUX)
            using OptType = int;
#elif defined(WINDOWS)
            using OptType = char;
#endif
            OptType yes = 1;
            if(setsockopt(m_socket.Raw(), SOL_SOCKET,
                SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
                PrintLastNetworkError("setsockopt");
                return false;
            }

            if (0 != (m_flags & SOCKETF_NonBlock))
            {
#if defined(LINUX)
                int const flags = fcntl(m_socket.Raw(), F_GETFL, 0);
                fcntl(m_socket.Raw(), F_SETFL, flags | O_NONBLOCK);
#elif defined(WINDOWS)
                u_long enable = 1;
                ioctlsocket(m_socket.Raw(), FIONBIO, &enable);
#endif
            }
            int status = bind(m_socket.Raw(),
                addrInfo->ai_addr,
                static_cast<int>(addrInfo->ai_addrlen));

            if(status == -1) {
                PrintLastNetworkError("bind");
                return false;
            } 

            status = listen(m_socket.Raw(), 10);
            if(status == -1) {
                PrintLastNetworkError("listen");
                return false;
            }
            
            return true; 
        });
    }
        
    Socket ServerConnection::Accept()
    {
#if defined(LINUX)
        using SockLenType = socklen_t;
#elif defined(WINDOWS)
        using SockLenType = int;
#endif
        sockaddr_storage theirAddr;
        SockLenType addrSize = sizeof(theirAddr);
        auto const fd = accept(m_socket.Raw(), 
            reinterpret_cast<sockaddr*>(&theirAddr),
            &addrSize);

        if(fd == kBadSocket) {
            return Socket();
        }

        return Socket(fd);
    }

    void ServerConnection::Close()
    {
        m_socket.Close();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    Socket ClientConnect(const char* hostname, const char* service, uint32_t flags)
    {
        Socket result;
        bool connected = GetAddrInfo(hostname, service, flags, [&](addrinfo* addrInfo) {
            result = Socket(addrInfo->ai_family, 
                addrInfo->ai_socktype, 
                addrInfo->ai_protocol);
            
            if(!result.Valid()) {
                fprintf(stderr, "network: Socket create failed.\n");
                return false;
            }

            const auto status = connect(result.Raw(), 
                addrInfo->ai_addr,
                static_cast<int>(addrInfo->ai_addrlen));

            if(status != 0) 
            {
                PrintLastNetworkError("connect");
                return false;
            }
            return true;
        });
        if(connected)
            return std::move(result);
        else
            return Socket();
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    CircularBuffer::CircularBuffer(uint32_t bufferSize, MemPoolId poolId)
        : m_buffer(bufferSize, poolId)
        , m_startPos(0)
        , m_endPos(0)
        , m_size(0)
    {
    }

    bool CircularBuffer::Write(const void* data, uint32_t len)
    {
        if(RemainingSize() < len) 
            return false;
       
        const auto origLen = len;
        const auto bufSize = static_cast<uint32_t>(m_buffer.size());
        const char* dataCopy = reinterpret_cast<const char*>(data);

        auto endPos = m_endPos;
        if(m_startPos <= endPos) 
        {
            const uint32_t copyLen = Min<uint32_t>(len, bufSize - endPos);
            memcpy(&m_buffer[endPos], dataCopy, copyLen);
            len -= copyLen;
            endPos += copyLen;
            dataCopy += copyLen;
        }

        if(endPos == bufSize) 
            endPos = 0;

        // we may have to copy more if we wrapped around
        if(len && endPos < m_startPos) 
        {
            const uint32_t copyLen = Min<uint32_t>(m_startPos - endPos, len);
            memcpy(&m_buffer[endPos], dataCopy, len);
            endPos += copyLen;
        }

        m_size += origLen;
        m_endPos = endPos;
        return true;
    }

    bool CircularBuffer::Read(void* data, uint32_t len, bool advance)
    {
        if(Size() < len)
            return false;

        const auto origLen = len;
        const auto bufSize = static_cast<uint32_t>(m_buffer.size());
        char* destBuffer = reinterpret_cast<char*>(data);

        auto startPos = m_startPos;
        if(startPos >= m_endPos)
        {
            const auto copyLen = Min<uint32_t>(len, bufSize - startPos);
            memcpy(destBuffer, &m_buffer[startPos], copyLen);
            len -= copyLen;
            startPos += copyLen;
            destBuffer += copyLen;
        }

        if(startPos == bufSize)
            startPos = 0;

        if(len && startPos < m_endPos)
        {
            const uint32_t copyLen = Min<uint32_t>(len, m_endPos - startPos);
            memcpy(destBuffer, &m_buffer[startPos], copyLen);
            startPos += copyLen;
        }

        if(advance) 
        {
            m_size -= origLen;
            m_startPos = startPos;
        }
        return true;
    }

    const void* CircularBuffer::Peek(uint32_t len)
    {
        const auto bufSize = m_buffer.size();
        if(m_size) 
        {
            const auto end = (m_startPos < m_endPos) ? 
                m_endPos : bufSize;
            if(len <= (end - m_startPos)) 
                return &m_buffer[m_startPos];
            else
                return nullptr;
        } else {
            return nullptr;
        }
        
    }

    bool CircularBuffer::Skip(uint32_t len)
    {
        if(Size() < len)
            return false;
        
        m_size -= len;

        const auto bufSize = static_cast<uint32_t>(m_buffer.size());

        auto startPos = m_startPos;
        if(startPos >= m_endPos)
        {
            const auto copyLen = Min<uint32_t>(len, bufSize - startPos);
            len -= copyLen;
            startPos += copyLen;
        }

        if(startPos == bufSize)
            startPos = 0;

        if(len && startPos < m_endPos)
        {
            const auto copyLen = Min<uint32_t>(len, m_endPos - startPos);
            startPos += copyLen;
        }

        m_startPos = startPos;
        return true;
        
    }

    uint32_t CircularBuffer::Size() const
    {
        return m_size;
    }

    uint32_t CircularBuffer::RemainingSize() const
    {   
        const auto bufSize = static_cast<uint32_t>(m_buffer.size());
        return bufSize - m_size;
    }


    
    ////////////////////////////////////////////////////////////////////////////////
    namespace {
        struct MessageHeader 
        {
            uint16_t m_typeId;
            uint16_t m_size;
        };
    
        static_assert(sizeof(MessageHeader) == 4, "unexpected MessageHeader found");
    }

    MessageProcessor::MessageProcessor(Socket&& socket, 
        uint32_t maxMessageSize,
        uint32_t incomingBufferSize,
        MemPoolId poolId)
        : m_socket(std::move(socket))
        , m_messageBuffer(Max(maxMessageSize, uint32_t(sizeof(MessageHeader))), poolId)
        , m_buffer(Max(maxMessageSize, incomingBufferSize), poolId)
    {
    }
        
    MessageProcessor::~MessageProcessor()
    {
    }

    bool MessageProcessor::Valid() const 
    {
        return m_socket.Valid();
    }
        
    MessageProcessor::UpdateStatusType MessageProcessor::Update()
    {
        if(!m_socket.Valid())
            return UPDATE_Closed;

        int64_t bytesRead = -1;
        do {
            char readBuffer[1024];
            const size_t amountToRead = Min<uint32_t>(sizeof(readBuffer), 
                    m_buffer.RemainingSize());

            if(amountToRead == 0) 
                return UPDATE_Full;
        
            bytesRead = m_socket.Read(readBuffer, amountToRead);
            if(bytesRead > 0) 
            {
                m_buffer.Write(readBuffer, uint32_t(bytesRead));
            } 
            else if (bytesRead == -1)
            {
                if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    return UPDATE_NoData;
                }
                else 
                {
                    PrintLastNetworkError("socket read");
                    return UPDATE_Error;
                }
            }
        } while (bytesRead > 0);

        if(bytesRead == 0) {
            m_socket = Socket();
            fprintf(stderr, "network: remote closed connection (clean)\n");
            return UPDATE_Closed;
        } 
        return UPDATE_Success;
    }

    int MessageProcessor::Process()
    {
        int numProcessed = 0;
        while(m_buffer.Size() >= sizeof(MessageHeader)) 
        {
            MessageHeader header;
            if(!m_buffer.Read(&header, sizeof(header), false)) {
                break;
            }

            if(m_buffer.Size() >= (header.m_size + sizeof(header))) 
            {
                // if there's not enough data to read this message, panic and error out
                if(header.m_size + sizeof(header) > m_messageBuffer.size())
                {
                    // TODO resize up to max limit
                    fprintf(stderr, "network: Cannot read message of type %u with payload size %u. "
                        " It is too big, the maximum message size is %u\n",
                        header.m_typeId,
                        uint32_t(sizeof(header) + header.m_size),
                        uint32_t(m_messageBuffer.size()));
                    m_buffer.Skip(sizeof(header) + header.m_size);
                    continue;
                }

                m_buffer.Skip(sizeof(header));
                const void* data = nullptr;
                bool dataRead = false;

                // attempt to point directly at the payload
                if(header.m_size > 0)
                {
                    data = m_buffer.Peek(header.m_size);
                    if(!data) 
                    {
                        const bool readSuccessful = m_buffer.Read(&m_messageBuffer[0], 
                            Min<uint32_t>(static_cast<uint32_t>(m_messageBuffer.size()), header.m_size));
                        ASSERT(readSuccessful && "read failed - this should succeed "
                            "due to the code above.");
                        if(!readSuccessful)
                        {
                            fprintf(stderr, "network: fatal - Failed to read %u bytes from "
                            "circular buffer.\n", header.m_size);
                            break;
                        }

                        dataRead = true;
                        data = &m_messageBuffer[0];
                    }
                }

                HandleMessage(header.m_typeId, data, header.m_size);
                if(!dataRead)
                    m_buffer.Skip(header.m_size);
                ++numProcessed;
            }
            else
                break;
        }
        return numProcessed;
    }
        
    bool MessageProcessor::SendLoop(const void* buffer, uint32_t size)
    {
        if(!m_socket.Valid())
            return false;

        const char* byteBuffer = reinterpret_cast<const char*>(buffer);
        size_t bytesRemaining = int64_t(size);

        while(bytesRemaining > 0) 
        {
            int64_t bytesWritten = m_socket.Write(byteBuffer, bytesRemaining);
            if(bytesWritten <= 0) {
                m_socket = Socket();
                return false;
            } 

            bytesRemaining -= static_cast<size_t>(bytesWritten);
            byteBuffer += bytesWritten;
        }

        return true;
    }

        
    bool MessageProcessor::SendMessage(uint32_t typeId, const void* data, uint32_t dataSize)
    {
        if(!m_socket.Valid())
        {
            fprintf(stderr, "network: Failed to send message %u with %u data bytes: socket is invalid\n",
                typeId, dataSize);
            return false;
        }

        MessageHeader header;
        header.m_typeId = uint16_t(typeId);
        header.m_size = uint16_t(dataSize);

        if(!SendLoop(&header, sizeof(header))) 
        {
            fprintf(stderr, "network: Failed to send message %u with %u data bytes: header send failed\n",
                typeId, dataSize);
            return false;
        }

        if(!SendLoop(data, dataSize)) 
        {
            fprintf(stderr, "network: Failed to send message %u with %u data bytes: payload send failed\n",
                typeId, dataSize);
            return false;
        }

        return true;
    }

    void MessageProcessor::Close()
    {
        m_socket = Socket();
    }


}

