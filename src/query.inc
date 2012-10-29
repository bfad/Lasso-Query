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


    public onCreate(datasource::ds_connect, sql::string) => {
        .'source'       = #datasource
        .'query_params' = #sql
    }
    public onCreate(datasource::ds_connect, result_type::string, sql::string) => {
        .'result_type' = (:#result_type)
        .onCreate(#datasource, #sql)
    }
    public onCreate(datasource::ds_connect, result_type::trait_positionallyKeyed, sql::string) => {
        .'result_type' = #result_type
        .onCreate(#datasource, #sql)
    }
    public onCreate(datasource::ds_database, sql::string) => {
        .'source'       = #datasource
        .'query_params' = #sql
    }
    public onCreate(datasource::ds_database, result_type::string, sql::string) => {
        .'result_type' = (:#result_type)
        .onCreate(#datasource, #sql)
    }
    public onCreate(datasource::ds_database, result_type::trait_positionallyKeyed, sql::string) => {
        .'result_type' = #result_type
        .onCreate(#datasource, #sql)
    }
    public onCreate(result_type::trait_positionallyKeyed, ...) => {
        .'result_type' = #result_type
        .onCreate(:#rest)
    }
    public onCreate(result_type::string, ...) => {
        .'result_type'  = (:#result_type)
        .onCreate(:#rest)
    }
    public onCreate(...) => {
        .'query_params' = #rest
        .'source' = query_inline_source
    }

    
    protected loadData => {
        .data_loaded? return
        
        .'query' = .source->getData(.query_params)
        
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
        // Don't seem to need this code
        // local(scope)   = map(
        //         ::currentinline=.source,
        //            ::currentset=#currSet?#currSet|(:(:), (:), 0)
        // )
        // inline_scopePush(#scope)
        // //.source->named?
        // //  inline_namedPut(.source->named, #scope)
        // handle => { inline_scopePop }
        
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