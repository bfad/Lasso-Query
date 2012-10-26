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