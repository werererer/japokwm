# Julia wrapper for header: wlr_list.h
# Automatically generated using Clang.jl


function wlr_list_init(list)
    ccall((:wlr_list_init, wlr_list), Bool, (Ptr{wlr_list},), list)
end

function wlr_list_finish(list)
    ccall((:wlr_list_finish, wlr_list), Cvoid, (Ptr{wlr_list},), list)
end

function wlr_list_for_each(list, callback)
    ccall((:wlr_list_for_each, wlr_list), Cvoid, (Ptr{wlr_list}, Ptr{Cvoid}), list, callback)
end

function wlr_list_push()
    ccall((:wlr_list_push, wlr_list), Cint, ())
end

function wlr_list_insert()
    ccall((:wlr_list_insert, wlr_list), Cint, ())
end

function wlr_list_del(list, index)
    ccall((:wlr_list_del, wlr_list), Cvoid, (Ptr{wlr_list}, Csize_t), list, index)
end

function wlr_list_pop(list)
    ccall((:wlr_list_pop, wlr_list), Ptr{Cvoid}, (Ptr{wlr_list},), list)
end

function wlr_list_peek(list)
    ccall((:wlr_list_peek, wlr_list), Ptr{Cvoid}, (Ptr{wlr_list},), list)
end

function wlr_list_cat()
    ccall((:wlr_list_cat, wlr_list), Cint, ())
end

function wlr_list_qsort(list, compare)
    ccall((:wlr_list_qsort, wlr_list), Cvoid, (Ptr{wlr_list}, Cvoid), list, compare)
end

function wlr_list_find()
    ccall((:wlr_list_find, wlr_list), Cint, ())
end
