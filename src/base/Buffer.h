#ifndef BUFFER_H
#define BUFFER_H

#include <vector>
#include <assert.h>
#include <stddef.h>
#include <string>
#include <cstring>
#include <sys/uio.h>
#include <unistd.h>

class Buffer {
public:
    Buffer(int initSize = 1024);
    ~Buffer() = default;

    size_t writeableBytes() const;
    size_t readableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;
    void ensureWriteable(size_t len);
    void hasWritten(size_t len);

    void retrieve(size_t len);
    void retrieveUtil(const char* end);
    void retrieveAll();
    std::string retrieveAllToStr();

    char* beginWrite();
    const char* beginWrite() const;

    void append(const std::string& str);
    void append(const char* data, size_t len);
    void append(const void* data, size_t len);
    void append(const Buffer& buff);

    ssize_t readFd(int fd, int* errNo);
    ssize_t writeFd(int fd, int* errNo);

private:
    char* begin();
    const char* begin() const;
    void makeSpace(size_t len);

    
    std::vector<char> buffer_;
    size_t readPos_;
    size_t writePos_;
};


#endif