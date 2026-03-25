#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <QDir>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <optional>

class Application;

class AuthService
{
    friend class Application;

public:

    bool idVerify(std::string in_user_name, std::string in_password);

private:

    QString try_get_key_path();
    bool get_key_from_file(const std::string& filename, std::uint8_t& key_out);
    std::uint8_t obf_key(std::uint8_t k);
    void xor_inplace(std::string& s, unsigned char key);
    std::string xor_copy(const std::string& in, std::uint8_t key);
    std::string bytes_to_hex(const std::string& s);
    bool read_u32(std::ifstream& in, std::uint32_t& v);
    bool read_u8(std::ifstream& in, std::uint8_t& v);
    bool read_blob(std::ifstream& in, std::string& s, std::uint32_t len);

private:
    AuthService();
    AuthService(const AuthService& copy) = delete;
    AuthService& operator=(const AuthService& copy) = delete;
    AuthService(AuthService&& move) = delete;
    AuthService& operator=(AuthService&& move) = delete;

private:
    static constexpr std::int32_t magic = 0x44533143u; // 'C''S''1''D' little-endian display
    static constexpr std::int8_t version = 1;
    static constexpr std::uint8_t key_mask = 0x5Au;
};

#endif // AUTH_SERVICE_H
