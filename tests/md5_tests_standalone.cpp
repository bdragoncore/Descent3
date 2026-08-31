/*
 * D3 Coverage Tests - md5 module (Standalone)
 * 
 * Tests for MD5 hashing implementation.
 * Uses known MD5 test vectors for validation.
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdint>

// ============================================================================
// MD5 Implementation - RFC 1321
// ============================================================================

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    unsigned char buffer[64];
} MD5_CTX;

static const unsigned char PADDING[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define F(x, y, z) ((x & y) | (~x & z))
#define G(x, y, z) ((x & z) | (y & ~z))
#define H(x, y, z) (x ^ y ^ z)
#define I(x, y, z) (y ^ (x | ~z))

#define ROTATE_LEFT(x, n) ((x << n) | (x >> (32 - n)))

static void MD5Init(MD5_CTX* ctx) {
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    ctx->count[0] = ctx->count[1] = 0;
}

static void MD5Transform(uint32_t state[4], const uint32_t block[16]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    
    a = ROTATE_LEFT(a + F(b,c,d) + block[0] + 0xd76aa478, 7) + b;
    d = ROTATE_LEFT(d + F(a,b,c) + block[1] + 0xe8c7b756, 12) + a;
    c = ROTATE_LEFT(c + F(d,a,b) + block[2] + 0x242070db, 17) + d;
    b = ROTATE_LEFT(b + F(c,d,a) + block[3] + 0xc1bdceee, 22) + c;
    a = ROTATE_LEFT(a + F(b,c,d) + block[4] + 0xf57c0faf, 7) + b;
    d = ROTATE_LEFT(d + F(a,b,c) + block[5] + 0x4787c62a, 12) + a;
    c = ROTATE_LEFT(c + F(d,a,b) + block[6] + 0xa8304613, 17) + d;
    b = ROTATE_LEFT(b + F(c,d,a) + block[7] + 0xfd469501, 22) + c;
    a = ROTATE_LEFT(a + F(b,c,d) + block[8] + 0x698098d8, 7) + b;
    d = ROTATE_LEFT(d + F(a,b,c) + block[9] + 0x8b44f7af, 12) + a;
    c = ROTATE_LEFT(c + F(d,a,b) + block[10] + 0xffff5bb1, 17) + d;
    b = ROTATE_LEFT(b + F(c,d,a) + block[11] + 0x895cd7be, 22) + c;
    a = ROTATE_LEFT(a + F(b,c,d) + block[12] + 0x6b901122, 7) + b;
    d = ROTATE_LEFT(d + F(a,b,c) + block[13] + 0xfd987193, 12) + a;
    c = ROTATE_LEFT(c + F(d,a,b) + block[14] + 0xa679438e, 17) + d;
    b = ROTATE_LEFT(b + F(c,d,a) + block[15] + 0x49b40821, 22) + c;

    a = ROTATE_LEFT(a + G(b,c,d) + block[1] + 0xf61e2562, 5) + b;
    d = ROTATE_LEFT(d + G(a,b,c) + block[6] + 0xc040b340, 9) + a;
    c = ROTATE_LEFT(c + G(d,a,b) + block[11] + 0x265e5a51, 14) + d;
    b = ROTATE_LEFT(b + G(c,d,a) + block[0] + 0xe9b6c7aa, 20) + c;
    a = ROTATE_LEFT(a + G(b,c,d) + block[5] + 0xd62f105d, 5) + b;
    d = ROTATE_LEFT(d + G(a,b,c) + block[10] + 0x02441453, 9) + a;
    c = ROTATE_LEFT(c + G(d,a,b) + block[15] + 0xd8a1e681, 14) + d;
    b = ROTATE_LEFT(b + G(c,d,a) + block[4] + 0xe7d3fbc8, 20) + c;
    a = ROTATE_LEFT(a + G(b,c,d) + block[9] + 0x21e1cde6, 5) + b;
    d = ROTATE_LEFT(d + G(a,b,c) + block[14] + 0xc33707d6, 9) + a;
    c = ROTATE_LEFT(c + G(d,a,b) + block[3] + 0xf4d50d87, 14) + d;
    b = ROTATE_LEFT(b + G(c,d,a) + block[8] + 0x455a14ed, 20) + c;
    a = ROTATE_LEFT(a + G(b,c,d) + block[13] + 0xa9e3e905, 5) + b;
    d = ROTATE_LEFT(d + G(a,b,c) + block[2] + 0xfcefa3f8, 9) + a;
    c = ROTATE_LEFT(c + G(d,a,b) + block[7] + 0x676f02d9, 14) + d;
    b = ROTATE_LEFT(b + G(c,d,a) + block[12] + 0x8d2a4c8a, 20) + c;

    a = ROTATE_LEFT(a + H(b,c,d) + block[5] + 0xfffa3942, 4) + b;
    d = ROTATE_LEFT(d + H(a,b,c) + block[8] + 0x8771f681, 11) + a;
    c = ROTATE_LEFT(c + H(d,a,b) + block[11] + 0x6d9d6122, 16) + d;
    b = ROTATE_LEFT(b + H(c,d,a) + block[14] + 0xfde5380c, 23) + c;
    a = ROTATE_LEFT(a + H(b,c,d) + block[1] + 0xa4beea44, 4) + b;
    d = ROTATE_LEFT(d + H(a,b,c) + block[4] + 0x4bdecfa9, 11) + a;
    c = ROTATE_LEFT(c + H(d,a,b) + block[7] + 0xf6bb4b60, 16) + d;
    b = ROTATE_LEFT(b + H(c,d,a) + block[10] + 0xbebfbc70, 23) + c;
    a = ROTATE_LEFT(a + H(b,c,d) + block[13] + 0x289b7ec6, 4) + b;
    d = ROTATE_LEFT(d + H(a,b,c) + block[0] + 0xeaa127fa, 11) + a;
    c = ROTATE_LEFT(c + H(d,a,b) + block[3] + 0xd4ef3085, 16) + d;
    b = ROTATE_LEFT(b + H(c,d,a) + block[6] + 0x04881d05, 23) + c;
    a = ROTATE_LEFT(a + H(b,c,d) + block[9] + 0xd9d4d039, 4) + b;
    d = ROTATE_LEFT(d + H(a,b,c) + block[12] + 0xe6db99e5, 11) + a;
    c = ROTATE_LEFT(c + H(d,a,b) + block[15] + 0x1fa27cf8, 16) + d;
    b = ROTATE_LEFT(b + H(c,d,a) + block[2] + 0xc4ac5665, 23) + c;

    a = ROTATE_LEFT(a + I(b,c,d) + block[0] + 0xf4292244, 6) + b;
    d = ROTATE_LEFT(d + I(a,b,c) + block[7] + 0x432aff97, 10) + a;
    c = ROTATE_LEFT(c + I(d,a,b) + block[14] + 0xab9423a7, 15) + d;
    b = ROTATE_LEFT(b + I(c,d,a) + block[5] + 0xfc93a039, 21) + c;
    a = ROTATE_LEFT(a + I(b,c,d) + block[12] + 0x655b59c3, 6) + b;
    d = ROTATE_LEFT(d + I(a,b,c) + block[3] + 0x8f0ccc92, 10) + a;
    c = ROTATE_LEFT(c + I(d,a,b) + block[10] + 0xffeff47d, 15) + d;
    b = ROTATE_LEFT(b + I(c,d,a) + block[1] + 0x85845dd1, 21) + c;
    a = ROTATE_LEFT(a + I(b,c,d) + block[8] + 0x6fa87e4f, 6) + b;
    d = ROTATE_LEFT(d + I(a,b,c) + block[15] + 0xfe2ce6e0, 10) + a;
    c = ROTATE_LEFT(c + I(d,a,b) + block[6] + 0xa3014314, 15) + d;
    b = ROTATE_LEFT(b + I(c,d,a) + block[13] + 0x4e0811a1, 21) + c;
    a = ROTATE_LEFT(a + I(b,c,d) + block[4] + 0xf7537e82, 6) + b;
    d = ROTATE_LEFT(d + I(a,b,c) + block[11] + 0xbd3af235, 10) + a;
    c = ROTATE_LEFT(c + I(d,a,b) + block[2] + 0x2ad7d2bb, 15) + d;
    b = ROTATE_LEFT(b + I(c,d,a) + block[9] + 0xeb86d391, 21) + c;

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
}

static void MD5Encode(unsigned char* out, const uint32_t* in, unsigned int len) {
    for (unsigned int i = 0, j = 0; j < len; i++, j += 4) {
        out[j] = in[i] & 0xff;
        out[j+1] = (in[i] >> 8) & 0xff;
        out[j+2] = (in[i] >> 16) & 0xff;
        out[j+3] = (in[i] >> 24) & 0xff;
    }
}

static void MD5Decode(uint32_t* out, const unsigned char* in, unsigned int len) {
    for (unsigned int i = 0, j = 0; j < len; i++, j += 4) {
        out[i] = in[j] | (in[j+1] << 8) | (in[j+2] << 16) | (in[j+3] << 24);
    }
}

static void MD5Update(MD5_CTX* ctx, const unsigned char* input, unsigned int len) {
    unsigned int i, idx = (ctx->count[0] >> 3) & 0x3F;
    unsigned int partLen = 64 - idx;
    
    ctx->count[0] += len << 3;
    if (ctx->count[0] < (len << 3)) ctx->count[1]++;
    ctx->count[1] += len >> 29;
    
    if (len >= partLen) {
        memcpy(&ctx->buffer[idx], input, partLen);
        MD5Transform(ctx->state, (uint32_t*)ctx->buffer);
        for (i = partLen; i + 63 < len; i += 64)
            MD5Transform(ctx->state, (uint32_t*)(input + i));
        idx = 0;
    } else {
        i = 0;
    }
    memcpy(&ctx->buffer[idx], input + i, len - i);
}

static void MD5Final(MD5_CTX* ctx, unsigned char digest[16]) {
    unsigned char bits[8];
    unsigned int idx, padLen;
    
    MD5Encode(bits, ctx->count, 8);
    idx = (ctx->count[0] >> 3) & 0x3F;
    padLen = (idx < 56) ? (56 - idx) : (120 - idx);
    MD5Update(ctx, PADDING, padLen);
    MD5Update(ctx, bits, 8);
    MD5Encode(digest, ctx->state, 16);
    memset(ctx, 0, sizeof(*ctx));
}

// ============================================================================
// Helper
// ============================================================================

static std::string md5_hex(const unsigned char* d) {
    std::ostringstream o;
    for (int i = 0; i < 16; i++) o << std::hex << std::setw(2) << std::setfill('0') << (int)d[i];
    return o.str();
}

// ============================================================================
// Tests
// ============================================================================

class MD5Test : public ::testing::Test {};

TEST_F(MD5Test, EmptyString) {
    MD5_CTX ctx; MD5Init(&ctx);
    unsigned char d[16]; MD5Final(&ctx, d);
    EXPECT_EQ(md5_hex(d), "d41d8cd98f00b204e9800998ecf8427e");
}

TEST_F(MD5Test, Single_a) {
    MD5_CTX ctx; MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)"a", 1);
    unsigned char d[16]; MD5Final(&ctx, d);
    EXPECT_EQ(md5_hex(d), "0cc175b9c0f1b6a831c399e269772661");
}

TEST_F(MD5Test, ABC) {
    MD5_CTX ctx; MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)"abc", 3);
    unsigned char d[16]; MD5Final(&ctx, d);
    EXPECT_EQ(md5_hex(d), "900150983cd24fb0d6963f7d28e17f72");
}

TEST_F(MD5Test, MessageDigest) {
    MD5_CTX ctx; MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)"message digest", 14);
    unsigned char d[16]; MD5Final(&ctx, d);
    EXPECT_EQ(md5_hex(d), "f96b697d7cb7938d525a2f31aaf161d0");
}

TEST_F(MD5Test, Alphabet) {
    MD5_CTX ctx; MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char*)"abcdefghijklmnopqrstuvwxyz", 26);
    unsigned char d[16]; MD5Final(&ctx, d);
    EXPECT_EQ(md5_hex(d), "c3fcd3d76192e4007dfb496cca67e13b");
}

TEST_F(MD5Test, Incremental) {
    MD5_CTX c1, c2;
    MD5Init(&c1); MD5Update(&c1, (unsigned char*)"abc", 3);
    unsigned char d1[16]; MD5Final(&c1, d1);
    
    MD5Init(&c2); MD5Update(&c2, (unsigned char*)"a", 1);
    MD5Update(&c2, (unsigned char*)"b", 1);
    MD5Update(&c2, (unsigned char*)"c", 1);
    unsigned char d2[16]; MD5Final(&c2, d2);
    
    EXPECT_EQ(md5_hex(d1), md5_hex(d2));
}

TEST_F(MD5Test, LargeChunked) {
    std::string s(1000, 'A');
    MD5_CTX c1, c2;
    MD5Init(&c1); MD5Update(&c1, (unsigned char*)s.c_str(), s.size());
    unsigned char d1[16]; MD5Final(&c1, d1);
    
    MD5Init(&c2);
    for (int i = 0; i < 100; i++) MD5Update(&c2, (unsigned char*)"AAAAAAAAAA", 10);
    unsigned char d2[16]; MD5Final(&c2, d2);
    
    EXPECT_EQ(md5_hex(d1), md5_hex(d2));
}

class MD5PropertyTest : public ::testing::Test {
protected:
    std::mt19937 rng{54321};
    std::string rnd_str(int len) {
        std::uniform_int_distribution<int> d(0, 61);
        const char* c = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::string s; for (int i = 0; i < len; i++) s += c[d(rng)];
        return s;
    }
};

TEST_F(MD5PropertyTest, Deterministic) {
    for (int i = 0; i < 20; i++) {
        std::string s = rnd_str(50);
        MD5_CTX c1, c2;
        MD5Init(&c1); MD5Update(&c1, (unsigned char*)s.c_str(), s.size());
        unsigned char d1[16]; MD5Final(&c1, d1);
        MD5Init(&c2); MD5Update(&c2, (unsigned char*)s.c_str(), s.size());
        unsigned char d2[16]; MD5Final(&c2, d2);
        EXPECT_EQ(md5_hex(d1), md5_hex(d2));
    }
}

TEST_F(MD5PropertyTest, DifferentInputs) {
    std::vector<std::string> inputs = {"test1", "test2", "different", "Test1"};
    std::vector<std::string> hashes;
    for (auto& in : inputs) {
        MD5_CTX c; MD5Init(&c);
        MD5Update(&c, (unsigned char*)in.c_str(), in.size());
        unsigned char d[16]; MD5Final(&c, d);
        hashes.push_back(md5_hex(d));
    }
    for (size_t i = 0; i < hashes.size(); i++)
        for (size_t j = i + 1; j < hashes.size(); j++)
            EXPECT_NE(hashes[i], hashes[j]);
}

TEST(MD5EdgeCase, SingleBytes) {
    for (int i = 0; i < 256; i++) {
        unsigned char c = (unsigned char)i;
        MD5_CTX ctx; MD5Init(&ctx);
        MD5Update(&ctx, &c, 1);
        unsigned char d[16]; MD5Final(&ctx, d);
        EXPECT_EQ(md5_hex(d).size(), 32u);
    }
}
