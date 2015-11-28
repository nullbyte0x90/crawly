INC=-I/home/nullbyte/libs/curl-7.45.0/include
default:
	g++ -std=c++11 -o crawly main.cpp -lcurl $(INC) -g
