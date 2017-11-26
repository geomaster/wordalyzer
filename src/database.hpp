#pragma once
#include <string>
#include <vector>
#include <exception>

#include "audio.hpp"

struct sqlite3;

namespace wordalyzer {
    class database_exception : public std::exception {
    private:
        int ret;
        std::string message;

    public:
        database_exception(int _ret, const std::string& _message) :
            ret(_ret), message(_message) {}
        database_exception(int _ret, std::string&& _message) :
            ret(_ret), message(std::move(_message)) {}

        const char* what() const throw()
        {
            return message.c_str();
        }

        int get_return_code()
        {
            return ret;
        }
    };

    class no_such_clip_exception : public std::exception {
    private:
        std::string message;

    public:
        no_such_clip_exception(const std::string& clip_name) :
            message("No such clip: `" + clip_name + "`") {}

        const char* what() const throw()
        {
            return message.c_str();
        }
    };

    class duplicate_clip_exception : public std::exception {
    private:
        std::string message;

    public:
        duplicate_clip_exception(const std::string& clip_name) :
            message("Duplicate clip name: `" + clip_name + "`") {}

        const char* what() const throw()
        {
            return message.c_str();
        }
    };

    class database {
    private:
        sqlite3* db;

        int check_ret(int ret);
        bool clip_exists(const std::string& name);
        static const char* get_schema();

    public:
        database(const std::string& filename);

        std::vector<std::string> get_all_clip_names();
        clip_t get_clip(const std::string& clip_name);
        void remove_clip(const std::string& clip_name);
        void add_clip(const clip_t& clip);

        word_t get_clip_word(const std::string& clip_name, int word_idx);

        ~database();
    };
}
