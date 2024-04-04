#include <opencv4/opencv2/opencv.hpp>
#include<QString>
#define STEG_HEADER_SIZE sizeof(uint32_t) * 8

class Image {
public:
    Image(){printf("No image loaded");};
    Image(const char*);
    //Encode with and without identifiers
    void encode(const char* message ,const char* identifier = nullptr, int noise = 0);
    QString decode();

private:
    bool read (const char*);
    bool write (const char*);
    cv::Mat image;
	unsigned int size = 0;
};
