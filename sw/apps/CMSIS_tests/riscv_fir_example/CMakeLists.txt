set(RISCV_FIR_EXAMPLE math_helper.c riscv_fir_data.c riscv_fir_example_f32.c )
add_application(riscv_fir_example_f32 "${RISCV_FIR_EXAMPLE}")
