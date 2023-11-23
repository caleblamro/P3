/*
Abel Haddis and I worked on this together before the solo project rule, Got permission to submit together
We worked on the code together
 */
#ifndef __INPUT_BUFFER__H__
#define __INPUT_BUFFER__H__

#include <string>
#include <vector>

class InputBuffer {
public:
    void GetChar(char&);
    char UngetChar(char);
    std::string UngetString(std::string);
    bool EndOfInput();

private:
    std::vector<char> input_buffer;
};

#endif  //__INPUT_BUFFER__H__
