#include <jni.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <thread>
#include <vector>

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>
#endif

static void negativo_rango(uint32_t *p, int inicio, int fin, bool usar_simd) {
    int i = inicio;

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    if (usar_simd) {
        const uint32x4_t mascara_rgb = vdupq_n_u32(0x00FFFFFFu);
        for (; i + 4 <= fin; i += 4) {
            uint32x4_t pix = vld1q_u32(p + i);
            // XOR con 0x00FFFFFF invierte R, G y B, dejando alfa intacto.
            // Para cada canal de 8 bits: canal_negativo = 255 - canal.
            pix = veorq_u32(pix, mascara_rgb);
            vst1q_u32(p + i, pix);
        }
    }
#endif

    for (; i < fin; i++) {
        p[i] ^= 0x00FFFFFFu;
    }
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_negativosimd_NativeProcessor_negativoARGB(
        JNIEnv *env,
        jclass,
        jintArray pixels,
        jint count,
        jint threads,
        jboolean useSimd) {

    auto ti = std::chrono::high_resolution_clock::now();

    jint *raw = env->GetIntArrayElements(pixels, nullptr);
    if (raw == nullptr || count <= 0) return 0;

    auto *data = reinterpret_cast<uint32_t *>(raw);
    int T = std::max(1, std::min((int) threads, (int) count));
    int bloque = (count + T - 1) / T;

    std::vector<std::thread> hs;
    hs.reserve(T);

    for (int t = 0; t < T; t++) {
        int ini = t * bloque;
        int fin = std::min((int) count, ini + bloque);
        if (ini >= fin) break;
        hs.emplace_back(negativo_rango, data, ini, fin, useSimd == JNI_TRUE);
    }

    for (auto &h: hs) h.join();

    env->ReleaseIntArrayElements(pixels, raw, 0);

    auto tf = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(tf - ti).count();
}
