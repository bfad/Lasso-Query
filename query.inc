/*
* List of method with no paramters that we won't be able to use:
*   ascopy
*   asstring
*   ascopydeep
*   type
*   trait
*   parent
*   invoke
*   listmethods
*   oncreate
*/
define trait_query_result => trait {
    require __cols::map
    
    provide _unknowntag() => { return .c(method_name->asString) }
    provide _unknowntag(rhs) => {
        local(method_name) = method_name->asString
        fail_if(not #method_name->endsWith('='), error_code_methodNotFound, error_msg_methodNotFound)
        #method_name->remove(#method_name->size)
        
        .c(#method_name) = #rhs
    }
    provide c(col::string) => {
        fail_if(not .__cols->contains(#col), error_code_methodNotFound, error_msg_methodNotFound)
        return .__cols->find(#col)
    }
    provide c=(rhs, col::string) => {
        fail_if(not .__cols->contains(#col), error_code_methodNotFound, error_msg_methodNotFound)
        .__cols->find(#col) = #rhs
    }
    provide c() => .c('c')
    provide c=(rhs::string) => { .c('c') = #rhs }
    
    provide +(s::string) => {
        return .asString + #s
    }
}

define query_result => type {
    trait { import trait_query_result }
    data protected __cols::map
    
    public onCreate() => { .__cols = map }
    public onCreate(cols::map) => {
        .__cols = #cols
    }
}

if(not lasso_tagExists(::inline_type)) => {inline_type}
define inline_type->accessDoClose => .doClose

define query => type {
    trait { import trait_queriable, trait_positionallyKeyed }
    
    data
        private query,
        private query_params,
        private data_loaded = false,
        private result_type,
        private set_cols,
        public  set
    // Public Getters
    public data_loaded => .'data_loaded',
           result_type => .'result_type'


    public onCreate(result_type::trait_array, ...) => {
        .result_type = #result_type
        .onCreate((:#rest))
    }
    public onCreate(result_type::string, ...) => {
        .result_type  = (:#result_type)
        .onCreate((:#rest))
    }
    public onCreate(...) => {
        .query_params = #rest
    }

    
    protected loadData => {
        .data_loaded? return
        
        .query = inline_type(:.query_params)

        .query->capi and .query->accessDoClose?
            handle => { .query->close }
        .query->capi and .query->dsInfo->action == lcapi_datasourcePrepareSQL ?
            handle => { .query->closePrepared }

        .query->capi->invoke(.query->dsInfo)
        
        .'data_loaded' = true
        not .set? .set = 1
        .'set_cols' = array
        loop(.numSets) => { .'set_cols'->insert(void) }
    }
    
    private loadSetCol(set::integer=0) => {
        #set == 0? #set = .set
            
        .'set_cols'->get(#set)? return
        
        .'set_cols'->get(#set) = (
            with col in .query->dsinfo->getSet(#set)->get(INLINE_COLUMNINFO_POS)
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
        
        return .query->dsinfo->numSets
    }

    public size::integer => {
        .loadData

        return .query->dsinfo->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)
    }
    public foundCount::integer => .size
    
    public fieldNames::staticarray => {
        .loadData
        
        .loadSetCol
        
        return .'set_cols'->get(.set)
    }
    public columnNames::staticarray => .fieldNames
    
    public keyName => {
        // code adapted from keyColumn_name
        .loadData
    	local(key_name) = .query->'workingkeyfield_name'
        
        #key_name-> size == 0 ? 
            return '_RECID_'
    	return #key_name
    }
    public keyValue => {
        // code from keyColumn_value
        .loadData
        return .query->dsInfo->forcedRowID
    }

    public keys::staticarray => {
        .loadData

        return (1 to .size)->asStaticArray
    }

    public get(key::integer) => {
        .loadData
        .loadSetCol

        local(ret)  = map
        local(row)  = .query->dsinfo->getSet(.set)->get(INLINE_RESULTROWS_POS)->get(#key)
        local(cols) = .'set_cols'->get(.set)
        local(i)    = 0
        
        // Run through the columns for the row
        while(#row->size > #i) => {
            #i++
            if(#ret->contains(#cols->get(#i))) => {
                not #ret->find(#cols->get(#i))->isA(::array)?
                    #ret->find(#cols->get(#i)) = array(#ret->find(#cols->get(#i)))
                #ret->find(#cols->get(#i))->insert(#row->get(#i))
            else
                #ret->insert(#cols->get(#i) = #row->get(#i))
            }
        }
        
        // If we can't find a type, use the default query_result
        if(not .result_type || .result_type->size < .set || not .result_type->get(.set)) => {
            return query_result(#ret)
        else
            return(\(.result_type->get(.set))(#ret))
        }
    }

    public forEach => {
        .loadData
        
        local(currSet) = .query->dsInfo->getset(.set)
        local(scope)   = map(
                ::currentinline=.query,
                   ::currentset=#currSet?#currSet|(:(:), (:), 0)
        )
        inline_scopePush(#scope)
        //.query->named?
        //  inline_namedPut(.query->named, #scope)
        handle => { inline_scopePop }
        
        // For the current result set, loop through the returned rows
        //loop(.query->dsinfo->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)) => {
        // loop currently breaks iterate (it's a bug)
        local(i) = .size
        while(#i--) => {
            givenBlock(.get(loop_count))
        }
    }
}