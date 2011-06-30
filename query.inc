define trait_query_result => trait {
    require __cols::map
    
    provide _unknowntag() => {
        fail_if(not .__cols->contains(method_name->asString), error_code_methodNotFound, error_msg_methodNotFound)
        
        return .__cols->find(method_name->asString)
    }
    provide _unknowntag(rhs) => {
        local(method_name) = method_name->asString
        fail_if(not #method_name->endsWith('='), error_code_methodNotFound, error_msg_methodNotFound)
        #method_name->remove(#method_name->size)
        fail_if(not .__cols->contains(#method_name), error_code_methodNotFound, error_msg_methodNotFound)
        
        .__cols->find(#method_name) = #rhs
    }
    
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
        public    data_loaded,
        public    col_names,
        public    result_type

    public onCreate(result_type::string, ...) => {
        .query       = inline_type(:#rest)
        .data_loaded = false
        .result_type = #result_type
    }
    public onCreate(...) => {
        .onCreate('query_result', (:#rest))
    }
    
    protected load_data => {
        .query->capi->invoke(.query->dsInfo)
        .col_names = (
            with col_info in .query->dsinfo->getSet(1)->get(INLINE_COLUMNINFO_POS) 
            select #col_info->get(INLINE_COLINFO_NAME_POS)->asString
        )->asStaticArray
        
        .data_loaded = true
    }
    
    public forEach => {
        not .data_loaded? .load_data
        
        local(gb) = givenBlock
        //For now, assume one result set
        //loop(.query->dsinfo->getSet(1)->get(INLINE_FOUNDCOUNT_POS)) => {
        // loop currently breaks iterate (it's a bug)
        local(i) = .query->dsinfo->getSet(1)->get(INLINE_FOUNDCOUNT_POS)
        while(#i--) => {
            local(ret) = map
            local(row) = .query->dsinfo->getSet(1)->get(INLINE_RESULTROWS_POS)->get(loop_count)
            iterate(.col_names) => {
                // Create / insert into an array for columns with the same name
                if(#ret->contains(loop_value)) => {
                    not #ret->find(loop_value)->isA(::array)?
                        #ret->find(loop_value) = array(#ret->find(loop_value))
                    #ret->find(loop_value)->insert(#row->get(loop_count))
                else
                    #ret->insert(loop_value=#row->get(loop_count))
                }
            }
            #gb->invoke(\(.result_type)(#ret))
        }
    }
}