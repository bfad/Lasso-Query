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
        expect($values->find('first') , $qr->first)
        expect($values->find('second'), $qr->second)
        expect($values->find('third') , $qr->third)
    }
    
    it("fails with a 'method not found' error if the method is not in the map") => {
        expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
            $qr->two
        }
    }
    
    it("updates the value for the specified method on assignment") => {
        $qr->third = 'theta'
        
        expect($values->find('first') , $qr->first)
        expect($values->find('second'), $qr->second)
        expect('theta'                , $qr->third)
    }
    
    describe("'c' method with arguments") => {
        it("returns the value for the argument it's passed") => {
            expect($values->find('first') , $qr->c('first'))
            expect($values->find('second'), $qr->c('second'))
            expect($values->find('third') , $qr->c('third'))
        }
        
        it("fails with a 'method not found' error if the argument is not in the map") => {
            expect->error(error_code_methodNotFound, error_msg_methodNotFound) => { 
                $qr->c('two')
            }
        }
        
        it("updates the value for the specified argument on assignment") => {
            $qr->c('third') = 'alpha'
            
            expect($values->find('first') , $qr->c('first'))
            expect($values->find('second'), $qr->c('second'))
            expect('alpha'                , $qr->c('third'))
        }
    }
    
    describe("'c' method without arguments") => {
        it("returns the value for c in the map") => {
            local(result) = query_result((:'c'), (:'rhino'))
            
            expect('rhino', #result->c)
            expect('rhino', #result->c())
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
            expect('beta', #result->c)
            expect('beta', #result->c())
            
            #result->c() = 'gamma'
            expect('gamma', #result->c)
            expect('gamma', #result->c())
        }
    }
    
    describe("'invoke' special method with arguments") => {
        it("returns the value for the argument it's passed") => {
            expect($values->find('first') , $qr->invoke('first'))
            expect($values->find('second'), $qr->invoke('second'))
            expect($values->find('third') , $qr->invoke('third'))
            
            expect($values->find('first') , $qr('first'))
            expect($values->find('second'), $qr('second'))
            expect($values->find('third') , $qr('third'))
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
            
            expect($values->find('first') , $qr->invoke('first'))
            expect($values->find('second'), $qr->invoke('second'))
            expect('delta'                , $qr->invoke('third'))
            
            $qr('third') = 'centauri'
            
            expect($values->find('first') , $qr('first'))
            expect($values->find('second'), $qr('second'))
            expect('centauri'             , $qr('third'))
        }
    }
    
    describe("'invoke' special method without arguments") => {
        it("returns the value for invoke in the map") => {
            local(result) = query_result((:'invoke'), (:'rhino'))
            
            expect('rhino', #result->invoke)
            expect('rhino', #result->invoke())
            expect('rhino', #result())
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
            expect('sigma', #result->invoke)
            expect('sigma', #result->invoke())
            expect('sigma', #result())
            
            #result->invoke() = 'mu'
            expect('mu', #result->invoke)
            expect('mu', #result->invoke())
            expect('mu', #result())
            
            #result() = 'eta'
            expect('eta', #result->invoke)
            expect('eta', #result->invoke())
            expect('eta', #result())
        }
    }
    
    describe("'colPosition' method with arguments") => {
        it("returns the first position in the array for the specified column name") => {
            local(result) = query_result((:'one', 'two', 'two'), (:'foo', 'bar', 'baz'))
            
            expect(1, #result->colPosition('one'))
            expect(2, #result->colPosition('two'))
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
            
            expect('rhino', #result->colPosition)
            expect('rhino', #result->colPosition())
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
            expect('phi', #result->colPosition)
            expect('phi', #result->colPosition())
            
            #result->colPosition() = 'nu'
            expect('nu', #result->colPosition)
            expect('nu', #result->colPosition())
        }
    }
}