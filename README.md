# SteganographyTool
Steganography tool for use in watermarking made for final year project
This tool uses Qmake and openCV.
There are two views within the Ui encode and Decode.
Encode view contains
    - a feild to enter text, this is the text that will be embedded into the image
    - a button to upload text from a txt file that will be used to fill the text feild
    - a checkbox for redundancy that determines if the message is added once or as many times as possible
    - a box for detimining the number of images made
    - a dropdown box for determinging if the emebed text includes a numerical ID or a unique time code
    - a drop down box that determines the amount of noise the emebeded text causes. more noise allows for more text to be embeded
    - an upload image button that loads the image to be embeded into the given frame
    - a generate button for creating the steg image
Decode view contains
    - an upload image button that has the same back end functionality as the encode version
    - a checkbox for decididing if the output should be saved to a txt
    - a feild to enter the name of the file for said output
    - a decode button for extracting the text
    - an uneditable text feild that will contain and decoded text
    