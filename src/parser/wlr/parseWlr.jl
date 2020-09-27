using Clang

const outDir = "../../wlr"
wlr = "/usr/include/wlr"
for (root, dirs, files) in walkdir(wlr)
    for file in files
        outFile = replace("$root/$file", ".h" => ".jl") |> x -> replace(x, wlr => ".")
        outFile = joinpath("out", outFile) |> normpath
        mkpath(dirname(outFile))

        println("original: $root/$file")
        println("create:   $outFile")

        #compile
        wc = init(; headers = ["$root/$file"],
                  output_file = outFile,
                  clang_args = ["-DWLR_USE_UNSTABLE", "-I/usr/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include", "-I/usr/include/libdrm", "-I/usr/include/pixman-1"],
                  header_wrapped = (root, current)->current == root,
                 )
        run(wc)
   end
end
