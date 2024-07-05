#ifdef ARDUINO
    #include <Arduino.h>
    #include "SD.h"
    #include "FS.h"
    #include "SPI.h"
    #include <StreamUtils.h>
#else
    #include <iostream>
    #include <fstream>
#endif

#include <canvas.h>
#include <draw.h>

#include "files.h"
#include "../conf.h"

#ifdef ARDUINO

class ArduinoReadFileStream : public IReadStream {
    public:
        ArduinoReadFileStream(const char* fileName) 
        {
            _file = SD.open(fileName);
           _stream  = new ReadBufferingStream {_file, 2000};
        }

        inline virtual int read() {
            return _stream->read();
        };

        ~ArduinoReadFileStream() {
            _file.close();
            delete _stream;
        }

    private:
        ReadBufferingStream* _stream;
        fs::File _file;
};

class ArduinoFileStreamFactory : public IFileSystem {
    public:
        ArduinoFileStreamFactory(const char* root = NULL)
        {}

        virtual IReadStream* Open(const char* fileName) const
        {
            return new ArduinoReadFileStream(fileName);
        }
};

bool init_sd_card() {
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println("Card Mount Failed");
        return false;
    }
    uint8_t cardType = SD.cardType();
    
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        tft_header_msg("No SD card attached");
        return false;
    }
    
    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC) {
        Serial.println("MMC");
    } else if (cardType == CARD_SD) {
        Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);   
    Serial.println("initialisation done.");
    return true;
}

bool init_file_system() {
    return init_sd_card();
}

IFileSystem* get_file_system(const char* root) {
    return new ArduinoFileStreamFactory(root);
}

#else

class LocalFileStream : public IReadStream {
    public:
        LocalFileStream(const char* fileName) {
            _stream = new std::ifstream(fileName);
        }

        virtual int read() {
            char buffer[1];
            _stream->read(buffer, 1);
            return *buffer;
        };

        ~LocalFileStream() {
            _stream->close();
            delete _stream;
        }

    private:
        std::ifstream* _stream;
};

class LocalFileStreamFactory : public IFileSystem {
    public:
        LocalFileStreamFactory(const char* root = NULL) : _root(root)
        {}

        inline virtual IReadStream* Open(const char* fileName) const {
            auto fullPath =  _root + "/" + fileName;
            return new LocalFileStream(fullPath.c_str());
        }
    private:
        std::string _root;
};

bool init_file_system() {
    return true;
}

IFileSystem* get_file_system(const char* root) {
    return new LocalFileStreamFactory(root);
}
#endif