#include <sqlite3.h>
#include <string.h>

#include "database.hpp"

using namespace wordalyzer;
using namespace std;

int wordalyzer::database::check_ret(int ret)
{
    if (ret != SQLITE_OK && ret != SQLITE_DONE && ret != SQLITE_ROW) {
        if (db != nullptr) {
            throw database_exception(ret, sqlite3_errmsg(db));
        } else {
            throw database_exception(ret, "An SQLite initialization error has occurred.");
        }
    }

    return ret;
}

const char* wordalyzer::database::get_schema()
{
    return
        "CREATE TABLE IF NOT EXISTS clip("
        "   name TEXT PRIMARY KEY,"
        "   vector_size INTEGER,"
        "   window_size INTEGER,"
        "   window_stride INTEGER);"

        "CREATE TABLE IF NOT EXISTS word("
        "   clip_name TEXT,"
        "   word_index INTEGER,"
        "   vectors_serialized BLOB,"
        "   PRIMARY KEY (clip_name, word_index));"

        "CREATE INDEX IF NOT EXISTS word_by_clip"
        "   ON word(clip_name);";
}

wordalyzer::database::database(const std::string& filename) : db(nullptr)
{
    check_ret(sqlite3_open(filename.c_str(), &db));

    // Create the schema. This is done every time, as the schema contains
    // IF NOT EXISTS clauses.
    const char* schema = get_schema();
    check_ret(sqlite3_exec(db, schema, nullptr, 0, nullptr));
}

void wordalyzer::database::add_clip(const clip_t& clip)
{
    if (clip_exists(clip.name)) {
        throw duplicate_clip_exception(clip.name);
    }

    // Add the clip entry
    const char clip_statement_str[] =
        "INSERT INTO clip (name, vector_size, window_size, window_stride)"
        "   VALUES (?, ?, ?, ?)";

    sqlite3_stmt* clip_statement = nullptr;
    check_ret(sqlite3_prepare_v3(db,
                                 clip_statement_str,
                                 sizeof(clip_statement_str),
                                 0,
                                 &clip_statement,
                                 nullptr));
    try {
        check_ret(sqlite3_bind_text(clip_statement,
                                    1,
                                    clip.name.c_str(),
                                    clip.name.length(),
                                    SQLITE_TRANSIENT));
        check_ret(sqlite3_bind_int(clip_statement, 2, clip.vector_size));
        check_ret(sqlite3_bind_int(clip_statement, 3, clip.window_size));
        check_ret(sqlite3_bind_int(clip_statement, 4, clip.window_stride));

        check_ret(sqlite3_step(clip_statement));
    } catch (database_exception& e) {
        sqlite3_finalize(clip_statement);
        throw e;
    }

    sqlite3_finalize(clip_statement);

    // Add the word entries for words in the clip
    const char word_statement_str[] =
        "INSERT INTO word (clip_name, word_index, vectors_serialized)"
        "   VALUES (?, ?, ?)";

    sqlite3_stmt* word_statement = nullptr;
    check_ret(sqlite3_prepare_v3(db,
                                 word_statement_str,
                                 sizeof(word_statement_str),
                                 0,
                                 &word_statement,
                                 nullptr));

    try {
        check_ret(sqlite3_bind_text(word_statement,
                                    1,
                                    clip.name.c_str(),
                                    clip.name.length(),
                                    SQLITE_TRANSIENT));
        for (size_t i = 0; i < clip.words.size(); i++) {
            vector<byte> serialized = serialize_word(clip.words[i]);

            check_ret(sqlite3_bind_int(word_statement, 2, i));
            check_ret(sqlite3_bind_blob(word_statement,
                                        3,
                                        &serialized[0],
                                        serialized.size(),
                                        SQLITE_TRANSIENT));

            check_ret(sqlite3_step(word_statement));
        }
    } catch  (database_exception& e) {
        sqlite3_finalize(word_statement);
        throw e;
    }

    sqlite3_finalize(word_statement);
}

bool wordalyzer::database::clip_exists(const string& name)
{
    const char select_statement_str[] =
        "SELECT name FROM clip WHERE name = ?";

    sqlite3_stmt* select_statement = nullptr;
    check_ret(sqlite3_prepare_v3(db,
                                 select_statement_str,
                                 sizeof(select_statement_str),
                                 0,
                                 &select_statement,
                                 nullptr));
    try {
        check_ret(sqlite3_bind_text(select_statement,
                                            1,
                                            name.c_str(),
                                            name.length(),
                                            SQLITE_TRANSIENT));

        int ret = check_ret(sqlite3_step(select_statement));
        sqlite3_finalize(select_statement);
        return ret == SQLITE_ROW;
    } catch (database_exception& e) {
        sqlite3_finalize(select_statement);
        throw e;
    }
}

