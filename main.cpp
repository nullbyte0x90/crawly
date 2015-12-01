#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <locale.h>
#include <algorithm>


#include "curl/curl.h"
#include "utf8.h"

using namespace std;

const char * dir = "/home/nullbyte/gits/crawly/pliczek";
const char * config = ".conf"; //TODO sterowanie programu plikiem .conf
string openingSeparators = "> \"(\n";
string closingSeparators = "\")  <\n";
bool htmlOnly = true;
bool verbose = false;

size_t writeFunc(void * buffer, size_t size, size_t numberMembers, void * userp) {

    size_t realSize = size * numberMembers; //liczba bajtow
    string * contentString = (string *) userp;
    contentString->append((char *) buffer, realSize);


    return realSize; //zwraca liczbe "obsluzonych" bajtow
}

bool getToFile(string * url, string * filename) {//glupia funkcja pobierajaca do pliku
    CURL * curl = curl_easy_init();

    if (curl) {
        CURLcode result;

        //pliczek
        FILE * file = fopen(dir, "w+");
        if (file == NULL) {
            cout << "Error opening file" << endl;
            return true;
        }

        string bigString;


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

bool checkType(string type, CURL * curl) {//sprawdz HEAD'em czy typ sie zgadza
    string headerString;

    curl_easy_setopt(curl, CURLOPT_HEADER, 1); // dolacz headery do info
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1); //w przypadku http/https praktycznie HEAD, inne protokoly - nie pobieraj body
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &headerString);
    //na tym etapie dostaniemy headery

    curl_easy_perform(curl); //pobierz naglowek

    int findPosition = headerString.find(type);

    if (findPosition != headerString.npos) {//znaleziono stringa
        //posprzataj dla przyszlego transferu body
        if (verbose) cout << "Type ok" << endl;
        if (verbose) cout << headerString << endl;
        curl_easy_setopt(curl, CURLOPT_NOBODY, 0);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        return true;
    } else {
        if (verbose) cout << "Type not correct" << endl;
        return false;
    }


}

bool getToString(string * url, string * targetString) {
    CURL * curl = curl_easy_init();
    string headerString;

    if (curl) {
        CURLcode result;


        curl_easy_setopt(curl, CURLOPT_URL, url->c_str()); // ustaw url'a
        if (verbose) curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeFunc); //ustaw funkcje callback dla libcurla
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //podazaj za przekierowywaniem

        if (htmlOnly) {
            string type = "text/html"; //TODO poszukuj danych typow w linkach, lista typow w configu
            if (checkType(type, curl) == true) {//jesli typ sie zgadza to pobieramy body
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, targetString);
                result = curl_easy_perform(curl); //faktyczny transfer
            } else {//jesli nie to 
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

vector<string> * getUrls(string &parseString, vector<string> * outputVector) {
    int searchFromWhere = 0; //odkad szukamy


    int found, foundEnd; // pozycje

    while ((found = parseString.find("href=", searchFromWhere)) != string::npos) {
        found += 6; //przesuniecie na poczatek linku
        foundEnd = parseString.find("\"", found); //znalezienie zamykajacego apostrofu
        string url = parseString.substr(found, foundEnd - found); //substring miedzy nimi

        //cout<<url<<endl;

        outputVector->push_back(url); //dodaj linka do listy

        searchFromWhere = foundEnd; //szukaj od ostatniego
    }

}

bool isOpeningSeparator(char character) {
    for (char definedChar : openingSeparators) {
        if (definedChar == character) {
            return true;
        }
    }
    return false;
}

bool isClosingSeparator(char character) {
    for (char definedChar : closingSeparators) {
        if (definedChar == character) {
            return true;
        }
    }
    return false;
}

//word valid

bool wordValid(string &word) {
    locale loc;
    if(word.empty()) return false;
    for (int i = 0; i < word.size(); i++) {
        if (!isalpha(word[i], loc)) return false;
    }
    
    return true;
}


vector<string> * getWords(string &sourceHTML, vector<string> * words) {
    string word;
    bool wordStarted;
    for (int i = 0; i < sourceHTML.size(); i++) {
        if (isOpeningSeparator(sourceHTML[i])) {
            i++;
            //char test = sourceHTML[i];
            while (!isClosingSeparator(sourceHTML[i]) && i < sourceHTML.size()) {
                word += sourceHTML[i];
                i++;
            }
            if (wordValid(word)) {
                transform(word.begin(), word.end(), word.begin(), ::tolower);
                words->push_back(word);
                word.clear();
            }else{
                word.clear();
            }
        }
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        cout << "Provide the url to kick in" << endl;
        return 1;
    }
    vector<string> urls;
    curl_global_init(CURL_GLOBAL_DEFAULT); //inicjalizacja
    string input(argv[1]);
    urls.push_back(input);
    string output;
    vector<string> words;

    do {
        cout << "New link" << endl;
        getToString(&(urls.at(0)), &output);
        getUrls(output, &urls);
        getWords(output, &words);
        cout << "Got " << words.size() << "words\n";
        for (string word : words) {
            cout << word << "\n";
        }

        urls.erase(urls.begin());
    } while (urls.size() < 1000 && urls.size() != 0);
    return 0;
}
