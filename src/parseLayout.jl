include("coreUtils.jl")
using DelimitedFiles

const layoutDir = "layouts/"

#=
# Containerfiles format:
# ===
# 0.4 1.0 0.2 0.4
# ===
# where
# 0.4 = x
# 1.0 = y
# 0.4 = width
# 0.4 = height
=#
function readContainers(file :: String) :: Array{Container}
    res = []
    if isfile(file)
        arr = readdlm(file, ' ', Float64)
        if size(arr)[2] != 4
            println("Wrong size of array in: $file")
        end
        for i in 1:size(arr)[1]
            push!(res, Container(arr[i,1], arr[i,2], arr[i,3], arr[i,4]))
        end
    else
        println("Error: $file is not a file or doesn't exist")
    end
    return res
end

#=
# Layouts are stored like this:
# ${layoutsDirectroy}/${layoutName}/${Containers}
# where ${Containers} name is corresponding to the number of Clients that are
# visible.
=#
function getLayout(layout :: String) :: Array{Array{Container, 1}, 1}
    layoutPath = joinpath(layoutDir, layout)
    res = []
    i = 1
    if isdir(layoutPath)
        for file in readdir(layoutPath)
            filePath = joinpath(layoutPath, string(i))
            push!(res, readContainers(filePath))
            i += 1
        end
    else
        println("Error: $layoutPath is not a directory or doesn't exist")
    end
    return res
end
