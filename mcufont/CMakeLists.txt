cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(mcufont 
src/mf_encoding.c 
src/mf_font.c 
src/mf_justify.c 
src/mf_kerning.c 
src/mf_rlefont.c 
src/mf_bwfont.c 
src/mf_scaledfont.c 
src/mf_wordwrap.c 

src/DejaVuSans12.c 
src/DejaVuSans12bw_bwfont.c 
src/DejaVuSans12bw.c 
src/DejaVuSerif16.c 
src/DejaVuSerif32.c 
src/fixed_5x8.c 
src/fixed_7x14.c 
src/fixed_10x20.c
)

target_include_directories(mcufont PUBLIC "${PROJECT_SOURCE_DIR}/mcufont/include")


#./decoder/mf_encoding.c 
#./decoder/mf_font.c 
#./decoder/mf_justify.c 
#./decoder/mf_kerning.c 
#./decoder/mf_rlefont.c 
#./decoder/mf_bwfont.c 
#./decoder/mf_scaledfont.c 
#./decoder/mf_wordwrap.c 

#./fonts/DejaVuSans12.c 
#./fonts/DejaVuSans12bw_bwfont.c 
#./fonts/DejaVuSans12bw.c 
#./fonts/DejaVuSerif16.c 
#./fonts/DejaVuSerif32.c 
#./fonts/fixed_5x8.c 
#./fonts/fixed_7x14.c 
#./fonts/fixed_10x20.c