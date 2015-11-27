INC=-I/home/nullbyte/libs/curl-7.45.0/include
default:
	g++ -o crawly main.cpp -lcurl $(INC)
