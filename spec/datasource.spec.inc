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
}