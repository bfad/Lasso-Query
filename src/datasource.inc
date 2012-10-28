define ds_connect => type {
    data
        protected dsInfo,
        protected capi
    
    public dsInfo => .'dsInfo',
           capi   => .'capi'
    
    public onCreate(ds_type::string, -host::string, -port::integer=0, -username::string='', -password::string='', -schema::string='', -encoding::string='UTF-8') => {
        .dsInfo = dsInfo
        .dsInfo->hostDatasource    = #ds_type
        .dsInfo->hostName          = #host
        .dsInfo->hostPort          = #port->asString
        .dsInfo->hostUsername      = #username
        .dsInfo->hostPassword      = #password
        .dsInfo->hostSchema        = #schema
        .dsInfo->hostTableEncoding = #encoding
        .dsInfo->maxRows           = -1
        
        .capi = \(#ds_type)
    }
    
    public saveConnection(connection::dsInfo) => {
        .'dsInfo' = #connection
    }
    
    public getData(sql::string)::dsInfo => {
        local(ds_data) = .dsInfo->makeInheritedCopy
        
        #ds_data->statement    = #sql
        #ds_data->action       = lcapi_datasourceExecSQL
        
        .capi->invoke(#ds_data)
        
        not .dsInfo->connection
            ? .saveConnection(#ds_data->makeInheritedCopy)
        
        return #ds_data
    }
    
    public close => {
        .dsInfo->action = lcapi_datasourceCloseConnection
		.capi->invoke(.dsInfo)
    }
}


protect => {\inline_type}
define inline_type->accessDoClose => .doClose
define query_inline_source => type {parent inline_type
    public getData(...) => {
        ..reset(:#rest)

        ..capi and ..accessDoClose?
            handle => { ..close }
        ..capi and ..dsInfo->action == lcapi_datasourcePrepareSQL ?
            handle => { ..closePrepared }

        local(code, msg)
        protect => {
            handle_error => {
                #code = error_code
                #msg  = error_msg
    		    var(__app_profiler__)?
                    $__app_profiler__->inlineEnd
            }
            var(__app_profiler__)?
			    $__app_profiler__->inlineBegin(.dsInfo)
            
            ..capi->invoke(..dsInfo)

            var(__app_profiler__)?
			    $__app_profiler__->inlineEnd
        }
		void != #code?
			error_code = #code
        void != #msg?
			error_msg = #msg
        
        return ..dsInfo
    }
}