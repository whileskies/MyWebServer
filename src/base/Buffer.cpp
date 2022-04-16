#include "Buffer.h"

Buffer::Buffer(int initSize) 
            : buffer_(initSize), 
              readPos_(0), 
              writePos_(0) 
{
    assert(initSize > 0);
}

size_t Buffer::readableBytes() const
{
    return writePos_ - readPos_;
}

size_t Buffer::writeableBytes() const
{
    return buffer_.size() - writePos_;
}

size_t Buffer::prependableBytes() const 
{
    return readPos_;
}

const char* Buffer::peek() const
{
    return begin() + readPos_;
}

void Buffer::retrieve(size_t len)
{
    assert(len <= readableBytes());
    readPos_ += len;
}

void Buffer::retrieveUtil(const char* end)
{
    assert(peek() <= end);
    retrieve(end - peek());
}

void Buffer::retrieveAll()
{
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::retrieveAllToStr()
{
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
}

char* Buffer::beginWrite()
{
    return begin() + writePos_;
}

const char* Buffer::beginWrite() const
{
    return begin() + writePos_;
}

void Buffer::hasWritten(size_t len)
{
    writePos_ += len;
}

void Buffer::append(const char* data, size_t len)
{
    assert(data);
    ensureWriteable(len);
    std::copy(data, data + len, beginWrite());
    hasWritten(len);
}

void Buffer::append(const std::string& str)
{
    append(str.data(), str.length());
}

void Buffer::append(const void* data, size_t len)
{
    assert(data);
    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer& buff)
{
    append(buff.peek(), buff.readableBytes());
}

char* Buffer::begin()
{
    return &*buffer_.begin();
}

const char* Buffer::begin() const
{
    return &*buffer_.begin();
}

void Buffer::makeSpace(size_t len)
{
    if (writeableBytes() + prependableBytes() < len)
    {
        buffer_.resize(writePos_ + len);
    }
    else
    {
        size_t readable = readableBytes();
        std::copy(begin() + readPos_, begin() + writePos_, begin());
        readPos_ = 0;
        writePos_ = readPos_ + readable;
        assert(readable == readableBytes());
    }
}

void Buffer::ensureWriteable(size_t len)
{
    if (writeableBytes() < len)
    {
        makeSpace(len);
    }
    assert(writeableBytes() >= len);
}

ssize_t Buffer::readFd(int fd, int* errNo)
{
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = writeableBytes();

    iov[0].iov_base = begin() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0)
    {
        *errNo = errno;
    }
    else if (static_cast<size_t>(len) <= writable)
    {
        writePos_ += len;
    }
    else
    {
        writePos_ = buffer_.size();
        append(buff, len - writable);
    }

    return len;
}

ssize_t Buffer::writeFd(int fd, int* errNo)
{
    size_t readable = readableBytes();
    ssize_t len = write(fd, peek(), readable);
    if (len < 0)
    {
        *errNo = errno;
        return len;
    }
    readPos_ += len;
    return len;
}
