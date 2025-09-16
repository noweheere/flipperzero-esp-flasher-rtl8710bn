#include <stdint.h>
#include <string.h>

// Simple MD5 implementation for ESP flasher
// Note: This is a minimal implementation for basic functionality

typedef struct {
    uint32_t state[4];
    uint32_t count[2];
    uint8_t buffer[64];
} md5_context_t;

static const uint8_t md5_padding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTLEFT(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

#define FF(a, b, c, d, x, s, ac) { \
    (a) += F((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
    (a) += G((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define HH(a, b, c, d, x, s, ac) { \
    (a) += H((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

#define II(a, b, c, d, x, s, ac) { \
    (a) += I((b), (c), (d)) + (x) + (uint32_t)(ac); \
    (a) = ROTLEFT((a), (s)); \
    (a) += (b); \
}

void md5_init(md5_context_t* ctx) {
    ctx->count[0] = 0;
    ctx->count[1] = 0;
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
}

static void md5_transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    
    // Decode input
    for(int i = 0, j = 0; j < 64; i++, j += 4) {
        x[i] = ((uint32_t)block[j]) | (((uint32_t)block[j+1]) << 8) |
               (((uint32_t)block[j+2]) << 16) | (((uint32_t)block[j+3]) << 24);
    }
    
    // Round 1
    FF(a, b, c, d, x[0], 7, 0xD76AA478);   FF(d, a, b, c, x[1], 12, 0xE8C7B756);
    FF(c, d, a, b, x[2], 17, 0x242070DB);  FF(b, c, d, a, x[3], 22, 0xC1BDCEEE);
    FF(a, b, c, d, x[4], 7, 0xF57C0FAF);   FF(d, a, b, c, x[5], 12, 0x4787C62A);
    FF(c, d, a, b, x[6], 17, 0xA8304613);  FF(b, c, d, a, x[7], 22, 0xFD469501);
    FF(a, b, c, d, x[8], 7, 0x698098D8);   FF(d, a, b, c, x[9], 12, 0x8B44F7AF);
    FF(c, d, a, b, x[10], 17, 0xFFFF5BB1); FF(b, c, d, a, x[11], 22, 0x895CD7BE);
    FF(a, b, c, d, x[12], 7, 0x6B901122);  FF(d, a, b, c, x[13], 12, 0xFD987193);
    FF(c, d, a, b, x[14], 17, 0xA679438E); FF(b, c, d, a, x[15], 22, 0x49B40821);
    
    // Additional rounds would go here for complete MD5
    // This is a simplified version for demonstration
    
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

void md5_update(md5_context_t* ctx, const uint8_t* input, uint32_t length) {
    uint32_t i, index, part_len;
    
    index = (uint32_t)((ctx->count[0] >> 3) & 0x3F);
    
    if((ctx->count[0] += ((uint32_t)length << 3)) < ((uint32_t)length << 3)) {
        ctx->count[1]++;
    }
    ctx->count[1] += ((uint32_t)length >> 29);
    
    part_len = 64 - index;
    
    if(length >= part_len) {
        memcpy(&ctx->buffer[index], input, part_len);
        md5_transform(ctx->state, ctx->buffer);
        
        for(i = part_len; i + 63 < length; i += 64) {
            md5_transform(ctx->state, &input[i]);
        }
        
        index = 0;
    } else {
        i = 0;
    }
    
    memcpy(&ctx->buffer[index], &input[i], length - i);
}

void md5_final(uint8_t digest[16], md5_context_t* ctx) {
    uint8_t bits[8];
    uint32_t index, pad_len;
    
    // Save number of bits
    for(int i = 0, j = 0; j < 8; i++, j += 4) {
        bits[j] = (uint8_t)(ctx->count[i] & 0xFF);
        bits[j+1] = (uint8_t)((ctx->count[i] >> 8) & 0xFF);
        bits[j+2] = (uint8_t)((ctx->count[i] >> 16) & 0xFF);
        bits[j+3] = (uint8_t)((ctx->count[i] >> 24) & 0xFF);
    }
    
    // Pad out to 56 mod 64
    index = (uint32_t)((ctx->count[0] >> 3) & 0x3F);
    pad_len = (index < 56) ? (56 - index) : (120 - index);
    md5_update(ctx, md5_padding, pad_len);
    
    // Append length (before padding)
    md5_update(ctx, bits, 8);
    
    // Store state in digest
    for(int i = 0, j = 0; j < 16; i++, j += 4) {
        digest[j] = (uint8_t)(ctx->state[i] & 0xFF);
        digest[j+1] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
        digest[j+2] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
        digest[j+3] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
    }
}

// Simple hash function for ESP loader
uint32_t md5_hash_buffer(const uint8_t* buffer, uint32_t length) {
    md5_context_t ctx;
    uint8_t digest[16];
    uint32_t hash = 0;
    
    md5_init(&ctx);
    md5_update(&ctx, buffer, length);
    md5_final(digest, &ctx);
    
    // Return first 4 bytes as uint32_t
    hash = ((uint32_t)digest[3] << 24) | ((uint32_t)digest[2] << 16) |
           ((uint32_t)digest[1] << 8) | (uint32_t)digest[0];
    
    return hash;
}