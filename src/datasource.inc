protect => {\database_registry}
define database_registry->getDatasourceHost(name::string, -port::string='', -alias::string='', -schema::string='', -username::string='') => {
    #port != ''
        ? #port = ' AND port = ' + database_qs(#port)
    #alias != ''
        ? #alias = ' AND alias = ' + database_qs(#alias)
    #schema != ''
        ? #schema = ' AND schema = ' + database_qs(#schema)
    #username != ''
        ? #username = ' AND username = ' + database_qs(#username)
        
    with r in .sql->executeLazy('SELECT ' + .hostColumns + ' FROM ' + .dshostsTable + 
        ' WHERE name = ' + database_qs(#name) + #port + #alias + #schema + #username + 
        ' LIMIT 1'
    ) do { return .hostMap(#r) }
}

define ds_connect => type {
    trait { import trait_close }
    data
        protected dsInfo,
        protected capi
    
    public dsInfo => .'dsInfo',
           capi   => .'capi'
    
    public onCreate(ds_type::string, host::string, -port::integer=0, -username::string='', -password::string='', -schema::string='', -encoding::string='UTF-8') => {
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
    
    public onCreate(hostname::string, -alias::string='', -schema::string='', -username::string='', -encoding::string='UTF-8') => {
        local(db_reg) = database_registry
        local(host)   = #db_reg->getDatasourceHost(#hostname, -alias=#alias, -schema=#schema, -username=#username)
        
        fail_if(not #host, 'Could not find data source information for "' + #hostname + '"')
        
        #host->find('port') = integer(#host->find('port'))
            
        return .onCreate(#db_reg->getDatasource(#host->get('id_datasource'))->get('name'),
                       #host->find('name')     || '',
                -port= #host->find('port'), // the default of zero taken care of in casting above
            -username= #host->find('username') || '',
            -password= #host->find('password') || '',
              -schema= #host->find('schema')   || '',
            -encoding= #encoding
        )
    }
    
    public database(db::string)::ds_database => ds_database(self, #db)
    
    public saveConnection(connection::dsInfo) => {
        .'dsInfo' = #connection
        
        // When a command-line process finishes, the db connection should be closed
        // This allows for the database connection to be closed at the end of a web request
        not web_response->isA(::null)
            ? define_atEnd({ self->close })
    }
    
    public getData(sql::string)::dsInfo => {
        local(ds_data) = .dsInfo->makeInheritedCopy
        
        #ds_data->statement    = #sql
        #ds_data->action       = lcapi_datasourceExecSQL
        
        local(err) = null
        protect => {
            handle_error => {
                #err = (: error_code, error_msg, error_stack)
                var(__app_profiler__)?
                    $__app_profiler__->inlineEnd
            }
            var(__app_profiler__)?
                $__app_profiler__->inlineBegin(.dsInfo)
            
            .capi->invoke(#ds_data)
            
            var(__app_profiler__)?
                $__app_profiler__->inlineEnd
        }
        null !== #err?
            fail(#err->get(1), #err->get(2), #err->get(3))
        
        not .dsInfo->connection
            ? .saveConnection(#ds_data->makeInheritedCopy)
        
        return #ds_data
    }
    public getData(database::string, sql::string)::dsInfo => {
        .dsInfo->databaseName = #database
        local(ret) = .getData(#sql)
        .dsInfo->databaseName = ''
        
        return #ret
    }
    
    public query(sql::string)::query => query(self, #sql)
    public query(result_type, sql::string)::query => query(self, #result_type, #sql)
    
    public close => {
        .dsInfo->action = lcapi_datasourceCloseConnection
        .capi->invoke(.dsInfo)
    }
}

define ds_database => type { 
    data
        private connection::ds_connect,
        private database
    
    public connection => .'connection',
           database   => .'database'
        
    public onCreate(connection::ds_connect, database::string) => {
        .'connection' = #connection
        .'database'   = #database
    }
    
    public getData(sql::string)::dsInfo => .connection->getData(.database, #sql)
    
    public query(sql::string)::query => query(self, #sql)
    public query(result_type, sql::string)::query => query(self, #result_type, #sql)
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

        local(err) = null
        protect => {
            handle_error => {
                #err = (: error_code, error_msg, error_stack)
                var(__app_profiler__)?
                    $__app_profiler__->inlineEnd
            }
            var(__app_profiler__)?
                $__app_profiler__->inlineBegin(.dsInfo)
            
            ..capi->invoke(..dsInfo)

            var(__app_profiler__)?
                $__app_profiler__->inlineEnd
        }
        null !== #err?
            fail(#err->get(1), #err->get(2), #err->get(3))
        
        return ..dsInfo
    }
}