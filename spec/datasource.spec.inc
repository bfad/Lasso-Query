local(path_here) = currentCapture->callsite_file->stripLastComponent
not #path_here->beginsWith('/')
    ? #path_here = io_file_getcwd + '/' + #path_here
not #path_here->endsWith('/')
    ? #path_here->append('/')
sourcefile(file(#path_here + 'spec_helper.inc'), -autoCollect=false)->invoke


var(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
handle => { $conn->close }

describe(::ds_connect) => {
    it('connects to the specified MySQL server') => {
        local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id) = #conn->getData("SELECT CONNECTION_ID() AS con_id")
            ->getSet(1)->get(INLINE_RESULTROWS_POS)->get(1)->first
        
        expect(boolean(#conn_id->isA(::integer)))
    }
    
    it('works with the query object to retrieve data') => {
        local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id
        expect(boolean(#conn_id->isA(::integer)))
    }
    
    it('allows multiple query objects to use the same connection') => {
        local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id1) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id
        local(conn_id2) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id

        expect(boolean(#conn_id1->isA(::integer)))
        expect(boolean(#conn_id2->isA(::integer)))
        expect(#conn_id1, #conn_id2)
    }
    
    it('allows multiple query objects to share the connection without overwriting data') => {
        local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(query1) = query(#conn, "SELECT CONNECTION_ID() AS con_id")
        local(query2) = query(#conn, "SELECT * FROM information_schema.CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;")
            
        expect(1, #query1->numSets)
        expect(2, #query2->numSets)
        expect(#query1->first->con_id, #query2->set(2)->first->con_id)
    }
    
    // Need tests for database_registry lookup
    // (Problem is how to set it up, especially since it's instance based)
    // (Do we assume global_command_line home or home home, etc?)
    
    it('runs a query in the context of a specified database') => {
        local(conn) = ds_connect('mysqlds', 'localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(num_rows) = #conn->getData('information_schema', "SELECT * FROM CHARACTER_SETS")
            ->getSet(1)->get(INLINE_RESULTROWS_POS)->size
        expect(#num_rows > 0)
    }
    
    it("each row of the first set is of type specified and following sets are the default") => {
        local(query) = query('tmp_query_result1', 
            $conn,
            "SELECT * FROM information_schema.CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;"
        )
        with row in #query do => {
            expect('tmp_query_result1', #row->type)
        }
        with row in #query->set(2) do => {
            expect('query_result', #row->type)
        }
    }
    
    it("each row of each set is of the types specified") => {
        local(query) = query((:'tmp_query_result1', 'tmp_query_result2'), 
            $conn,
            "SELECT * FROM information_schema.CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;"
        )
        with row in #query do => {
            expect('tmp_query_result1', #row->type)
        }
        with row in #query->set(2) do => {
            expect('tmp_query_result2', #row->type)
        }
    }
}

describe(::ds_database) => {
    it('runs a query in the context of a specified ds_connection and database') => {
        local(num_rows) = ds_database($conn, 'information_schema')->getData("SELECT * FROM CHARACTER_SETS")
            ->getSet(1)->get(INLINE_RESULTROWS_POS)->size
        expect(#num_rows > 0)
    }
    
    it('works with the query object to retrieve data') => {
        local(db) = ds_database($conn, 'information_schema')
        
        local(num_rows) = query(#db, "SELECT * FROM CHARACTER_SETS")->size
        expect(#num_rows > 0)
    }
    
    it("each row of the first set is of type specified and following sets are the default") => {
        local(query) = query('tmp_query_result1', 
            ds_database($conn, 'information_schema'),
            "SELECT * FROM CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;"
        )
        with row in #query do => {
            expect('tmp_query_result1', #row->type)
        }
        with row in #query->set(2) do => {
            expect('query_result', #row->type)
        }
    }
    
    it("each row of each set is of the types specified") => {
        local(query) = query((:'tmp_query_result1', 'tmp_query_result2'), 
            ds_database($conn, 'information_schema'),
            "SELECT * FROM CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;"
        )
        with row in #query do => {
            expect('tmp_query_result1', #row->type)
        }
        with row in #query->set(2) do => {
            expect('tmp_query_result2', #row->type)
        }
    }
}