if(not var(__loaded_query)) => {
    local(path_here) = currentCapture->callsite_file->stripLastComponent
    not #path_here->beginsWith('/')
        ? #path_here = io_file_getcwd + '/' + #path_here
    not #path_here->endsWith('/')
        ? #path_here->append('/')
	
    sourcefile(file(#path_here + '../query.inc'), -autoCollect=false)->invoke
    
    lcapi_loadModule((sys_masterHomePath || sys_homePath || '/var/lasso/home') + '/LassoModules/MySQLConnector.' + sys_dll_ext)
    
    var(__loaded_query) = true
}