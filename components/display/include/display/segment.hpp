// SPDX-FileCopyrightText: 2023 Matthew Nickson <mnickson@sidingsmedia.com>
// SPDX-License-Identifier: MIT

#ifndef DISPLAY_SEGMENT_H_
#define DISPLAY_SEGMENT_H_

class Segment
{
private:
protected:
    int max_chars_;
    char* message_;
public:
    // Constructor. Create display with a maximum length
    Segment(int len);

    // Write a given message to the display
    virtual void Write(char* msg) = 0;
};

#endif  // DISPLAY_SEGMENT_H_
