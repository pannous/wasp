#include <curl/curl.h>
#include <curl/easy.h>
#include <sstream>
#include <iostream>
//using namespace std;

typedef const char *chars;


size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
	std::string data((chars) ptr, (size_t) size * nmemb);
	*((std::stringstream *) stream) << data << std::endl;
	return size * nmemb;
}

//extern "C"
chars fetch(chars url) {
    std::stringstream out;
    auto curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); //Prevent "numberjmp causes uninitialized stack frame" bug
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
//		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    char *copy = (char *) malloc(sizeof(char) * out.str().length() + 1);
    sprintf(copy, "%s", out.str().data());
//	strcpy(copy, data);
//	printf("%s", copy);
    return copy;
}
