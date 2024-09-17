cmake_minimum_required(VERSION 3.5.0)
project(holod)
set(CMAKE_CXX_STANDARD 11)  # или более позднюю версию стандарта

add_executable(holod
src/calc_q.c
src/calc_t.c
src/grafic.c
src/hol_math.c
src/hol_prn.c
src/hol3.c
src/topls.c
)

# target_compile_options(holod PRIVATE "-Wall" "-finput-charset=UTF-8" "-fexec-charset=CP1251")
target_compile_options(holod PRIVATE "-Wall" "-finput-charset=UTF-8")
