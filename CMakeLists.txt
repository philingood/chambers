cmake_minimum_required(VERSION 3.0.0)
project(holod)

add_executable(holod
src/CALC_Q.C
src/CALC_T.C
src/GRAFIC.C
src/HOL_MATH.C
src/HOL_PRN.C
src/HOL3.C
src/TOPLS.C
)

# target_compile_options(holod PRIVATE "-Wall" "-finput-charset=UTF-8" "-fexec-charset=CP1251")
target_compile_options(holod PRIVATE "-Wall" "-finput-charset=UTF-8")
