#include <argon2.h>
#include "../src/bitcoin/crypto/blake3/blake3.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// ---- Minimal standalone SHA3-256 (Keccak FIPS 202) ----
// Based on tiny-keccak by David Leon Gil (CC0 license)
namespace {
static const uint64_t keccakRC[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL, 0x8000000080008000ULL,
    0x000000000000808bULL, 0x0000000080000001ULL, 0x8000000080008081ULL, 0x8000000000008009ULL,
    0x000000000000008aULL, 0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL, 0x8000000000008003ULL,
    0x8000000000008002ULL, 0x8000000000000080ULL, 0x000000000000800aULL, 0x800000008000000aULL,
    0x8000000080008081ULL, 0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};
static const uint8_t keccakRho[24] = {
     1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
    27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44
};
static const uint8_t keccakPi[24] = {
    10,  7, 11, 17, 18,  3,  5, 16,  8, 21, 24,  4,
    15, 23, 19, 13, 12,  2, 20, 14, 22,  9,  6,  1
};

inline uint64_t rotl64(uint64_t x, int s) { return (x << s) | (x >> (64 - s)); }

void keccakf(uint64_t st[25]) {
    uint64_t b[5], t;
    for (int i = 0; i < 24; ++i) {
        // Theta
        for (int x = 0; x < 5; ++x)
            b[x] = st[x] ^ st[x+5] ^ st[x+10] ^ st[x+15] ^ st[x+20];
        for (int x = 0; x < 5; ++x) {
            t = b[(x+4)%5] ^ rotl64(b[(x+1)%5], 1);
            for (int y = 0; y < 25; y += 5) st[y+x] ^= t;
        }
        // Rho and Pi
        t = st[1];
        for (int x = 0; x < 24; ++x) {
            b[0] = st[keccakPi[x]];
            st[keccakPi[x]] = rotl64(t, keccakRho[x]);
            t = b[0];
        }
        // Chi
        for (int y = 0; y < 25; y += 5) {
            for (int x = 0; x < 5; ++x) b[x] = st[y+x];
            for (int x = 0; x < 5; ++x)
                st[y+x] = b[x] ^ ((~b[(x+1)%5]) & b[(x+2)%5]);
        }
        // Iota
        st[0] ^= keccakRC[i];
    }
}

// General-purpose SHA3-256: works for any input length
void sha3_256(uint8_t output[32], const uint8_t* input, size_t inlen) {
    // SHA3-256: rate = 136 bytes, capacity = 64 bytes
    constexpr size_t rate = 136;
    uint64_t st[25] = {};

    // Absorb full blocks
    size_t offset = 0;
    while (offset + rate <= inlen) {
        for (size_t i = 0; i < rate / 8; ++i) {
            uint64_t v;
            std::memcpy(&v, input + offset + i * 8, 8);
            st[i] ^= v;
        }
        keccakf(st);
        offset += rate;
    }

    // Absorb remaining bytes + padding into a temporary buffer
    uint8_t buf[rate] = {};
    size_t remaining = inlen - offset;
    std::memcpy(buf, input + offset, remaining);
    buf[remaining] = 0x06;       // SHA3 domain separator
    buf[rate - 1] |= 0x80;      // final bit of pad10*1

    for (size_t i = 0; i < rate / 8; ++i) {
        uint64_t v;
        std::memcpy(&v, buf + i * 8, 8);
        st[i] ^= v;
    }
    keccakf(st);

    // Squeeze first 32 bytes
    std::memcpy(output, st, 32);
}
} // namespace
// ---- End SHA3-256 ----

namespace {

std::string toHex(const uint8_t* data, size_t len) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (size_t i = 0; i < len; ++i) {
        out << std::setw(2) << static_cast<unsigned>(data[i]);
    }
    return out.str();
}

int hexDigit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

std::vector<uint8_t> parseHex(const std::string& hex) {
    if (hex.size() % 2 != 0) {
        throw std::runtime_error("hex string must have even length");
    }
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        const int hi = hexDigit(hex[i]);
        const int lo = hexDigit(hex[i + 1]);
        if (hi < 0 || lo < 0) {
            throw std::runtime_error("invalid hex character");
        }
        bytes.push_back(static_cast<uint8_t>((hi << 4) | lo));
    }
    return bytes;
}

void printUsage(const char* argv0) {
    std::cerr << "Usage: " << argv0 << " <80-byte-block-header-hex>\n";
    std::cerr << "Example: " << argv0 << " 0000... (160 hex chars)\n";
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 2) {
            printUsage(argv[0]);
            return 1;
        }

        const std::string headerHex = argv[1];
        auto input = parseHex(headerHex);
        if (input.size() != 80) {
            throw std::runtime_error("header must be exactly 80 bytes (160 hex chars)");
        }

        uint8_t blake3Out[32];
        blake3_hasher hasher;
        blake3_hasher_init(&hasher);
        blake3_hasher_update(&hasher, input.data(), input.size());
        blake3_hasher_finalize(&hasher, blake3Out, sizeof(blake3Out));

        static constexpr char saltStr[] = "RinCoinSalt";
        uint8_t argon2Out[32];
        argon2_context context = {};
        context.out = argon2Out;
        context.outlen = sizeof(argon2Out);
        context.pwd = blake3Out;
        context.pwdlen = sizeof(blake3Out);
        context.salt = reinterpret_cast<uint8_t*>(const_cast<char*>(saltStr));
        context.saltlen = std::strlen(saltStr);
        context.t_cost = 2;
        context.m_cost = 64;
        context.lanes = 1;
        context.threads = 1;
        context.version = ARGON2_VERSION_13;
        context.allocate_cbk = nullptr;
        context.free_cbk = nullptr;
        context.flags = ARGON2_DEFAULT_FLAGS;

        if (argon2d_ctx(&context) != ARGON2_OK) {
            throw std::runtime_error("Argon2d hashing failed");
        }

        uint8_t sha3Out[32];
        sha3_256(sha3Out, argon2Out, sizeof(argon2Out));

        std::cout << "input_header_hex=" << headerHex << '\n';
        std::cout << "step1_blake3=" << toHex(blake3Out, sizeof(blake3Out)) << '\n';
        std::cout << "step2_argon2d_v13=" << toHex(argon2Out, sizeof(argon2Out)) << '\n';
        std::cout << "step3_sha3_256=" << toHex(sha3Out, sizeof(sha3Out)) << '\n';

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
