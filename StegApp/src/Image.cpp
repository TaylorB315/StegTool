#include "headers/Image.h"
#include <iostream>
#include<QDebug>
Image::Image(const char* filename) {
    if(read(filename)) {
        printf("Read %s\n", filename);
        this->filename = filename;
	}
    else {
        printf("Failed to read %s\n", filename);
	}
}

bool Image::read(const char* filename) {
	this->image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    // Check if the image was loaded successfully
    if (image.empty()) {
        std::cout << "Could not open or find the image." << std::endl;
        return false;
    }
	this->size = image.rows * image.cols; //* image.channels(); TODO:ADD MULTI CHANNEL EMBEDDING
	return true;
}

bool Image::write(const char* filename){
	return cv::imwrite(filename, image);
}

void Image::encode(const char* message, const char* identifier, int noise){
	//Len is the length in bits of the message
    int len = strlen(message) * 8;
    if(len + STEG_HEADER_SIZE > size*noise) {
        printf("\e[31m[ERROR] This message is too large (%lu bits / %u bits)\e[0m\n", len+STEG_HEADER_SIZE, size*noise);
		return;
	}
	//pixel is an array where each item is the pixel data for a channel in that pixel
	//FOR EVERY BYTE IN THE IMAGE DATA, SHIFTS IT RIGHT BY INCREASING VALUES
	//THIS CAUSES EACH BIT OF LEN TO BE ANDED WITH 1UL TO CLEAR ALL EXCEPT THE RIGHTMOST BIT
	//I.E 10110111 WOULD BECOME 0000001 AND 11011010 WOULD BECOME 00000000 
	//THE RIGHTMOST BIT IS THEN OR'ED WITH data[i]
	//COMPARING THE ONE BIT ISOLATED WITH LEN WITH data[i] MEANS THAT IF THE RIGHTMOST BIT OF LEN IS 1 THEN data[i] RIGHTMOST BIT
	//BECOMES THE SAME

	//this loop is for adding the length of the hidden message to the start of the image

    for(int i = 0; i < MESSAGE_LEN_HEADER; ++i){
        int row = i / image.cols;
        int col = i % image.cols;
		cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);
		pixel[0] &= 0xFE;
        pixel[0] |= (len >> (MESSAGE_LEN_HEADER - 1 - i)) & 1UL;
	}

    //embeds the noise amount as meta data within it's section
    for(int i = 0; i < NOISE_AMOUNT_HEADER; ++i){
        int row = (i + MESSAGE_LEN_HEADER) / image.cols;
        int col = (i + MESSAGE_LEN_HEADER) % image.cols;
        cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);
        pixel[0] &= 0xFE;
        pixel[0] |= (noise >> (NOISE_AMOUNT_HEADER - 1 - i)) & 1UL;
    }
	//this loop is for adding the message itself
	//same principal as before only difference is we are shifting along the message binary
	//the shift amount is the length to make the message binary be added right to left instead of left to right
	//i in message[i/8] is divided by 8 as i is meant to iterate up to len which is stored in bits but message is an array of bytes
	//dividing by 8 means that up to 8 the index is zero (1/8=0.125=0) and then after 8 it is 1 (9/8=1.125=1)
	//division is always rounded down
//	for(int i = 0; i<len; ++i){
//        int row = (i+STEG_HEADER_SIZE) / image.cols;
//        int col = (i+STEG_HEADER_SIZE) % image.cols;
//		cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);
//		pixel[0] &= 0xFE;
//		pixel[0] |= (message[i/8] >> ((len-1-i)%8)) & 1;
//	}
    int position = STEG_HEADER_SIZE;
    for(int i = 0; i < len; ++i) {
        //position only increases once we've reached the LSB
        if (i > 0 && i % noise == 0){
            ++position;
        }
        int row = position / image.cols;
        int col = position % image.cols;
        cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);

        for (int j = 0; j < noise; ++j){

        }
        // Calculate the noiseBitPosition for the current bit
        // This is a countdown from (noise-1) to 0, then wraps around
        int noiseBitPosition = (noise - 1) - ((i % noise) % noise);

        // Extract the current bit from the message
        int messageBit = (message[i / 8] >> (7 - (i % 8))) & 1;

        // Create a mask to set all bits to 1 except target bit
        unsigned char mask = ~(1 << noiseBitPosition);
        // ANDing the mask leave all bits unchanged except the target bit which becomes zero
        // shifting the message bit means that it moves to the position og the target bit
        // i.e if target bit is 3 message bit becomes 1000 instead of just 1
        // ORing the two sets the pixel bit to 1 if messagebit is 1 and leaves it as zero if it is zero
        pixel[0] = (pixel[0] & mask) | (messageBit << noiseBitPosition);

    }


    write((QString("../StegApp/imgs/output") + identifier + ".png").toStdString().c_str());
}

QString Image::decode() {
    unsigned int len = 0;
    int depth =0;

	//(len<<1) shifts everything left each time so that the data is loaded into the bit right to left
	//Data[i] is anded to 1 to get the rightmost bit same as before
    for(int i = 0; i < MESSAGE_LEN_HEADER; ++i){
        int row = i / image.cols;
        int col = i % image.cols;
		cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);
		len = (len << 1) | (pixel[0] & 1);
	}
    //creates the buffer with the size of the message length. +7 so it doesn't cut off early
    char *buffer = new char[(len+10)/8];

    //gets the depth of pixel bits it needs to check i.e noise
    for(int i = 0; i < NOISE_AMOUNT_HEADER; ++i){
        int row = (i + MESSAGE_LEN_HEADER) / image.cols;
        int col = (i + MESSAGE_LEN_HEADER) % image.cols;
        cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);
        depth = (depth << 1) | (pixel[0] & 1);
    }


    //Same logic as before this time applied to the buffer and stored in the message as a char array.
    int position = STEG_HEADER_SIZE;
    if(len<(image.cols*image.rows)){
        for(int i = 0; i < len; ++i){
            if (i > 0 && i % depth == 0){
                ++position;
            }
            int row = position / image.cols;
            int col = position % image.cols;
            cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);

            int noiseBitPosition = (depth - 1) - ((i % depth) % depth);
            buffer[i/8] = (buffer[i/8] << 1) | ((pixel[0] & (1 << noiseBitPosition)) >> noiseBitPosition);
        }
    }
	return buffer;
}
