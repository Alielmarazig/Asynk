#pragma once

// MSVC does not define M_PI unless this is set
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <vector>
#include <complex>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <algorithm>
#include <numeric>

namespace asynk {

using Complex = std::complex<double>;

/**
 * Radix-2 Cooley-Tukey FFT. Input size must be power of 2.
 * In-place computation.
 */
inline void fft(std::vector<Complex>& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly operations
    for (size_t len = 2; len <= N; len <<= 1) {
        double angle = -2.0 * M_PI / static_cast<double>(len);
        Complex wlen(std::cos(angle), std::sin(angle));
        for (size_t i = 0; i < N; i += len) {
            Complex w(1.0, 0.0);
            for (size_t j = 0; j < len / 2; ++j) {
                Complex u = x[i + j];
                Complex v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}

/**
 * Inverse FFT.
 */
inline void ifft(std::vector<Complex>& x) {
    for (auto& c : x) c = std::conj(c);
    fft(x);
    for (auto& c : x) {
        c = std::conj(c);
        c /= static_cast<double>(x.size());
    }
}

/**
 * Next power of 2 >= n.
 */
inline size_t nextPow2(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

/**
 * FFT-based cross-correlation of two real signals.
 * Returns correlation array of length len(a) + len(b) - 1.
 * Peak index gives the lag; peak magnitude gives confidence.
 */
inline std::vector<double> crossCorrelate(
    const std::vector<float>& a,
    const std::vector<float>& b)
{
    size_t resultLen = a.size() + b.size() - 1;
    size_t N = nextPow2(resultLen);

    std::vector<Complex> fa(N, {0.0, 0.0});
    std::vector<Complex> fb(N, {0.0, 0.0});

    for (size_t i = 0; i < a.size(); ++i)
        fa[i] = Complex(a[i], 0.0);
    for (size_t i = 0; i < b.size(); ++i)
        fb[i] = Complex(b[i], 0.0);

    fft(fa);
    fft(fb);

    // Multiply fa by conj(fb)
    for (size_t i = 0; i < N; ++i)
        fa[i] *= std::conj(fb[i]);

    ifft(fa);

    std::vector<double> result(resultLen);
    for (size_t i = 0; i < resultLen; ++i)
        result[i] = fa[i].real();

    return result;
}

} // namespace asynk
