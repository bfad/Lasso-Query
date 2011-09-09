// This should let us run this anywhere and still properly import the file
local(path_here) = io_file_getcwd + '/' + {}->callsite_file->stripLastComponent
not #path_here->endsWith('/') ? #path_here += '/'
local(f) = file(#path_here + '../query.inc')
sourcefile(#f->readString, #f->path, false, false)->invoke

// Really testing trait_query_result
describe(::query_result) => {
    it("returns the value when the method is a key") => {
        local(values) = map(
             'first'='foo',
            'second'='bar',
             'third'='baz',
         )
        local(qr) = query_result(#values)
        
        //with key in #values->keys do {
        //    expect(#qr->\(tag(#key))->invoke, #values->find(#key))
        //}
        expect(#qr->first , #values->find('first'))
        expect(#qr->second, #values->find('second'))
        expect(#qr->third , #values->find('third'))
    }
}

//stdoutnl({}->callsite_file)

//with key in #values->keys do {
    //stdoutnl(#qr->\(tag(#key))->invoke)
//}

/*
* [describe] registers a context to run tests (it statements)
*   - You should be able to nest [describe] statements with inner having the outer's context
*   - Eventually want before and after methods that will be run before and after each test
*
* [it] registers a test into a [describe] context
*   - I'm thinking it needs access to any locals defined in the [describe] hierarchy
*   - May need to look into closures and yield statements for this
*/
