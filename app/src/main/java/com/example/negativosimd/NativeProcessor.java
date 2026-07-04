package com.example.negativosimd;

public final class NativeProcessor {
    static {
        System.loadLibrary("negativosimd");
    }

    private NativeProcessor() {}

    // Devuelve el tiempo nativo de procesamiento en nanosegundos.
    public static native long negativoARGB(int[] pixels, int count, int threads, boolean useSimd);
}
