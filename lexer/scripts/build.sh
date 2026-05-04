# g++ main.cpp -x c++ -o main -Wall - Wextra
# | g++ -x c++ -

#!/bin/bash
g++ ../source/main.cpp ../source/excptsextra.cpp ../source/morse.cpp ../source/tokenMaker.cpp -I ../include -o ../compiled/lexer -Wall -Wextra

echo "Build Success"

#cd ../compiled/

#./lexer
