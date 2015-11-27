#include <iostream>
#include <stdio.h>
#include <string>

#include "curl/curl.h"

const char * dir = "/home/nullbyte/gits/crawly/pliczek";


size_t writeFunc(void * buffer, size_t size, size_t numberMembers, void * userp) {
    std::cout<<"Liczba size = "<<size<<std::endl;
    std::cout<<"Liczba nmemb = "<<numberMembers<<std::endl;
    size_t realSize = size * numberMembers;
    std::string * contentString = (std::string *)userp;
    contentString->append((char *)buffer, realSize);


    return realSize;
}


int main(int argc, char **argv)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL * curl = curl_easy_init();

    if(curl) {
        CURLcode result;

        //pliczek
        FILE * file = fopen(dir, "w+");
        std::string bigString;


        curl_easy_setopt(curl, CURLOPT_URL, argv[1]); // ustaw url'a
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//nie sprawdzaj certow
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);//ustaw funkcje callback dla libcurla
        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bigString);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        result = curl_easy_perform(curl);//faktyczny transfer

        // std::cout<<bigString<<std::endl;
        curl_easy_cleanup(curl);
    }

    return 0;
}
