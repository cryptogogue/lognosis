// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#ifndef LOGNOSIS_LOGGER_H
#define LOGNOSIS_LOGGER_H

#include <loguru/loguru.hpp>

#define LGN_LOG(key, verbosity_name, ...)                                           \
    {                                                                               \
        loguru::Verbosity verbosity = loguru::Verbosity_ ## verbosity_name;         \
        if ( Lognosis::filter ( key, ( Lognosis::Verbosity )verbosity )) {          \
            (( verbosity ) > loguru::current_verbosity_cutoff())                    \
                ? (void)0                                                           \
                : loguru::log(verbosity, __FILE__, __LINE__, key, __VA_ARGS__);     \
        }                                                                           \
    }

#define LGN_LOG_SCOPE(key, verbosity_name, ...)                                                                 \
    loguru::LogScopeRAII LOGURU_ANONYMOUS_VARIABLE ( error_context_RAII_ ) =                                    \
    ( !Lognosis::filter ( key, ( Lognosis::Verbosity )loguru::Verbosity_ ## verbosity_name ))                   \
        ? loguru::LogScopeRAII ()                                                                               \
        : loguru::LogScopeRAII ( loguru::Verbosity_ ## verbosity_name, __FILE__, __LINE__, key, __VA_ARGS__ )

namespace Lognosis {

    enum Verbosity {
        OFF         = -9,
        FATAL       = -3,
        ERROR       = -2,
        WARNING     = -1,
        INFO        = 0,
        MAX         = 9,
    };

//----------------------------------------------------------------//
bool        filter                                  ( const char* key, Verbosity verbosity );
void        init                                    ( int argc, char **argv );
void        setFilter                               ( const char* filter, Verbosity verbosity );
void        useCurrentFilterForAllNewThreads        ();

} // namespace Lognosis
#endif
