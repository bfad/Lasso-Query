if(not var(__loaded_query)) => {
    local(path_here) = currentCapture->callsite_file->stripLastComponent
    not #path_here->beginsWith('/')
        ? #path_here = io_file_getcwd + '/' + #path_here
    not #path_here->endsWith('/')
        ? #path_here->append('/')
	
    sourcefile(file(#path_here + '../query.inc'), -autoCollect=false)->invoke
    
    lcapi_loadModule((sys_masterHomePath || sys_homePath || '/var/lasso/home') + '/LassoModules/MySQLConnector.' + sys_dll_ext)
    
    define tmp_query_result1 => type {
        parent query_result
            
        public onCreate(p0,p1) => {
            ..onCreate(#p0, #p1)
        }
    }
    define tmp_query_result2 => type {
        parent query_result
            
        public onCreate(p0,p1) => {
            ..onCreate(#p0, #p1)
        }
    }
    
    var(__loaded_query) = true
}