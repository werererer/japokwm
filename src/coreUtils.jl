# from 0-1
struct Container
    x :: Cdouble
    y :: Cdouble
    width :: Cdouble
    height :: Cdouble
end

mutable struct Monitor
    m :: Container # monitor area
    w :: Container # window area
    tagset :: Int
    mfact :: Float64
    nmaster :: Int
end

struct cContainerArr
    container :: Ptr{Container}
    size :: Cint
end

