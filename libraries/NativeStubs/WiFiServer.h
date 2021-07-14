#pragma once

class WiFiServer {
public:
    WiFiServer(uint16_t port = 80, uint8_t max_clients = 4) {}
    ~WiFiServer() { end(); }

    void       listenOnLocalhost() {}
    WiFiClient available() { return WiFiClient(); }
    WiFiClient accept() { return available(); }
    void       begin(uint16_t port = 0) {}
    void       begin(uint16_t port, int reuse_enable) {}
    void       setNoDelay(bool nodelay) {}
    bool       getNoDelay() { return false; }
    bool       hasClient() { return false; }
    size_t     write(const uint8_t* data, size_t len) { return 0; }
    size_t     write(uint8_t data) { return write(&data, 1); }

    void end() {}
    void close() {}
    void stop() {}

    operator bool() { return false; }

    int  setTimeout(uint32_t seconds) { return seconds; }
    void stopAll() {}
};
