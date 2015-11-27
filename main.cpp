#include <iostream>
#include <stdio.h>
#include <string>

#include "curl/curl.h"

const char * dir = "/home/nullbyte/gits/crawly/pliczek";
const char * config = ".conf"; //TODO sterowanie programu plikiem .conf
bool htmlOnly = true;
bool verbose = true;

size_t writeFunc(void * buffer, size_t size, size_t numberMembers, void * userp) {

    size_t realSize = size * numberMembers; //liczba bajtow
    std::string * contentString = (std::string *)userp;
    contentString->append((char *) buffer, realSize);


    return realSize; //zwraca liczbe "obsluzonych" bajtow
}

bool getToFile(std::string * url, std::string * filename) {//glupia funkcja pobierajaca do pliku
    CURL * curl = curl_easy_init();

    if (curl) {
        CURLcode result;

        //pliczek
        FILE * file = fopen(dir, "w+");
        if (file == NULL) {
            std::cout << "Error opening file" << std::endl;
            return true;
        }

        std::string bigString;


        curl_easy_setopt(curl, CURLOPT_URL, url->c_str()); // ustaw url'a
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); //nie sprawdzaj certow
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc);//ustaw funkcje callback dla libcurla
        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bigString);

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        result = curl_easy_perform(curl); //faktyczny transfer

        curl_easy_cleanup(curl);

        return false;
    } else {
        return true;
    }
}

bool checkType(std::string type, CURL * curl) {//sprawdz HEAD'em czy typ sie zgadza
    std::string headerString;

    curl_easy_setopt(curl, CURLOPT_HEADER, 1); // dolacz headery do info
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1); //w przypadku http/https praktycznie HEAD, inne protokoly - nie pobieraj body
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &headerString);
    //na tym etapie dostaniemy headery

    curl_easy_perform(curl); //pobierz naglowek

    int findPosition = headerString.find(type);

    if (findPosition != headerString.npos) {//znaleziono stringa
        //posprzataj dla przyszlego transferu body
        if(verbose) std::cout<<"Type ok"<<std::endl;
        if(verbose) std::cout<<headerString<<std::endl;
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        return true;
    }else{
        if(verbose) std::cout<<"Type not correct"<<std::endl;
        return false;
    }


}

bool getToString(std::string * url, std::string * targetString) {
    CURL * curl = curl_easy_init();
    std::string headerString;

    if (curl) {
        CURLcode result;


        curl_easy_setopt(curl, CURLOPT_URL, url->c_str()); // ustaw url'a
        if (verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc); //ustaw funkcje callback dla libcurla
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //podazaj za przekierowywaniem

        if (htmlOnly) {
            std::string type = "text/html"; //TODO poszukuj danych typow w linkach, lista typow w configu
            if (checkType(type, curl) == true) {//jesli typ sie zgadza to pobieramy body
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, targetString);
                result = curl_easy_perform(curl); //faktyczny transfer
            }else{//jesli nie to 
                curl_easy_cleanup(curl);
                return true;
            }
        } else {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, targetString); //pointer przekazywany do callbacka
            curl_easy_perform(curl);
        }
        

        curl_easy_cleanup(curl); //zamkniecie curla

        return false;
    } else {
        return true;
    }
}

int main(int argc, char **argv) {
    curl_global_init(CURL_GLOBAL_DEFAULT); //inicjalizacja

    std::string input(argv[1]);
    std::string output;


    getToString(&input, &output);

    std::cout << output << std::endl;

    return 0;
}
