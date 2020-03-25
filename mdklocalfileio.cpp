#include "mdklocalfileio.h"
#include <QtCore/QtDebug>
#include <QtCore/QUrl>

MdkLocalFileIO::MdkLocalFileIO():
    mdk::MediaIO()
{
    // empty
}

void MdkLocalFileIO::registerOnce()
{
    qDebug() << "Registering MdkLocalFileIO";
    MediaIO::registerOnce(NAME, []{ return new MdkLocalFileIO();});
}

const char *MdkLocalFileIO::name() const
{
    return NAME;
}

const std::set<std::string> &MdkLocalFileIO::protocols() const {
    qDebug() << "Localfiles protocols requested";
    static const std::set<std::string> s{PROTOCOL};
    return s;
}

int64_t MdkLocalFileIO::read(uint8_t *data, int64_t maxSize)
{
    if (!_videoFile || !_videoFile->isOpen()) {
        return 0;
    }
    return _videoFile->read(reinterpret_cast<char*>(data), maxSize);
}

bool MdkLocalFileIO::seek(int64_t offset, int from)
{
    if (!_videoFile || !_videoFile->isOpen()) {
        return false;
    }

    qint64 position = offset;
    if (from == SEEK_CUR) {
        position += _videoFile->pos();
    } else if (from == SEEK_END) {
        position += size();
    }
    return _videoFile->seek(position);
}

int64_t MdkLocalFileIO::position() const
{
    if (!_videoFile || !_videoFile->isOpen()) {
        return 0;
    }
    return _videoFile->pos();
}

int64_t MdkLocalFileIO::size() const
{
    if (!_videoFile || !_videoFile->isOpen()) {
        return 0;
    }
    return _videoFile->size();
}

bool MdkLocalFileIO::onUrlChanged()
{
    if (_videoFile != nullptr) {
        _videoFile->close();
        _videoFile.reset();
    }

    if (url().empty())
        return true;

    QUrl protocolUrl(QUrl(QString::fromStdString(url())));
    protocolUrl.setScheme("file");

    _videoFile = std::make_unique<QFile>(protocolUrl.toLocalFile());

    qDebug() << "Localfile: Opening" << _videoFile->fileName();
    if (!_videoFile->open(QFile::ReadOnly)) {
        qDebug() << "Unable to open" << _videoFile->fileName();
        _videoFile.reset();
        return false;
    }

    return true;
}
