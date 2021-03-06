as.exe -march=armv7-a "armv6/bilinearfilter_v6.asm.s" -o "out/bilinearfilter_v6.o"
as.exe -march=armv7-a "armv6/copymem8x4_v6.asm.s" -o "out/copymem8x4_v6.o"
as.exe -march=armv7-a "armv6/copymem8x8_v6.asm.s" -o "out/copymem8x8_v6.o"
as.exe -march=armv7-a "armv6/copymem16x16_v6.asm.s" -o "out/copymem16x16_v6.o"
as.exe -march=armv7-a "armv6/dc_only_idct_add_v6.asm.s" -o "out/dc_only_idct_add_v6.o"
as.exe -march=armv7-a "armv6/dequant_idct_v6.asm.s" -o "out/dequant_idct_v6.o"
as.exe -march=armv7-a "armv6/dequantize_v6.asm.s" -o "out/dequantize_v6.o"
as.exe -march=armv7-a "armv6/filter_v6.asm.s" -o "out/filter_v6.o"
as.exe -march=armv7-a "armv6/idct_v6.asm.s" -o "out/idct_v6.o"
as.exe -march=armv7-a "armv6/intra4x4_predict_v6.asm.s" -o "out/intra4x4_predict_v6.o"
as.exe -march=armv7-a "armv6/iwalsh_v6.asm.s" -o "out/iwalsh_v6.o"
as.exe -march=armv7-a "armv6/loopfilter_v6.asm.s" -o "out/loopfilter_v6.o"
as.exe -march=armv7-a "armv6/simpleloopfilter_v6.asm.s" -o "out/simpleloopfilter_v6.o"
as.exe -march=armv7-a "armv6/sixtappredict8x4_v6.asm.s" -o "out/sixtappredict8x4_v6.o"
as.exe -march=armv7-a "armv6/vp8_sad16x16_armv6.asm.s" -o "out/vp8_sad16x16_armv6.o"
as.exe -march=armv7-a "armv6/vp8_variance_halfpixvar16x16_h_armv6.asm.s" -o "out/vp8_variance_halfpixvar16x16_h_armv6.o"
as.exe -march=armv7-a "armv6/vp8_variance_halfpixvar16x16_hv_armv6.asm.s" -o "out/vp8_variance_halfpixvar16x16_hv_armv6.o"
as.exe -march=armv7-a "armv6/vp8_variance_halfpixvar16x16_v_armv6.asm.s" -o "out/vp8_variance_halfpixvar16x16_v_armv6.o"
as.exe -march=armv7-a "armv6/vp8_variance8x8_armv6.asm.s" -o "out/vp8_variance8x8_armv6.o"
as.exe -march=armv7-a "armv6/vp8_variance16x16_armv6.asm.s" -o "out/vp8_variance16x16_armv6.o"
as.exe -march=armv7-a "armv6/walsh_v6.asm.s" -o "out/walsh_v6.o"
as.exe -march=armv7-a "armv6/vp8_mse16x16_armv6.asm.s" -o "out/vp8_mse16x16_armv6.o"
as.exe -march=armv7-a "armv6/vp8_short_fdct4x4_armv6.asm.s" -o "out/vp8_short_fdct4x4_armv6.o"
ar.exe rc libvpxarmv7_asm.a "out/bilinearfilter_v6.o" "out/copymem8x4_v6.o" "out/copymem8x8_v6.o" "out/copymem16x16_v6.o" "out/dc_only_idct_add_v6.o" "out/dequant_idct_v6.o" "out/dequantize_v6.o" "out/filter_v6.o" "out/idct_v6.o" "out/intra4x4_predict_v6.o" "out/iwalsh_v6.o" "out/loopfilter_v6.o" "out/simpleloopfilter_v6.o" "out/sixtappredict8x4_v6.o" "out/vp8_sad16x16_armv6.o" "out/vp8_variance_halfpixvar16x16_h_armv6.o" "out/vp8_variance_halfpixvar16x16_hv_armv6.o" "out/vp8_variance_halfpixvar16x16_v_armv6.o" "out/vp8_variance8x8_armv6.o" "out/vp8_variance16x16_armv6.o" "out/walsh_v6.o" "out/vp8_mse16x16_armv6.o" "out/vp8_short_fdct4x4_armv6.o" 