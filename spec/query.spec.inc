if(not lasso_tagExists(::query)) => {
    // This should let us run this anywhere and still properly import the file
    local(path_here) = io_file_getcwd + '/' + {}->callsite_file->stripLastComponent
    not #path_here->endsWith('/') ? #path_here += '/'
    local(f) = file(#path_here + '../query.inc')
    sourcefile(#f->readString, #f->path, false, false)->invoke
}

lcapi_loadModule(sys_userCapiModulePath + 'MySQLConnector.dylib')
local(conn) = array(
    -datasource='mysqlds', 
          -name='localhost', 
          -port='3306', 
      -username='root', 
      -password=''
)

describe("This test file") => {
    it("connects to specified MySQL server") => {
        local(conn_id)
        
        expect->error(error_code_noerror, error_msg_noerror) => {
            inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
                #conn_id = field('con_id')
            }
        }
        expect(error_code_noerror, error_code)
        expect(boolean(#conn_id->isA(::integer)))
    }
}
if(error_code == error_code_noerror) => {
describe(::query) => {
    it("shares the connection when declared and executed inside an inline") => {
        local(conn_id, inline_conn_id)
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            query(-sql="SELECT CONNECTION_ID() AS con_id")->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#inline_conn_id, #conn_id)
    }
    
    it("shares the connection when executed inside an inline though declared outside of it") => {
        local(conn_id, inline_conn_id)
        local(query) = query(-sql="SELECT CONNECTION_ID() AS con_id")
        
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#inline_conn_id, #conn_id)
    }
    
    it("doesn't share the connection when executed outside an inline though declared inside of it") => {
        local(conn_id, inline_conn_id)
        
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            local(query)    = query(-sql="SELECT CONNECTION_ID() AS con_id")
        }
        
        expect(not #query->data_loaded)
        expect->error => {
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
    }

    it("shares the connection when declared and executed inside another [query]") => {
        local(conn_id, outer_conn_id)
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = field('con_id')
            query(-sql="SELECT CONNECTION_ID() AS con_id")->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#outer_conn_id, #conn_id)
    }
    
    it("shares the connection when executed inside another [query] though declared outside of it") => {
        local(conn_id, outer_conn_id)
        local(query) = query(-sql="SELECT CONNECTION_ID() AS con_id")
        
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = field('con_id')
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#outer_conn_id, #conn_id)
    }
    
    it("doesn't share the connection when executed outside another [query] though declared inside of it") => {
        local(conn_id, outer_conn_id)
        
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = field('con_id')
            local(query)    = query(-sql="SELECT CONNECTION_ID() AS con_id")
        }
        
        expect(not #query->data_loaded)
        expect->error => {
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
    }
    
    it("returns a [trait_queriable] whose each iteration is a [query_result]") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id")
        
        expect(#query->isA(::trait_queriable) != 0)
        
        with row in #query do => {
            expect(#row->isA(::query_result) != 0)
        }
    }
    
    it("returns a [trait_queriable] whose each iteration is of type passed as first argument") => {
        define tmp_query_result => type {
            parent query_result
            
            public onCreate(arg::map) => {
                ..onCreate(#arg)
            }
        }
        
        local(query) = query('tmp_query_result', -host=#conn, -sql="SELECT CONNECTION_ID() AS con_id")
        
        expect(#query->isA(::trait_queriable) != 0)

        with row in #query do => {
            expect(#row->type, 'tmp_query_result')
        }
    }

    it("stores values in an [array] when multiple columns share the same name") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id, 'foo' AS con_id")
        
        with row in #query do => {
            expect(#row->isA(::query_result)  != 0)
            expect(#row->con_id->isA(::array) != 0)
            expect('foo', #row->con_id->second)
        }
    }
}
}