protect => {\trait_positionallyKeyed}
define trait_positionallyKeyed->findFirst(val) => { 
    local(i) = 1
    {
        #i > .size? return void
        .get(#i) == #val ? return #i
        #i++
        currentCapture->restart
    }()
}

define trait_query_result => trait {
    require __cols::trait_positionallyKeyed
    require __data::trait_positionallyKeyed
    
    // While overriding colPosition() is fine, do not override colPosition(::string)
    provide colPosition(col::string) => {
        local(position) = .__cols->findFirst(#col)
        fail_if(void == #position, error_code_methodNotFound, error_msg_methodNotFound)
        
        return #position
    }
    provide colPosition() => .c('colPosition')
    provide colPosition=(rhs) => {return .c('colPosition') := #rhs}
    
    provide _unknowntag() => { return .c(method_name->asString) }
    provide _unknowntag(rhs) => {
        local(method_name) = method_name->asString
        fail_if(not #method_name->endsWith('='), error_code_methodNotFound, error_msg_methodNotFound)
        #method_name->remove(#method_name->size)
        
        .c(#method_name) = #rhs
    }
    
    provide c(col::string) => {
        return .__data->get(.colPosition(#col))
    }
    provide c=(rhs, col::string) => {
        return .__data->get(.colPosition(#col)) := #rhs
    }
    provide c() => .c('c')
    provide c=(rhs::string) => { return .c('c') := #rhs }
    
    // A bit of syntactic sugar that allows for #row('foo') instead of #row->c('foo')
    provide invoke(col::string) => .c(#col)
    provide invoke=(rhs, col::string) => { return .c(#col) := #rhs }
    provide invoke() => .c('invoke')
    provide invoke=(rhs::string) => { return .c('invoke') := #rhs }
    
    provide +(s::string) => {
        return .asString + #s
    }
}

define query_result => type {
    trait { import trait_query_result }
    data protected __cols,
         protected __data
    
    public onCreate() => { .__cols = (:); .__data = (:) }
    public onCreate(cols::trait_positionallyKeyed, data::trait_positionallyKeyed) => {
        .__cols = #cols
        .__data = #data
    }
}