#include "excptsextra.h"
#include <string>


CompileError::CompileError(std::string message, std::string Line, int lineNumber){
            this->message = message;
            this->Line = Line;
            this->lineNumber = lineNumber;
            ErrorMsg = "Compile Error : " + message +" At Line (" + std::to_string(lineNumber+1) +") : " + Line ;
        }

const char* CompileError::what() const noexcept {
    return ErrorMsg.c_str();
}