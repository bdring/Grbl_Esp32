class WebSocketsServer {
public:
    WebSocketsServer(uint16_t port, String origin = "", String protocol = "arduino") {}
    ~WebSocketsServer(void) {}

    void broadcastBIN(uint8_t* buf, size_t len) {}
};
