#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "string"
#include "vector"
#include "memory"
#include "ffmpeg.h"

namespace sm {
namespace media {

struct Pixel {
    uint8_t red, green, blue, alpha;
};

struct Image {
    uint32_t width, height;
    std::vector<Pixel> pixels;
};

class ImageLoader {
public:
    ImageLoader();
    ~ImageLoader();

    std::shared_ptr<Image> loadImage(const std::string &filename);
    std::vector<uint8_t> encodeImage(const std::shared_ptr<Image> &image);

    std::shared_ptr<Image> decodeImage(const std::vector<uint8_t> &vector);

    const std::vector<uint8_t> *myVec;
    uint32_t position;

private:
    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *codecCtx = nullptr;
    AVCodec *codec;
    AVFrame *frame;

    std::vector<uint8_t> buffer;

    std::string filename;

    void close();

    bool encodeFrame(AVFrame *pFrame);

    std::shared_ptr<Image> readFromCtx();
};


std::shared_ptr<Image> loadImage(std::string filename);

std::vector<uint8_t> encodeImage(const std::shared_ptr<Image> &image);

std::shared_ptr<Image> decodeImage(const std::vector<uint8_t> &data);

}
}

#endif //IMAGELOADER_H
