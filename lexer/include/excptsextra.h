#ifndef EXTRA_EXCEPTIONS_H
#define EXTRA_EXCEPTIONS_H

#include <string>
#include <exception>

class CompileError : public std::exception{
    private:
        std::string message;
        std::string Line;
        std::string ErrorMsg;
        int lineNumber;
        
    public:
        CompileError(std::string , std::string , int );

        const char* what() const noexcept override;
};




#endif //EXTRA_EXCEPTIONS_H