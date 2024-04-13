#include "headers/Image.h"
#include <iostream>
#include<QDebug>
Image::Image(const char* filename) {
    if(read(filename)) {
        this->filename = filename;
	}
    else {
        qDebug() << "Failed to read " << filename;
	}
}

bool Image::read(const char* filename) {
    this->image.release();
    this->image = cv::imread(filename, cv::IMREAD_UNCHANGED);
    // Check if the image was loaded successfully
    if (image.empty()) {
        qDebug() << "Could not open or find the image.";
        return false;
    }
    //size is the number of bytes in the image
    this->size = image.rows * image.cols * image.channels();
	return true;
}

void Image::write(const char* fileName){
    if (fileName == nullptr) {
        std::cerr << "Filename is null!" << std::endl;
        return;
    }
    if (this->image.empty()) {
        std::cerr << "Attempt to write an empty image!" << std::endl;
        return;
    }
    cv::imwrite(fileName, this->image);
}

bool Image::encode(const char* message, const char* identifier, int noise, bool redundancy){
    //Len is the length in bits of the message
    int len = strlen(message) * 8;

    //if the len of the message+the header is bigger than the number of bytes * the bits used per btye (noise)
    //then the text will not fit
    if(len + STEG_HEADER_SIZE > size*noise) {
        return false;
    }
    //ensures the encoding runs at least once even when redundancy is off
    bool firstRun = true;

    //channel is the current channel the bit is being embedded in
    int channel = 0;

    //position is the number of pixels that have been iterated over
    int position = 0;

    //calculated from the position, used at the y coord when accessing a pixel
    int row = 0;

    //calculated from the position, used at the x coord when accessing a pixel
    int col = 0;

    //offset is where the last loop ended +1, making it moved across the image.
    int offset = 0;

    //This is used when embedding the message, it is the current bit being embedded into the image
    int messageBit = 0;

    //this is used to store which bit within the pixel byte is being edited, it will countdown from noise to 0 then loop
    int noiseBitPosition = 0;

    //this mask is generated to isolate the bit in the current noiseBitPosition so that it can be edited
    unsigned char mask = 0;

    //loop runs once, then if redundancy is on (user has ticked the box) it loops until there is no more room for redundancy
    while(redundancy || firstRun){
        //checks if there is space in the image for another loop
        if (position+(position-offset) >= (size/image.channels())){
            redundancy=false;
            break;
        }
        //only updates the offset after the first run
        if(!firstRun){
            offset = position + 1;
        }

        //FOR EVERY BYTE IN THE IMAGE DATA, SHIFTS IT RIGHT BY INCREASING VALUES
        //THIS CAUSES EACH BIT OF LEN TO BE ANDED WITH 1UL TO CLEAR ALL EXCEPT THE RIGHTMOST BIT
        //I.E 10110111 WOULD BECOME 0000001 AND 11011010 WOULD BECOME 00000000
        //THE RIGHTMOST BIT IS THEN OR'ED WITH data[i]
        //COMPARING THE ONE BIT ISOLATED WITH LEN WITH data[i] MEANS THAT IF THE RIGHTMOST BIT OF LEN IS 1 THEN data[i] RIGHTMOST BIT
        //BECOMES THE SAME


        //this loop is for adding the length of the hidden message to the start of the image
        channel = 0;
        position = offset;
        row = 0;
        col = 0;
        //pixel is an array where each item is the pixel data for a channel in that pixel
        for(int i = 0; i < MESSAGE_LEN_HEADER; ++i){
            //resets the channel to zero everytime it reaches the top channel
            //only increases the position when all channels have been used
            if (channel >= image.channels()){
                ++position;
                channel = 0;
            }
            row = position / image.cols;
            col = position % image.cols;
            uchar* pRow = image.ptr<uchar>(row);
            int index = (col * image.channels()) + channel;
            uchar& pixel = pRow[index];
            //sets the LSB to zero
            pixel &= 0xFE;
            //shifts the len binary value along so that the next bit in the Loop is it's LSB, the &1Ul sets everything except the LSB to zero
            //oring that with the current pixel channel data makes the LSB match the current len bit
            pixel |= (len >> (MESSAGE_LEN_HEADER - 1 - i)) & 1UL;
            ++channel;
        }

        //embeds the noise amount as meta data within it's section
        channel = 0;
        position = MESSAGE_LEN_HEADER + offset;
        row = 0;
        col = 0;
        //same logic as before, now encoding depth
        for(int i = 0; i < NOISE_AMOUNT_HEADER; ++i){
            if (channel >= image.channels()){
                ++position;
                channel = 0;
            }
            row = position / image.cols;
            col = position % image.cols;
            uchar* pRow = image.ptr<uchar>(row);
            int index = (col * image.channels()) + channel;
            uchar& pixel = pRow[index];
            pixel &= 0xFE;
            pixel |= (noise >> (NOISE_AMOUNT_HEADER - 1 - i)) & 1UL;
            ++channel;
        }

        channel = 0;
        position = STEG_HEADER_SIZE + offset;
        row = 0;
        col = 0;
        messageBit = 0;
        noiseBitPosition = 0;
        mask = 0;
        //now adding the message itself after the header
        for(int i = 0; i < len; ++i) {
            //channel only increases once we've reached the LSB
            if (i > 0 && i % noise == 0){
                ++channel;
                if (channel >= image.channels()){
                    channel = 0;
                    ++position;
                }
            }
            row = position / image.cols;
            col = position % image.cols;
            uchar* pRow = image.ptr<uchar>(row);
            int index = (col * image.channels()) + channel;
            uchar& pixel = pRow[index];

            // Calculate the noiseBitPosition for the current bit
            // This is a countdown from (noise-1) to 0, then wraps around
            noiseBitPosition = (noise - 1) - ((i % noise) % noise);

            // Extract the current bit from the message
            //i in message[i/8] is divided by 8 as i is meant to iterate up to len which is stored in bits but message is an array of bytes
            //dividing by 8 means that up to 8 the index is zero (1/8=0.125=0) and then after 8 it is 1 (9/8=1.125=1)
            //anded with 1 to isolate the LSB same as before
            messageBit = (message[i / 8] >> (7 - (i % 8))) & 1;

            // Create a mask to set all bits to 1 except target bit
            mask = ~(1 << noiseBitPosition);

            // ANDing the mask leave all bits unchanged except the target bit which becomes zero
            // shifting the message bit means that it moves to the position og the target bit
            // i.e if target bit is 3 message bit becomes 1000 instead of 0001
            // ORing the two sets the pixel bit to 1 if messagebit is 1 and leaves it as zero if it is zero
            pixel = (pixel & mask) | (messageBit << noiseBitPosition);

        }
        firstRun = false;
    }
    write((QString("../StegApp/imgs/output/output") + identifier + ".png").toStdString().c_str());
    return true;
}

