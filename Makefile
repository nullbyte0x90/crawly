INC=-I/home/nullbyte/libs/curl-7.45.0/include -I/home/nullbyte/libs/utf8cpp/source
default:
	g++ -std=c++11 -o crawly main.cpp -lcurl $(INC) -g
