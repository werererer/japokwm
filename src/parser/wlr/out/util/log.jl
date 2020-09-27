# Julia wrapper for header: log.h
# Automatically generated using Clang.jl


function wlr_log_init(verbosity, callback)
    ccall((:wlr_log_init, log), Cvoid, (wlr_log_importance, wlr_log_func_t), verbosity, callback)
end

function wlr_log_get_verbosity()
    ccall((:wlr_log_get_verbosity, log), wlr_log_importance, ())
end
efinition: wlr_log_errno ( verb , fmt , ... ) wlr_log ( verb , fmt ": %s" , ## __VA_ARGS__ , strerror ( errno ) )

@cenum wlr_log_importance::UInt32 begin
    WLR_SILENT = 0
    WLR_ERROR = 1
    WLR_INFO = 2
    WLR_DEBUG = 3
    WLR_LOG_IMPORTANCE_LAST = 4
end


const wlr_log_func_t = Ptr{Cvoid}
