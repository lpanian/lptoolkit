#pragma once
#ifndef INCLUDED_toolkit_network_HH
#define INCLUDED_toolkit_network_HH

#include "toolkit/dynary.hh"

#ifdef WINDOWS
#include <Winsock2.h>
#endif

namespace lptk
{
    void NetworkInit();

    ////////////////////////////////////////////////////////////////////////////////
    enum SocketFlags : uint32_t {
        SOCKETF_Local = (1 << 0),                 // unix local socket
        SOCKETF_Stream = (1 << 1),                // TCP or similar
        SOCKETF_Datagram = (1 << 2),              // UDP or similar
        SOCKETF_NonBlock = (1 << 3),              // Don't block
    };

    class Socket
    {
    public:
        typedef
#if defined(LINUX)
            int
#elif defined(WINDOWS)
            SOCKET 
#else
#pragma error("No socket implementation for this platform")
#endif
        SocketType;
       
        Socket();
        Socket(SocketType s);
        Socket(int family, int type, int protocol);
        Socket(Socket &&other);
        Socket& operator=(Socket&& other);
        ~Socket();
    
        bool Valid() const;

        int64_t Write(const void* buf, size_t count) const;
        int64_t Read(void* buf, size_t count) const;
        void Close() ;

        inline SocketType Raw() const { return m_socket; }
    private:
        Socket(const Socket&) DELETED;
        Socket& operator=(const Socket&) DELETED;

        SocketType m_socket;
    };

    ////////////////////////////////////////////////////////////////////////////////
    Socket ClientConnect(const char* hostname, const char* service, 
        uint32_t flags = SOCKETF_Stream);

    ////////////////////////////////////////////////////////////////////////////////
    // manage sockets, close them, do stuff on callbacks
    class ServerConnection
    {
    public:
        ServerConnection(uint32_t flags);
        bool Listen(const char* service);
        Socket Accept();
        void Close();
    private:
        uint32_t m_flags;
        Socket m_socket;
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    // this is a circular buffer that tries to keep 
    class CircularBuffer
    {
    public:
        CircularBuffer(uint32_t bufferSize, MemPoolId poolId);

        bool Write(const void* data, uint32_t len);
        bool Read(void* data, uint32_t len, bool advance = true);
        const void* Peek(uint32_t len);
        bool Skip(uint32_t len);

        uint32_t Size() const;
        uint32_t RemainingSize() const;
    private:
        DynAry<char> m_buffer;
        uint32_t m_startPos;
        uint32_t m_endPos;
        uint32_t m_size;
    };

    ////////////////////////////////////////////////////////////////////////////////
    // message sending utility class
    class MessageProcessor
    {
    public:
        enum UpdateStatusType { 
            UPDATE_Closed = -2,
            UPDATE_Error = -1,
            UPDATE_Success = 0,
            UPDATE_Full = 1,
            UPDATE_NoData = 2,
        };
        MessageProcessor(Socket&& socket, 
            uint32_t maxMessageSize = 1024,
            uint32_t incomingBufferSize = (1 << 12), 
            MemPoolId poolId = MEMPOOL_Network);
        MessageProcessor(const MessageProcessor&) DELETED;
        MessageProcessor& operator=(const MessageProcessor&) DELETED;
        virtual ~MessageProcessor();

        virtual UpdateStatusType Update();
        virtual int Process();
        virtual bool SendMessage(uint32_t typeId, const void* data, uint32_t dataSize);

        void Close();
        bool Valid() const ;
    protected:
        virtual void HandleMessage(uint32_t typeId, const void* data, uint32_t dataSize) = 0;
    private:
        bool SendLoop(const void* buffer, uint32_t size);

        Socket m_socket;
        DynAry<char> m_messageBuffer;
        CircularBuffer m_buffer;
    };
}

#endif


