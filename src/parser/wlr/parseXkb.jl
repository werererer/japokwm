using Clang

const outDir = "out"
files = [
         "/usr/include/X11/Xlib.h",
       ]

outFile = "out/outfile.jl"
mkpath(dirname(outFile))

#compile
wc = init(; headers = files,
          output_file = outFile,
          clang_args = ["-DWLR_USE_UNSTABLE", "-I/usr/lib/gcc/x86_64-pc-linux-gnu/10.2.0/include", "-I/usr/include/libdrm", "-I/usr/include/pixman-1", "xdg-shell-protocol.o"],
          header_wrapped = (root, current)->current == root,
         )
run(wc)
