// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#include <limits.h>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <sys/stat.h>

using namespace std;

#include <lognosis/Logger.h>

namespace Lognosis {

//================================================================//
// LogFilter
//================================================================//
class LogFilter {
public:
    
    static LogFilter sMaster;
    static mutex sMasterMutex;
    
    map < string, Verbosity > mFilter;
    
    //----------------------------------------------------------------//
    bool filter ( string key, Verbosity verbosity ) const {
    
        map < string, Verbosity >::const_iterator modeIt = this->mFilter.find ( key );
        if ( modeIt != this->mFilter.end ()) {
            return verbosity <= modeIt->second;
        }
        return true;
    }
    
    //----------------------------------------------------------------//
    static LogFilter& get () {
        static thread_local unique_ptr < LogFilter > filter;
        
        if ( !filter ) {
            
            filter = make_unique < LogFilter >();
            
            sMasterMutex.lock ();
            *filter = sMaster;
            sMasterMutex.unlock ();
        }
        
        return *filter;
    }
    
    //----------------------------------------------------------------//
    void promoteToMaster () {
    
        sMasterMutex.lock ();
        sMaster = *this;
        sMasterMutex.unlock ();
    }
    
    //----------------------------------------------------------------//
    void setFilter ( string filter, Verbosity mode ) {

        this->mFilter [ filter ] = mode;
    }
};

LogFilter           LogFilter::sMaster;
mutex               LogFilter::sMasterMutex;

static string       sLogFilePath;
static string       sLogFileName;
static size_t       sLogFileHistoryLength   = 0;
static size_t       sLogFileMaxSizeInBytes  = 0;

//----------------------------------------------------------------//
string      formatLogFileName   ( string path, size_t index, bool error );

//================================================================//
// Logger
//================================================================//

//----------------------------------------------------------------//
bool filter ( const char* key, Verbosity verbosity ) {

    return LogFilter::get ().filter ( key, verbosity );
}

//----------------------------------------------------------------//
string formatLogFileName ( size_t index, bool error ) {

    char buffer [ PATH_MAX ];

    string ext = error ? ".error.log" : ".log";

    if ( index == 0 ) {
        snprintf ( buffer, sizeof ( buffer ), "%s/%s%s", sLogFilePath.c_str (), sLogFileName.c_str (), ext.c_str ());
    }
    else {
        snprintf ( buffer, sizeof ( buffer ), "%s/%s.%d%s", sLogFilePath.c_str (), sLogFileName.c_str (), ( int )index, ext.c_str ());
    }
    return buffer;
}

//----------------------------------------------------------------//
void init ( int argc, char **argv ) {

    loguru::g_stderr_verbosity = loguru::Verbosity_MAX;
    
    loguru::g_preamble          = false;
    
    loguru::g_preamble_date     = false;
    loguru::g_preamble_time     = false;
    loguru::g_preamble_uptime   = false;
    loguru::g_preamble_thread   = false;
    loguru::g_preamble_file     = false;
    loguru::g_preamble_verbose  = false;
    loguru::g_preamble_pipe     = false;
    
    loguru::init ( argc, argv, NULL );
    
    useCurrentFilterForAllNewThreads ();
}

//----------------------------------------------------------------//
void initLogFiles ( const char* logFilePath, const char* logFileName, size_t historyLength, size_t maxSizeInBytes ) {

    sLogFilePath            = logFilePath;
    sLogFileName            = logFileName;
    sLogFileHistoryLength   = historyLength;
    sLogFileMaxSizeInBytes  = maxSizeInBytes;
    
    if (( sLogFilePath.size () == 0 ) || ( sLogFileName.size () == 0 )) return;

    if ( !( sLogFilePath.back () == '/' )) {
        sLogFilePath.push_back ( '/' );
    }

    loguru::create_directories ( sLogFilePath.c_str ());

    char buffer [ PATH_MAX + 1 ];
    char* abspath = realpath ( logFilePath, buffer );
    
    assert ( abspath );

    sLogFilePath = abspath;
    
    loguru::add_file ( formatLogFileName ( 0, false ).c_str (), loguru::Append, loguru::Verbosity_MAX );
    loguru::add_file ( formatLogFileName ( 0, true ).c_str (), loguru::Append, loguru::Verbosity_WARNING );
        
    rotateLogFiles ();
    
}

//----------------------------------------------------------------//
void rotateLogFiles () {

    if (( sLogFilePath.size () == 0 ) || ( sLogFileName.size () == 0 )) return;

    string current      = formatLogFileName ( 0, false );
    string currentErr   = formatLogFileName ( 0, true );
    
    struct stat buffer0;
    struct stat buffer1;
    
    stat ( current.c_str (), &buffer0 );
    stat ( current.c_str (), &buffer1 );
    
    if ((( size_t )buffer0.st_size < sLogFileMaxSizeInBytes ) && (( size_t )buffer1.st_size < sLogFileMaxSizeInBytes )) return;
    
    loguru::remove_callback ( current.c_str ());
    loguru::remove_callback ( currentErr.c_str ());
    
    for ( size_t i = 0; i < sLogFileHistoryLength; ++i ) {
    
        size_t index = sLogFileHistoryLength - i - 1;
        
        string logFileName = formatLogFileName ( index, false );
        string errFileName = formatLogFileName ( index, true );
                
        if ( i == 0 ) {
            remove ( logFileName.c_str ());
            remove ( errFileName.c_str ());
        }
        else {
            rename ( logFileName.c_str (), formatLogFileName ( index + 1, false ).c_str ());
            rename ( errFileName.c_str (), formatLogFileName ( index + 1, true ).c_str ());
        }
    }
    
    loguru::add_file ( current.c_str (), loguru::Append, loguru::Verbosity_MAX );
    loguru::add_file ( currentErr.c_str (), loguru::Append, loguru::Verbosity_WARNING );
}

//----------------------------------------------------------------//
void setFilter ( const char* filter, Verbosity verbosity ) {

    LogFilter::get ().setFilter ( filter, verbosity );
}

//----------------------------------------------------------------//
void useCurrentFilterForAllNewThreads () {

    LogFilter::get ().promoteToMaster ();
}

} // namespace Lognosis
