/*
 * Copyright (c) 2014-2019 WangBin <wbsecg1 at gmail.com>
 * Original code is from QtAV project
 */
#pragma once
#include <set>
#include <string>
#include <vector>
#include "global.h"
MDK_NS_BEGIN
class MDK_API MDK_NOVTBL MediaIO
{
public:
    enum class AccessMode : int8_t {
        Read, // default
        Write
    };
    static bool registerOnce(const char* name, std::function<MediaIO*()>&& creator);
    static const std::vector<std::string>& registered();
    static MediaIO* create(const char* name = "FFmpeg");
    //static std::vector<std::string> registered();
    /*!
     * \brief createForProtocol
     * \return Null if none of registered MediaIO supports the protocol/url
     */
    static MediaIO* createForProtocol(const std::string& protocol_or_url);
    /*!
     * \brief createForUrl
     * Create a MediaIO and setUrl(url) if protocol of url is supported. Will block current thread.
     * Example: MediaIO *aio = MediaIO::createForUrl("assets:/icon/test.mkv");
     * \return MediaIO instance with url set. Null if protocol is not supported.
     * TODO: async setUrl() (io = createForUrl(...), io->abort()) or no setUrl() but manually open()/close()? (can not fallback to
     * others if the returned io fails to open)
     */
    static MediaIO* createForUrl(const std::string& url, AccessMode mode = AccessMode::Read);

    virtual ~MediaIO();
    bool setUrl(const std::string& url);
    std::string url() const;
    /*!
     * \brief setAccessMode
     * A MediaIO instance can be 1 mode, Read (default) or Write. If !isWritable(), then set to Write will fail and mode does not change
     * Call it before any function!
     * \return false if set failed
     */
    bool setAccessMode(AccessMode value);
    AccessMode accessMode() const;

    virtual const char* name() const = 0;
    /// supported protocols. default is empty
    virtual const std::set<std::string>& protocols() const;
    virtual bool isSeekable() const = 0;
    virtual bool isWritable() const { return false;}
    /*!
     * \brief read
     * Read at most maxSize bytes to data, and return the bytes were actually read
     * It can be an async operation.
     * \return bytes actually read. 0 if reach EOF
     */
    virtual int64_t read(uint8_t *data, int64_t maxSize) = 0;
    /*!
     * \brief write
     * Write at most maxSize bytes from data, and return the bytes were actually written
     * It can be an async operation.
     */
    virtual int64_t write(const uint8_t* data, int64_t maxSize) { return 0; }
    /*!
     * \brief seek
     * \param from SEEK_SET, SEEK_CUR and SEEK_END from stdio.h. offset <= 0 for SEEK_END
     * \return true if success
     */
    virtual bool seek(int64_t offset, int from = SEEK_SET) = 0;
    // TODO: peek. for probing. https://github.com/rust-av/rust-av/issues/13
    /*!
     * \brief position
     * MUST implement this. Used in seek
     * TODO: implement internally by default
     */
    virtual int64_t position() const = 0;
    /*!
     * \brief size
     * \return <=0 if not support
     */
    virtual int64_t size() const = 0;
    /*!
     * \brief isVariableSize
     * Experiment: A hack for size() changes during playback.
     * If true, containers that estimate duration from pts(or bit rate) will get an invalid duration. Thus no eof get
     * when the size of playback start reaches. So playback will not stop.
     * Demuxer seeking should work for this case.
     */
    virtual bool isVariableSize() const { return false;}
    bool isOpen() const;
    /*!
     * \brief bufferSize
     * Currently buffering is implemented by derived classes to avoid copying. Default is 10M
     */
    int64_t bufferSize() const;
    void setBufferSize(int64_t value);
    
    virtual bool abort() { return false;}
    virtual bool setTimeout(int64_t, TimeoutCallback) { return false;}
protected:
    /*!
     * \brief onUrlChanged
     * Here you can close old url, parse new url() and open it.
     * \return false if failed to open, or url/protocol is not supported
     */
    virtual bool onUrlChanged() {return false;}
    MediaIO();
private:
    class Private;
    std::unique_ptr<Private> d;
};
MDK_NS_END
