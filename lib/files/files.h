#pragma once

class IReadStream {
    public:
        virtual int read() = 0;

        virtual ~IReadStream(){}
};

class IFileSystem {
    public:
        virtual IReadStream* Open(const char* fileName) const = 0;
};

bool init_file_system();

IFileSystem* get_file_system(const char* root = (const char*)0);