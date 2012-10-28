local(path_here) = currentCapture->callsite_file->stripLastComponent
not #path_here->beginsWith('/')
    ? #path_here = io_file_getcwd + '/' + #path_here
not #path_here->endsWith('/')
    ? #path_here->append('/')
sourcefile(file(#path_here + 'spec_helper.inc'), -autoCollect=false)->invoke


describe(::ds_connect) => {
    it('connects to the specified MySQL server') => {
        local(conn) = ds_connect('mysqlds', -host='localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id) = #conn->getData("SELECT CONNECTION_ID() AS con_id")
            ->getSet(1)->get(INLINE_RESULTROWS_POS)->get(1)->first
        
        expect(boolean(#conn_id->isA(::integer)))
    }
    
    it('works with the query object to retrieve data') => {
        local(conn) = ds_connect('mysqlds', -host='localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id
        expect(boolean(#conn_id->isA(::integer)))
    }
    
    it('allows multiple query objects to use the same connection') => {
        local(conn) = ds_connect('mysqlds', -host='localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(conn_id1) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id
        local(conn_id2) = query(#conn, "SELECT CONNECTION_ID() AS con_id")->first->con_id

        expect(boolean(#conn_id1->isA(::integer)))
        expect(boolean(#conn_id2->isA(::integer)))
        expect(#conn_id1, #conn_id2)
    }
    
    it('allows multiple query objects to share the connection without overwriting data') => {
        local(conn) = ds_connect('mysqlds', -host='localhost', -port=3306, -username='root', -password='')
        handle => { #conn->close }
        
        local(query1) = query(#conn, "SELECT CONNECTION_ID() AS con_id")
        local(query2) = query(#conn, "SELECT * FROM information_schema.CHARACTER_SETS;SELECT CONNECTION_ID() AS con_id;")
            
        expect(1, #query1->numSets)
        expect(2, #query2->numSets)
        expect(#query1->first->con_id, #query2->set(2)->first->con_id)
    }
}