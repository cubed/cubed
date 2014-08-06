/*
This holds utilities for disk and network IO
*/

#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>

namespace ioutil {

// Returns true if the file at path file_name exists
inline bool file_exists(const char *file_name) {
    struct stat buffer;
    return (stat (file_name, &buffer) == 0);
}

// Multiwriter is a simple class that allows writing to multiple ostream objects at once.
class MultiWriter {
    private:
        std::vector<std::ostream*> mwli;

    public:
        MultiWriter() {}

        // Add an ostream pointer to the list of printables
        void add(std::ostream *f) {
            mwli.push_back(f);
        }

        // Write to the ostreams (all at once)
        void write(const char* s, std::streamsize n) {
            for ( auto &i : mwli ) {
                i->write(s, n);
            }
        }
};

}