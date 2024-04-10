#include <opencv4/opencv2/opencv.hpp>
#include<QString>
//defines the size of the header, has to be a set size for decoding purposes
//header is where metadata is stored
#define STEG_HEADER_SIZE sizeof(uint64_t) * 8
#define MESSAGE_LEN_HEADER STEG_HEADER_SIZE / 2
#define NOISE_AMOUNT_HEADER STEG_HEADER_SIZE / 2

class Image {
public:
    Image(const char*);
    Image(){};
    bool isEmpty(){return this->image.empty();}
    bool encode(const char* message ,const char* identifier = nullptr, int noise = 1, bool redundancy = false);
    QString decode();
    QString filename;

private:
    bool read (const char*);
    void write (const char*);
    cv::Mat image;
	unsigned int size = 0;
};
