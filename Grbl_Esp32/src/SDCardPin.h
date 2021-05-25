// TODO: This should be merged with SDCard.h but for now doing so
// causes strange include conflicts.

class SDCard : public Configuration::Configurable {
public:
    Pin _cardDetect;

    SDCard() = default;

    // Initializes pins.
    void init();

    // Configuration handlers.
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    ~SDCard() = default;
};
