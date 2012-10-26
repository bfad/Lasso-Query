local(path_here) = currentCapture->callsite_file->stripLastComponent
not #path_here->beginsWith('/')
    ? #path_here = io_file_getcwd + '/' + #path_here
not #path_here->endsWith('/')
    ? #path_here->append('/')
sourcefile(file(#path_here + 'spec_helper.inc'), -autoCollect=false)->invoke


// Really testing trait_query_result
describe(::query_result) => {
    beforeEach => {
        var(values) = map(
             'first'='foo',
            'second'='bar',
             'third'='baz',
        )
        var(qr) = query_result($values->keys, $values->values)
    }
    
    it("returns the value when the method is a key") => {
        //with key in $values->keys do {
        //    expect($qr->\(tag(#key))->invoke, $values->find(#key))
        //}
        expect($qr->first , $values->find('first'))
        expect($qr->second, $values->find('second'))
        expect($qr->third , $values->find('third'))
    }
    
    it("fails with a 'method not found' error if the method is not in the map") => {
        expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
            $qr->two
        }
    }
    
    it("updates the value for the specified method on assignment") => {
        $qr->third = 'theta'
        
        expect($qr->first , $values->find('first'))
        expect($qr->second, $values->find('second'))
        expect($qr->third , 'theta')
    }
    
    describe("'c' method with arguments") => {
        it("returns the value for the argument it's passed") => {
            expect($qr->c('first') , $values->find('first'))
            expect($qr->c('second'), $values->find('second'))
            expect($qr->c('third') , $values->find('third'))
        }
        
        it("fails with a 'method not found' error if the argument is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->c('two')
            }
        }
        
        it("updates the value for the specified argument on assignment") => {
            $qr->c('third') = 'alpha'
            
            expect($qr->c('first') , $values->find('first'))
            expect($qr->c('second'), $values->find('second'))
            expect($qr->c('third') , 'alpha')
        }
    }
    
    describe("'c' method without arguments") => {
        it("returns the value for c in the map") => {
            local(result) = query_result((:'c'), (:'rhino'))
            
            expect(#result->c  , 'rhino')
            expect(#result->c(), 'rhino')
        }
        
        it("fails with a 'method not found' error if c is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->c
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->c()
            }
        }
        
        it("updates the value for c on assignment") => {
            local(result) = query_result((:'c'), (:'rhino'))
            
            #result->c = 'beta'
            expect(#result->c  , 'beta')
            expect(#result->c(), 'beta')
            
            #result->c() = 'gamma'
            expect(#result->c  , 'gamma')
            expect(#result->c(), 'gamma')
        }
    }
    
    describe("'invoke' special method with arguments") => {
        it("returns the value for the argument it's passed") => {
            expect($qr->invoke('first') , $values->find('first'))
            expect($qr->invoke('second'), $values->find('second'))
            expect($qr->invoke('third') , $values->find('third'))
            
            expect($qr('first') , $values->find('first'))
            expect($qr('second'), $values->find('second'))
            expect($qr('third') , $values->find('third'))
        }
        
        it("fails with a 'method not found' error if the argument is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->invoke('two')
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr('two')
            }
        }
        
        it("updates the value for the specified argument on assignment") => {
            $qr->invoke('third') = 'delta'
            
            expect($qr->invoke('first') , $values->find('first'))
            expect($qr->invoke('second'), $values->find('second'))
            expect($qr->invoke('third') , 'delta')
            
            $qr('third') = 'centauri'
            
            expect($qr('first') , $values->find('first'))
            expect($qr('second'), $values->find('second'))
            expect($qr('third') , 'centauri')
        }
    }
    
    describe("'invoke' special method without arguments") => {
        it("returns the value for invoke in the map") => {
            local(result) = query_result((:'invoke'), (:'rhino'))
            
            expect(#result->invoke  , 'rhino')
            expect(#result->invoke(), 'rhino')
            expect(#result()        , 'rhino')
        }
        
        it("fails with a 'method not found' error if invoke is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->invoke
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->invoke()
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr()
            }
        }
        
        it("updates the value for invoke on assignment") => {
            local(result) = query_result((:'invoke'), (:'rhino'))
            
            #result->invoke = 'sigma'
            expect(#result->invoke  , 'sigma')
            expect(#result->invoke(), 'sigma')
            expect(#result()        , 'sigma')
            
            #result->invoke() = 'mu'
            expect(#result->invoke  , 'mu')
            expect(#result->invoke(), 'mu')
            expect(#result()        , 'mu')
            
            #result() = 'eta'
            expect(#result->invoke  , 'eta')
            expect(#result->invoke(), 'eta')
            expect(#result()        , 'eta')
        }
    }
    
    describe("'colPosition' method with arguments") => {
        it("returns the first position in the array for the specified column name") => {
            local(result) = query_result((:'one', 'two', 'two'), (:'foo', 'bar', 'baz'))
            
            expect(#result->colPosition('one'), 1)
            expect(#result->colPosition('two'), 2)
        }
        
        it("fails with a 'method not found' error if the specified column name is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->colPosition('two')
            }
        }
    }
    
    describe("'colPosition' method without arguments") => {
        it("returns the value for colPosition in the map") => {
            local(result) = query_result((:'colPosition'), (:'rhino'))
            
            expect(#result->colPosition  , 'rhino')
            expect(#result->colPosition(), 'rhino')
        }
        
        it("fails with a 'method not found' error if colPosition is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->colPosition
            }
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->colPosition()
            }
        }
        
        it("updates the value for colPosition on assignment") => {
            local(result) = query_result((:'colPosition'), (:'rhino'))
            
            #result->colPosition = 'phi'
            expect(#result->colPosition  , 'phi')
            expect(#result->colPosition(), 'phi')
            
            #result->colPosition() = 'nu'
            expect(#result->colPosition  , 'nu')
            expect(#result->colPosition(), 'nu')
        }
    }
}