#ifndef MDKLOCALFILEIO_H
#define MDKLOCALFILEIO_H

#include <set>
#include <QtCore/QFile>


// MediaIO.h and global.h from MDK have some unused parameters. We'll ignore those
// warnings.
#if defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif
#include "MediaIO.h"
#if defined __GNUC__
#pragma GCC diagnostic push
#elif defined __clang__
#pragma clang diagnostic pop
#endif

class MdkLocalFileIO: public MDK_NS::MediaIO
{
public:
    static constexpr char const * NAME = "MdkLocalFileIO";
    static constexpr char const * PROTOCOL = "localfile";

    MdkLocalFileIO();

    ~MdkLocalFileIO() override = default;

    static void registerOnce();

    const char* name() const override;

    const std::set<std::string> &protocols() const override;

    /** Always seekable! */
    bool isSeekable() const override { return true; }
    /** We don't need or want any writing done */
    bool isWritable() const override { return false; }

    /** Reading from file */
    int64_t read(uint8_t *data, int64_t maxSize) override;

    /** No writing is possible */
    int64_t write(const uint8_t *, int64_t) override { return 0; }

    /** Seek in file */
    bool seek(int64_t offset, int from = SEEK_SET) override;

    /** Get the current position */
    int64_t position() const override;

    /** Get the size of the video file */
    int64_t size() const override;

protected:
    bool onUrlChanged() override;
private:
    std::unique_ptr<QFile> _videoFile;
};

#endif // MDKLOCALFILEIO_H