QString Image::decode() {
    unsigned int len = 0;
    int depth = 0;

	//(len<<1) shifts everything left each time so that the data is loaded into the bit right to left
    //pixel[channel] is anded to 1 to get the rightmost bit same as before
    //very similar logic, just in reverse adding pixel data to message.
    int position = 0;
    int channel = 0;
    for(int i = 0; i < MESSAGE_LEN_HEADER; ++i){
        if (channel >= image.channels()){
            ++position;
            channel = 0;
        }
        int row = position / image.cols;
        int col = position % image.cols;
        uchar* pRow = image.ptr<uchar>(row);
        int index = (col * image.channels()) + channel;
        uchar& pixel = pRow[index];
        len = (len << 1) | (pixel & 1);
        ++channel;
	}
    //creates the buffer with the size of the message length. +10 to be safe so it doesn't cut off early
    char *buffer = new char[(len+10)/8];
    memset(buffer, 0, (len + 10) / 8);

    channel = 0;
    position = MESSAGE_LEN_HEADER;
    //gets the depth of pixel bits it needs to check i.e noise
    for(int i = 0; i < NOISE_AMOUNT_HEADER; ++i){
        if (channel >= image.channels()){
            ++position;
            channel = 0;
        }
        int row = position / image.cols;
        int col = position % image.cols;
        uchar* pRow = image.ptr<uchar>(row);
        int index = (col * image.channels()) + channel;
        uchar& pixel = pRow[index];
        depth = (depth << 1) | (pixel & 1);
        ++channel;
    }


    //Same logic as before this time applied to the buffer and stored in the message as a char array.
    position = STEG_HEADER_SIZE;
    channel = 0;
    //Checks that the size of the message given by the metadata could fit in the image and that the depth is within range
    //if either are false we can be certain the meta data is wrong and therefore the image is not encoded/encoded data was corrupted
    if(len<(size*depth) && (depth > 0 && depth <= 6)){
        for(int i = 0; i < len; ++i){
            if (i > 0 && i % depth == 0){
                ++channel;
                if (channel >= image.channels()){
                    channel = 0;
                    ++position;
                }
            }
            int row = position / image.cols;
            int col = position % image.cols;
            uchar* pRow = image.ptr<uchar>(row);
            int index = (col * image.channels()) + channel;
            uchar& pixel = pRow[index];

            //same logic as before however the mask is not needed as we do not need to clear the buffer because we are appending it
            int noiseBitPosition = (depth - 1) - ((i % depth) % depth);
            buffer[i/8] = (buffer[i/8] << 1) | ((pixel & (1 << noiseBitPosition)) >> noiseBitPosition);
        }
    }
	return buffer;
}
