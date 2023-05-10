cmake_minimum_required(VERSION 3.0.0)
project(holod)

add_executable(holod
HOLOD/CALC_Q.C  
HOLOD/CALC_T.C  
HOLOD/GRAFIC.C  
HOLOD/HOL_MATH.C
HOLOD/HOL_PRN.C 
HOLOD/HOL3.C    
HOLOD/TOPLS.C   
)
