if(not lasso_tagExists(::trait_positionallyKeyed)) => {trait_positionallyKeyed}
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
    provide colPosition=(rhs) => {.c('colPosition') = #rhs}
    
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
        .__data->get(.colPosition(#col)) = #rhs
    }
    provide c() => .c('c')
    provide c=(rhs::string) => { .c('c') = #rhs }
    
    // A bit of syntactic sugar that allows for #row('foo') instead of #row->c('foo')
    provide invoke(col::string) => .c(#col)
    provide invoke=(rhs, col::string) => { .c(#col) = #rhs }
    provide invoke() => .c('invoke')
    provide invoke=(rhs::string) => { .c('invoke') = #rhs }
    
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

protect => {\inline_type}
define inline_type->accessDoClose => .doClose
define query_inline_source => type {parent inline_type
    public getData(...) => {
        ..reset(:#rest)

        ..capi and ..accessDoClose?
            handle => { ..close }
        ..capi and ..dsInfo->action == lcapi_datasourcePrepareSQL ?
            handle => { ..closePrepared }

        local(code, msg)
        protect => {
            handle_error => {
                #code = error_code
                #msg  = error_msg
    		    var(__app_profiler__)?
                    $__app_profiler__->inlineEnd
            }
            var(__app_profiler__)?
			    $__app_profiler__->inlineBegin(.dsInfo)
            
            ..capi->invoke(..dsInfo)

            var(__app_profiler__)?
			    $__app_profiler__->inlineEnd
        }
		void != #code?
			error_code = #code
        void != #msg?
			error_msg = #msg
        
        return ..dsInfo
    }
}

define query => type {
    trait { import trait_queriable, trait_positionallyKeyed }
    
    data
        private query,
        private query_params,
        private source,
        private data_loaded = false,
        private result_type,
        private set_cols,
        public  set
    // Public Getters
    public data_loaded => .'data_loaded',
           result_type => .'result_type'


    public onCreate(result_type::trait_array, ...) => {
        .'result_type' = #result_type
        .onCreate((:#rest))
    }
    public onCreate(result_type::string, ...) => {
        .'result_type'  = (:#result_type)
        .onCreate((:#rest))
    }
    public onCreate(...) => {
        .'query_params' = #rest
        .'source' = query_inline_source
    }

    
    protected loadData => {
        .data_loaded? return
        
        .'query' = .source->getData(:.query_params)
        
        .'data_loaded' = true
        not .set? .set = 1
        .'set_cols' = array
        loop(.numSets) => { .'set_cols'->insert(void) }
    }
    
    private loadSetCol(set::integer=0) => {
        #set == 0? #set = .set
            
        .'set_cols'->get(#set)? return
        
        .'set_cols'->get(#set) = (
            with col in .query->getSet(#set)->get(INLINE_COLUMNINFO_POS)
            select #col->get(INLINE_COLINFO_NAME_POS)
        )->asStaticarray
    }

    public run => .loadData
    
    public set(index::integer) => {
        .set = (#index > 0 ? #index | 1)
        return self
    }
    
    public numSets::integer => {
        .loadData
        
        return .query->numSets
    }

    public size::integer => {
        .loadData

        return .query->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)
    }
    public foundCount::integer => .size
    
    public fieldNames::staticarray => {
        .loadData
        
        .loadSetCol
        
        return .'set_cols'->get(.set)
    }
    public columnNames::staticarray => .fieldNames
    
    public keyValue => {
        // code from keyColumn_value
        .loadData
        return .query->forcedRowID
    }

    public keys::staticarray => {
        .loadData

        return (1 to .size)->asStaticArray
    }
    
    public rowsArrayMap => {
        .loadData
        .loadSetCol
        
        local(
            rows    = .query->getSet(.set)->get(INLINE_RESULTROWS_POS),
            cols    = .'set_cols'->get(.set),
            output  = array,
            col_map = map,
            i       = 0
        )
        
        while(#i < #rows->size) => {
            #i++
            local(tmp) = map
            loop(#cols->size) => {
                #tmp->insert(#cols->get(loop_count) = #rows->get(#i)->get(loop_count))
            }
            #output->insert(#tmp)
        }
        
    	return #output
    }

    public get(key::integer) => {
        .loadData
        .loadSetCol

        local(row)  = .query->getSet(.set)->get(INLINE_RESULTROWS_POS)->get(#key)
        local(cols) = .'set_cols'->get(.set)
        
        // If we can't find a type, use the default query_result
        if(not .result_type || .result_type->size < .set || not .result_type->get(.set)) => {
            return query_result(#cols, #row)
        else
            return(\(.result_type->get(.set))(#cols, #row))
        }
    }

    public forEach => {
        .loadData
        
        local(currSet) = .query->getset(.set)
        local(scope)   = map(
                ::currentinline=.source,
                   ::currentset=#currSet?#currSet|(:(:), (:), 0)
        )
        inline_scopePush(#scope)
        //.source->named?
        //  inline_namedPut(.source->named, #scope)
        handle => { inline_scopePop }
        
        // For the current result set, loop through the returned rows
        //loop(.query->dsInfo->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)) => {
        // loop currently breaks iterate (it's a bug)
        // (I've also found this to be faster than even a plain capture block with currentCapture->restart)
        local(i) = .size
        while(#i--) => {
            givenBlock(.get(loop_count))
        }
    }
}