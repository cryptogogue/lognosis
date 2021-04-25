// Copyright (c) 2017-2018, Cryptogogue Inc. All Rights Reserved.
// http://cryptogogue.com

#include <map>
#include <memory>
#include <mutex>
#include <string>

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

LogFilter LogFilter::sMaster;
mutex LogFilter::sMasterMutex;

//================================================================//
// Logger
//================================================================//

//----------------------------------------------------------------//
bool filter ( const char* key, Verbosity verbosity ) {

    return LogFilter::get ().filter ( key, verbosity );
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
void setFilter ( const char* filter, Verbosity verbosity ) {

    LogFilter::get ().setFilter ( filter, verbosity );
}

//----------------------------------------------------------------//
void useCurrentFilterForAllNewThreads () {

    LogFilter::get ().promoteToMaster ();
}

} // namespace Lognosis
