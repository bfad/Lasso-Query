local(path_here) = currentCapture->callsite_file->stripLastComponent
not #path_here->beginsWith('/')
    ? #path_here = io_file_getcwd + '/' + #path_here
not #path_here->endsWith('/')
    ? #path_here->append('/')
sourcefile(file(#path_here + 'spec_helper.inc'), -autoCollect=false)->invoke


local(conn) = array(
    -datasource='mysqlds', 
          -name='localhost', 
          -port='3306', 
      -username='root', 
      -password=''
)
var(has_connection) = false
describe("This test file") => {
    it("connects to specified MySQL server") => {
        local(conn_id)
        
        expect->error(error_code_noerror, error_msg_noerror) => {
            inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
                #conn_id = field('con_id')
            }
            $has_connection = (error_code == error_code_noerror)
        }
        expect(error_code_noerror, error_code)
        expect(boolean(#conn_id->isA(::integer)))
    }
}
fail_if(not $has_connection, "Can not connect to the database")

describe(::query) => {
    it("correctly alerts you to SQL errors") => {
        expect->error(1064, `You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'SELECTD 'a'' at line 1`) => {
            query(-host=#conn, -sql="SELECTD 'a'")->run
        }
        expect->error(1064, `You have an error in your SQL syntax; check the manual that corresponds to your MySQL server version for the right syntax to use near 'SELECTD 'a'' at line 1`) => {
            with row in query(-host=#conn, -sql="SELECTD 'a'") do {#row}
        }
        
        // This not only checks for errors, but makes sure that it doesn't
        // behave like inline where the errors don't stop execution. If
        // something goes wrong, we want it to blow up.
        expect(error_code_noerror, error_code)
        local(did_raise_fail) = false
        handle_error => {
            #did_raise_fail = true
            error_reset
        }
        query(-host=#conn, -sql="SELECTD 'b'")->run
        // If all runs correctly, I probably won't even get to here
        expect(#did_raise_fail)
    }
    
    it("shares the connection when declared and executed inside an inline") => {
        local(conn_id, inline_conn_id)
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            query(-sql="SELECT CONNECTION_ID() AS con_id")->forEach => {
                #conn_id = #1->con_id
            }
        }
        
        expect(error_code_noerror, error_code)
        expect(#inline_conn_id, #conn_id)
    }

    it("shares the connection when executed inside an inline though declared outside of it") => {
        local(conn_id, inline_conn_id)
        local(query) = query(-sql="SELECT CONNECTION_ID() AS con_id")
        
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#inline_conn_id, #conn_id)
    }
    
    it("doesn't share the connection when executed outside an inline though declared inside of it") => {
        local(conn_id, inline_conn_id)
        
        inline(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #inline_conn_id = field('con_id')
            local(query)    = query(-sql="SELECT CONNECTION_ID() AS con_id")
        }
        
        expect(not #query->data_loaded)
        expect->error => {
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
    }

    it("shares the connection when declared and executed inside another [query]") => {
        local(conn_id, outer_conn_id)
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = #1->con_id
            query(-sql="SELECT CONNECTION_ID() AS con_id")->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#outer_conn_id, #conn_id)
    }
    
    it("shares the connection when executed inside another [query] though declared outside of it") => {
        local(conn_id, outer_conn_id)
        local(query) = query(-sql="SELECT CONNECTION_ID() AS con_id")
        
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = #1->con_id
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
        expect(error_code_noerror, error_code)
        expect(#outer_conn_id, #conn_id)
    }
    
    it("doesn't share the connection when executed outside another [query] though declared inside of it") => {
        local(conn_id, outer_conn_id)
        
        query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id") => {
            #outer_conn_id = #1->con_id
            local(query)    = query(-sql="SELECT CONNECTION_ID() AS con_id")
        }
        
        expect(not #query->data_loaded)
        expect->error => {
            #query->forEach => {
                #conn_id = #1->con_id
            }
        }
    }
    
    it("returns a [trait_queriable] whose each iteration is a [query_result]") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id")
        
        expect(#query->isA(::trait_queriable) != 0)
        
        with row in #query do => {
            expect(#row->isA(::query_result) != 0)
        }
    }
    
    it("returns a [trait_queriable] whose each iteration is of type passed as first argument") => {
        local(query) = query('tmp_query_result1', -host=#conn, -sql="SELECT CONNECTION_ID() AS con_id")
        
        expect(#query->isA(::trait_queriable) != 0)

        with row in #query do => {
            expect(#row->type, 'tmp_query_result1')
        }
    }

    it("returns the first value found when multiple columns share the same name") => {
        local(query) = query(-host=#conn, -sql="SELECT 'foo' AS con_id, CONNECTION_ID() AS con_id")
        
        with row in #query do => {
            expect(#row->isA(::query_result)  != 0)
            expect('foo', #row->con_id)
        }
    }
    
    it("allows for using classic lasso inline syntax") => {
        local(query) = query(-host=#conn, -database='information_schema', -table='CHARACTER_SETS', -search, -operator='bw', 'CHARACTER_SET_NAME'='l')
        
        expect(#query->size > 0)
    }

    it("implements first method") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id, 'foo' AS con_id")

        expect(#query->first->isA(::query_result) != 0)
    }
    
    it("has [query->fieldNames] return an array of the field names") => {
        local(query)  = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id, VERSION()")
        
        expect(#query->fieldNames->isA(::staticarray) != 0)
        expect(#query->fieldNames->get(1) == 'con_id')
        expect(#query->fieldNames->get(2) == 'VERSION()')
    }
    
    it("has [query->columnNames] return an array of the field names") => {
        local(query)  = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id, VERSION()")
        
        expect(#query->columnNames->isA(::staticarray) != 0)
        expect(#query->columnNames->get(1) == 'con_id')
        expect(#query->columnNames->get(2) == 'VERSION()')
    }
    
    it("has [query->foundCount] return the total number of rows found") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id UNION ALL SELECT 8", -maxRecords=1)

        expect(2, #query->foundCount)
    }
		
		it("has [query->size] return just the number of rows returned") => {
				local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id UNION ALL SELECT 8", -maxRecords=1)

				expect(1, #query->size)
		}
		it("has [query->shownCount] return just the number of rows returned") => {
				local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id UNION ALL SELECT 8", -maxRecords=1)

				expect(1, #query->shownCount)
		}
    
    it("returns the number of sets when [query->numSets] is called") => {
        local(query) = query(-host=#conn, -sql="SELECT CONNECTION_ID() AS con_id;SELECT VERSION();")
            
        expect(2, #query->numSets)
    }
    
    describe("Method rowsArrayMap") => {
        it("returns an array, each element of which is a map with the column names as keys and values as the data for that row") => {
            local(query)  = query(-host=#conn, -sql="SELECT 'a' AS first, 'b' AS second UNION ALL SELECT 'c' AS first, 'd' AS second")
            local(result) = #query->rowsArrayMap
            
            expect('array', #result->type)
            expect(2, #result->size)
            expect('a', #result->get(1)->find('first'))
            expect('b', #result->get(1)->find('second'))
            expect('c', #result->get(2)->find('first'))
            expect('d', #result->get(2)->find('second'))
        }
    }
    
    describe("-> dataRows") => {
        it("returns a staticarray of staticarrays - each internal array being a row of data") => {
            local(query)  = query(-host=#conn, -sql="SELECT 'a' AS first, 'b' AS second UNION ALL SELECT 'c', 'd'")
            local(result) = #query->dataRows
            
            expect('staticarray', #result->type)
            expect('staticarray', #result->get(1)->type)
            expect('staticarray', #result->get(2)->type)
            
            expect('a', #result->get(1)->get(1))
            expect('b', #result->get(1)->get(2))
            expect('c', #result->get(2)->get(1))
            expect('d', #result->get(2)->get(2))
        }
    }
}