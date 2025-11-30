#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>

class SHA256 {
public:
    unsigned int DIGEST_SIZE;
    SHA256() { DIGEST_SIZE = 32; reset(); }

    void update(const unsigned char* data, size_t len) {
        for(size_t i = 0; i < len; i++) {
            dataBuffer[dataLength] = data[i];
            dataLength++;
            if (dataLength == 64) {
                transform();
                totalLength += 512;
                dataLength = 0;
            }
        }
    }

    void update(const std::string& data) {
        update(reinterpret_cast<const unsigned char*>(data.c_str()), data.size());
    }

    std::string final() {
        unsigned int i = dataLength;

        if (dataLength < 56) {
            dataBuffer[i++] = 0x80;
            while (i < 56) dataBuffer[i++] = 0x00;
        } else {
            dataBuffer[i++] = 0x80;
            while (i < 64) dataBuffer[i++] = 0x00;
            transform();
            memset(dataBuffer, 0, 56);
        }

        totalLength += dataLength * 8;
        dataBuffer[63] = totalLength;
        dataBuffer[62] = totalLength >> 8;
        dataBuffer[61] = totalLength >> 16;
        dataBuffer[60] = totalLength >> 24;
        dataBuffer[59] = totalLength >> 32;
        dataBuffer[58] = totalLength >> 40;
        dataBuffer[57] = totalLength >> 48;
        dataBuffer[56] = totalLength >> 56;
        transform();

        std::ostringstream result;
        for (int i = 0; i < 8; i++)
            result << std::hex << std::setw(8) << std::setfill('0') << state[i];

        reset();
        return result.str();
    }

private:
    unsigned char dataBuffer[64];
    unsigned int dataLength;
    unsigned long long totalLength;
    unsigned int state[8];

    void reset() {
        dataLength = 0;
        totalLength = 0;
        state[0] = 0x6a09e667;
        state[1] = 0xbb67ae85;
        state[2] = 0x3c6ef372;
        state[3] = 0xa54ff53a;
        state[4] = 0x510e527f;
        state[5] = 0x9b05688c;
        state[6] = 0x1f83d9ab;
        state[7] = 0x5be0cd19;
    }

    unsigned int rotr(unsigned int x, unsigned int n) {
        return (x >> n) | (x << (32 - n));
    }

    void transform() {
        unsigned int K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
            0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
            0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
            0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
            0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
            0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
            0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
            0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
            0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
            0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
        };

        unsigned int w[64], a, b, c, d, e, f, g, h;

        for (int i = 0; i < 16; i++)
            w[i] = (dataBuffer[i * 4] << 24) |
                (dataBuffer[i * 4 + 1] << 16) |
                (dataBuffer[i * 4 + 2] << 8) |
                (dataBuffer[i * 4 + 3]);

        for (int i = 16; i < 64; i++)
            w[i] = w[i - 16] + (rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3))
                + w[i - 7] + (rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10));

        a = state[0]; b = state[1]; c = state[2]; d = state[3];
        e = state[4]; f = state[5]; g = state[6]; h = state[7];

        for (int i = 0; i < 64; i++) {
            unsigned int t1 = h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25))
                + ((e & f) ^ (~e & g)) + K[i] + w[i];
            unsigned int t2 = (rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22))
                + ((a & b) ^ (a & c) ^ (b & c));
            h = g; g = f; f = e;
            e = d + t1;
            d = c; c = b; b = a;
            a = t1 + t2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }
};

inline std::string sha256(const std::string &data) {
    SHA256 sha;
    sha.update(data);
    return sha.final();
}

#endif
