#pragma once

#include "cryptopp/cryptlib.h"
#include "cryptopp/modes.h"
#include "cryptopp/aes.h"
#include "cryptopp/sha.h"
#include "cryptopp/filters.h"

#include "console.hpp"

namespace Encryption {
    // CryptoPP has such a retarded interface
    // this namespace is just for shortcuts to CryptoPP
    namespace CryptoCockAndBalls {
        inline std::string sha256(std::string data)
        {
            char rawDigest[CryptoPP::SHA256::DIGESTSIZE];

            CryptoPP::SHA256().CalculateDigest(
                (CryptoPP::byte*)rawDigest,
                (CryptoPP::byte*)data.data(),
                data.size()
            );

            return std::string(rawDigest, CryptoPP::SHA256::DIGESTSIZE);
        }

        namespace AES {
            inline std::string encrypt(std::string data, std::string key)
            {
                char IV[CryptoPP::AES::BLOCKSIZE];
                memset(IV, 0, CryptoPP::AES::BLOCKSIZE);

                CryptoPP::AES::Encryption aesEncryption((CryptoPP::byte*)key.data(), key.size());
                CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, (CryptoPP::byte*)IV);

                std::string cipher;
                cipher.reserve(data.size() * 2); // approximation

                CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(cipher));
                stfEncryptor.Put((CryptoPP::byte*)data.data(), data.size());
                stfEncryptor.MessageEnd();

                return cipher;
            }

            inline std::string decrypt(std::string cipher, std::string key)
            {
                char IV[CryptoPP::AES::BLOCKSIZE];
                memset(IV, 0, CryptoPP::AES::BLOCKSIZE);

                CryptoPP::AES::Decryption aesDecryption((CryptoPP::byte*)key.data(), key.size());
                CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, (CryptoPP::byte*)IV);

                std::string data;
                data.reserve(cipher.size()); // approximation

                CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(data));
                stfDecryptor.Put((CryptoPP::byte*)cipher.data(), cipher.size());
                stfDecryptor.MessageEnd();

                return data;
            }
        }
    }

    // some utils, separated to avoid cluttering the main namespace
    namespace Util {
        constexpr inline char BIN_TO_HEX_DIGIT(const char& x)
        {
            constexpr char HEX_DIGITS[] = {
                '0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'A', 'B',
                'C', 'D', 'E', 'F',
            };
            return HEX_DIGITS[x];
        }

        constexpr inline char HEX_DIGIT_TO_BIN(const char& x)
        {
            switch (x) {
            default:
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'A': case 'a': return 10;
            case 'B': case 'b': return 11;
            case 'C': case 'c': return 12;
            case 'D': case 'd': return 13;
            case 'E': case 'e': return 14;
            case 'F': case 'f': return 15;
            }
        }
    }

    // hex digest given binary data
    inline std::string hex(std::string data)
    {
        std::string hex;
        hex.reserve(data.size() * 2);

        for (const char& byte : data)
        {
            hex += Util::BIN_TO_HEX_DIGIT(byte >> 4);
            hex += Util::BIN_TO_HEX_DIGIT(byte & 0b1111);
        }

        return hex;
    }

    // binary data given hex digest
    inline std::string unhex(std::string hex)
    {
        std::string data;
        data.reserve(hex.size() / 2);

        for (size_t i = 0; i + 1 < hex.size(); i += 2)
        {
            const char& a = hex.at(i), b = hex.at(i + 1);
            const char byte = (Util::HEX_DIGIT_TO_BIN(a) << 4) | Util::HEX_DIGIT_TO_BIN(b);
            data += byte;
        }

        return data;
    }

    // returns a hardware id that should be universally unique, and be constant across multiple calls in different sessions
    inline std::string getHardwareUUID()
    {
        std::string output = exec("wmic csproduct get UUID");
        output.erase(std::remove_if(output.begin(), output.end(), [](char c) { return !std::isalnum(c); }), output.end());
        return output;
    }

    // returns the binary digest of the SHA-256 hash of `msg` (will always be 32 characters)
    inline std::string sha256(std::string msg)
    {
        return CryptoCockAndBalls::sha256(msg);
    }

    // gets a 256-bit AES encryption key that is based on hardware and should be constant across sessions
    inline std::string getEncryptionKey()
    {
        return sha256(getHardwareUUID());
    }

    // AES encrypt
    inline std::string encrypt(std::string data, std::string key = "")
    {
        if (key == "")
            key = getEncryptionKey();
        else
            key = sha256(key); // coerce to 256 bits
        
        return CryptoCockAndBalls::AES::encrypt(data, key);
    }

    // AES decrypt
    inline std::string decrypt(std::string data, std::string key = "")
    {
        if (key == "")
            key = getEncryptionKey();
        else
            key = sha256(key); // coerce to 256 bits

        return CryptoCockAndBalls::AES::decrypt(data, key);
    }
}
