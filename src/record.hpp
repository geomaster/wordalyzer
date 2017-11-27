#pragma once
#include <exception>
#include <string>

#include "audio.hpp"

namespace wordalyzer {
    class recording_exception : public std::exception {
    private:
        std::string message;

    public:
        recording_exception(const std::string& _message) : message(_message) {}
        recording_exception(std::string&& _message) : message(std::move(_message)) {}

        const char* what() const throw()
        {
            return message.c_str();
        }
    };

    audio_t record_audio();
}
