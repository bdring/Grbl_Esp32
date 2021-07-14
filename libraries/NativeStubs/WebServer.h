#pragma once

typedef int         HTTPMethod;
typedef const char* Uri;
typedef void (*THandlerFunction)(void);

#define CONTENT_LENGTH_UNKNOWN ((size_t)-1)

#define HTTP_ANY 1
class WebServer {
public:
    WebServer(IPAddress addr, int port = 80);
    WebServer(int port = 80) {}
    ~WebServer() = default;

    void begin() {};

    void setContentLength(const size_t contentLength) {}
    void collectHeaders(const char* headerKeys[], const size_t headerKeysCount) {}  // set the request headers to collect
    void on(const Uri& uri, HTTPMethod method, THandlerFunction fn) {}
    void onNotFound(THandlerFunction fn) {}    //called when handler is not assigned
    void onFileUpload(THandlerFunction fn) {}  //handle file uploads

    String arg(String name) { return name; }      // get request argument value by name
    bool   hasArg(String name) { return false; }  // check if argument exists

    template <typename T>
    size_t streamFile(T& file, const String& contentType) {
        return 0;
    }

    void sendHeader(const String& name, const String& value, bool first = false) {}
    void sendContentLength(size_t len) {}
    void sendContent(const String& content) {}
    void send_P(int code, const char* content_type, const char* content, size_t contentLength) {}
    void send(int n) {}

    // WiFiClient client() { return _currentClient; }
    void handleClient() {}
};
