cmake_minimum_required(VERSION 3.0.0)
project(holod-win64)

add_executable(holod-win64
HOLOD-win64/CALC_Q.C  
HOLOD-win64/CALC_T.C  
HOLOD-win64/GRAFIC.C  
HOLOD-win64/HOL_MATH.C
HOLOD-win64/HOL_PRN.C 
HOLOD-win64/HOL3.C    
HOLOD-win64/TOPLS.C   
)

target_compile_options(holod-win64 PRIVATE "-Wall" "-finput-charset=UTF-8" "-fexec-charset=CP1251")
