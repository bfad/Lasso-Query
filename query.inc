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

define query => type {
    trait { import trait_queriable }
    
    data
        private   query,
        private   query_params,
        public    data_loaded = false,
        public    result_type,
        public    set

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
        .query = inline_type(:.query_params)
        .query->capi->invoke(.query->dsInfo)
        
        .data_loaded = true
        not .set? .set = 1
    }
    
    public set(index::integer) => {
        .set = (#index > 0 ? #index | 1)
        return self
    }
    
    public forEach => {
        not .data_loaded? .loadData
        
        local(currSet) = .query->dsInfo->getset(1)
        local(scope)   = map(
				::currentinline=.query,
				   ::currentset=#currSet?#currSet|(:(:), (:), 0)
        )
		inline_scopePush(#scope)
		//.query->named?
		//	inline_namedPut(.query->named, #scope)
		handle => { inline_scopePop }
        
        //For now, assume one result set
        //loop(.query->dsinfo->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)) => {
        // loop currently breaks iterate (it's a bug)
        local(i) = .query->dsinfo->getSet(.set)->get(INLINE_FOUNDCOUNT_POS)
        while(#i--) => {
            local(ret) = map
            local(row) = .query->dsinfo->getSet(.set)->get(INLINE_RESULTROWS_POS)->get(loop_count)
            iterate(.query->dsinfo->getSet(.set)->get(INLINE_COLUMNINFO_POS)) => {
                loop_value = loop_value->get(INLINE_COLINFO_NAME_POS)->asString
                // Create / insert into an array for columns with the same name
                if(#ret->contains(loop_value)) => {
                    not #ret->find(loop_value)->isA(::array)?
                        #ret->find(loop_value) = array(#ret->find(loop_value))
                    #ret->find(loop_value)->insert(#row->get(loop_count))
                else
                    #ret->insert(loop_value=#row->get(loop_count))
                }
            }
            // If we can't find a type, use the default query_result
            if(not .result_type || .result_type->size < .set || not .result_type->get(.set)) => {
                givenBlock(query_result(#ret))
            else
                givenBlock(\(.result_type->get(.set))(#ret))
            }
        }
    }
}