vector<string> wordalyzer::database::get_all_clip_names()
{
    const char select_statement_str[] =
        "SELECT name FROM clip;";

    sqlite3_stmt* select_statement;
    check_ret(sqlite3_prepare_v3(db,
                                 select_statement_str,
                                 sizeof(select_statement_str),
                                 0,
                                 &select_statement,
                                 nullptr));

    try {
        vector<string> results;
        while (check_ret(sqlite3_step(select_statement)) == SQLITE_ROW) {
            const unsigned char* name = sqlite3_column_text(select_statement, 0);
            if (name != nullptr) {
                results.push_back(reinterpret_cast<const char*>(name));
            }
        }

        sqlite3_finalize(select_statement);
        return results;
    } catch (database_exception& e) {
        sqlite3_finalize(select_statement);
        throw e;
    }
}

void wordalyzer::database::remove_clip(const string& clip_name)
{
    const char clip_statement_str[] =
        "DELETE FROM clip WHERE name = ?";

    sqlite3_stmt* clip_statement;
    check_ret(sqlite3_prepare_v3(db,
                                 clip_statement_str,
                                 sizeof(clip_statement_str),
                                 0,
                                 &clip_statement,
                                 nullptr));

    try {
        check_ret(sqlite3_bind_text(clip_statement,
                                    1,
                                    clip_name.c_str(),
                                    clip_name.length(),
                                    SQLITE_TRANSIENT));

        check_ret(sqlite3_step(clip_statement));
    } catch (database_exception& e) {
        sqlite3_finalize(clip_statement);
        throw e;
    }

    sqlite3_finalize(clip_statement);

    const char word_statement_str[] =
        "DELETE FROM word WHERE clip_name = ?";

    sqlite3_stmt* word_statement;
    check_ret(sqlite3_prepare_v3(db,
                                 word_statement_str,
                                 sizeof(word_statement_str),
                                 0,
                                 &word_statement,
                                 nullptr));

    try {
        check_ret(sqlite3_bind_text(word_statement,
                                    1,
                                    clip_name.c_str(),
                                    clip_name.length(),
                                    SQLITE_TRANSIENT));
        check_ret(sqlite3_step(word_statement));
    } catch (database_exception& e) {
        sqlite3_finalize(word_statement);
        throw e;
    }

    sqlite3_finalize(word_statement);
}

clip_t wordalyzer::database::get_clip(const string& clip_name)
{
    const char clip_statement_str[] =
        "SELECT vector_size, window_size, window_stride FROM clip WHERE name = ?";

    sqlite3_stmt* clip_statement = nullptr;
    check_ret(sqlite3_prepare_v3(db,
                                 clip_statement_str,
                                 sizeof(clip_statement_str),
                                 0,
                                 &clip_statement,
                                 nullptr));

    clip_t result;
    try {
        check_ret(sqlite3_bind_text(clip_statement,
                                    1,
                                    clip_name.c_str(),
                                    clip_name.length(),
                                    SQLITE_TRANSIENT));

        int ret = check_ret(sqlite3_step(clip_statement));

        if (ret == SQLITE_ROW) {
            result.vector_size = sqlite3_column_int(clip_statement, 0);
            result.window_size = sqlite3_column_int(clip_statement, 1);
            result.window_stride = sqlite3_column_int(clip_statement, 2);
        } else {
            throw no_such_clip_exception(clip_name);
        }
    } catch (database_exception& e) {
        sqlite3_finalize(clip_statement);
        throw e;
    }

    sqlite3_finalize(clip_statement);

    const char word_statement_str[] =
        "SELECT word_index, vectors_serialized FROM word WHERE clip_name = ? ORDER BY word_index";

    sqlite3_stmt* word_statement = nullptr;
    check_ret(sqlite3_prepare_v3(db,
                                 word_statement_str,
                                 sizeof(word_statement_str),
                                 0,
                                 &word_statement,
                                 nullptr));
    try {
        check_ret(sqlite3_bind_text(word_statement,
                                    1,
                                    clip_name.c_str(),
                                    clip_name.length(),
                                    SQLITE_TRANSIENT));

        int next_index = 0;
        while (check_ret(sqlite3_step(word_statement)) == SQLITE_ROW) {
            int new_index = sqlite3_column_int(word_statement, 0);
            if (new_index != next_index++) {
                throw database_exception(-1, "Word indexes in a clip not valid, the database might be corrupted.");
            }

            vector<byte> word_bytes;
            word_bytes.resize(sqlite3_column_bytes(word_statement, 1));
            memcpy(&word_bytes[0], sqlite3_column_blob(word_statement, 1), word_bytes.size());

            result.words.push_back(deserialize_word(word_bytes));
        }
    } catch (database_exception& e) {
        sqlite3_finalize(word_statement);
        throw e;
    }

    sqlite3_finalize(word_statement);
    return result;
}

wordalyzer::database::~database()
{
    sqlite3_close(db);
}
