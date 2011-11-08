if(not var(__loaded_query)) => {
    // This should let us run this anywhere and still properly import the file
    local(path_here) = io_file_getcwd + '/' + {}->callsite_file->stripLastComponent
    not #path_here->endsWith('/') ? #path_here += '/'
    local(f) = file(#path_here + '../query.inc')
    sourcefile(#f->readString, #f->path, false, false)->invoke
    var(__loaded_query) = true
}

// Really testing trait_query_result
describe(::query_result) => {
    beforeEach => {
        local(values) = map(
             'first'='foo',
            'second'='bar',
             'third'='baz',
        )
        local(qr) = query_result(#values)
    }
    
    it("returns the value when the method is a key") => {
        //with key in #values->keys do {
        //    expect(#qr->\(tag(#key))->invoke, #values->find(#key))
        //}
        expect(#qr->first , #values->find('first'))
        expect(#qr->second, #values->find('second'))
        expect(#qr->third , #values->find('third'))
    }
    
    it("fails with a 'method not found' error if the method is not in the map") => {
        expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
            #qr->two
        }
    }
    
    it("updates the value for the specified method on assignment") => {
        #qr->third = 'theta'
        
        expect(#qr->first , #values->find('first'))
        expect(#qr->second, #values->find('second'))
        expect(#qr->third , 'theta')
    }
    
    describe("'c' method with arguments") => {
        it("returns the value for the argument it's passed") => {
            expect(#qr->c('first') , #values->find('first'))
            expect(#qr->c('second'), #values->find('second'))
            expect(#qr->c('third') , #values->find('third'))
        }
        
        it("fails with a 'method not found' error if the argument is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                #qr->c('two')
            }
        }
        
        it("updates the value for the specified argument on assignment") => {
            #qr->c('third') = 'theta'
            
            expect(#qr->c('first') , #values->find('first'))
            expect(#qr->c('second'), #values->find('second'))
            expect(#qr->c('third') , 'theta')
        }
    }
    
    describe("'c' method without arguments") => {
        it("returns the value for c in the map") => {
            #qr = query_result(map('c'='rhino'))
            
            expect(#qr->c  , 'rhino')
            expect(#qr->c(), 'rhino')
        }
        
        it("fails with a 'method not found' error if c is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                #qr->c
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                #qr->c()
            }
        }
        
        it("updates the value for c on assignment") => {
            #qr = query_result(map('c'='rhino'))
            
            #qr->c = 'theta'
            expect(#qr->c  , 'theta')
            expect(#qr->c(), 'theta')
            
            #qr->c() = 'gamma'
            expect(#qr->c  , 'gamma')
            expect(#qr->c(), 'gamma')
        }
    }
}