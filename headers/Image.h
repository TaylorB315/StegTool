#include <opencv2/opencv.hpp>
#include <string>

#define STEG_HEADER_SIZE sizeof(uint32_t) * 8

class Image {
public:
    Image(const char*);
    void encode(const char*);
    std::string decode ();

private:
    bool read (const char*);
    bool write (const char*);
    cv::Mat image;
	unsigned int size = 0;
};