#include "headers/Image.h"
#include <iostream>
#include<QDebug>
Image::Image(const char* filename) {
    if(read(filename)) {
        printf("Read %s\n", filename);
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
    printf(message);
	//Len is the length in bits of the message
    int len = strlen(message) * 8;
	if(len + STEG_HEADER_SIZE > size) {
		printf("\e[31m[ERROR] This message is too large (%lu bits / %u bits)\e[0m\n", len+STEG_HEADER_SIZE, size);
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

	for(int i = 0; i < STEG_HEADER_SIZE; ++i){
        int row = i / image.cols;
        int col = i % image.cols;
		cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);
		pixel[0] &= 0xFE;
		pixel[0] |= (len >> (STEG_HEADER_SIZE - 1 - i)) & 1UL;
	}
	//this loop is for adding the message itself
	//same principal as before only difference is we are shifting along the message binary
	//the shift amount is the length to make the message binary be added right to left instead of left to right
	//i in message[i/8] is divided by 8 as i is meant to iterate up to len which is stored in bits but message is an array of bytes
	//dividing by 8 means that up to 8 the index is zero (1/8=0.125=0) and then after 8 it is 1 (9/8=1.125=1)
	//division is always rounded down
    qDebug()<<image.cols;
    qDebug()<<image.rows;
	for(int i = 0; i<len; ++i){
        int row = (i+STEG_HEADER_SIZE) / image.cols;
        int col = (i+STEG_HEADER_SIZE) % image.cols;
        qDebug()<<col;
        qDebug()<<row;
		cv::Vec3b &pixel = image.at<cv::Vec3b>(row, col);
		pixel[0] &= 0xFE;
		pixel[0] |= (message[i/8] >> ((len-1-i)%8)) & 1;
	}

    write((QString("../StegApp/imgs/output") + identifier + ".png").toStdString().c_str());
}

QString Image::decode() {
	int len = 0;
	char buffer[256] = {0};
	//(len<<1) shifts everything left each time so that the data is loaded into the bit right to left
	//Data[i] is anded to 1 to get the rightmost bit same as before
	for(int i = 0; i < STEG_HEADER_SIZE; ++i){
        int row = i / image.cols;
        int col = i % image.cols;
		cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);
		len = (len << 1) | (pixel[0] & 1);
	}
	//gets the message length in bytes
	//int messageLength = len / 8;
	//Same logic as before this time applied to the message and stored in the message as a char array. see main for message decleration
    if(len<(image.cols*image.rows)){
        for(int i = 0; i < len; ++i){
            int row = (i+STEG_HEADER_SIZE) / image.cols;
            int col = (i+STEG_HEADER_SIZE) % image.cols;
            cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);
            buffer[i/8] = (buffer[i/8] << 1) | (pixel[0] & 1);
        }
    }
	return buffer;
}
