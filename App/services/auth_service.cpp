#include "auth_service.h"

AuthService::AuthService() {}

void AuthService::setKeyPath(const QString& key_path)
{
    _key_path = key_path;
}

std::uint8_t AuthService::obf_key(std::uint8_t k)
{
    return static_cast<std::uint8_t>(k ^ key_mask);
}

void AuthService::xor_inplace(std::string& s, unsigned char key)
{
    for (auto& ch : s)
        ch = static_cast<char>(static_cast<unsigned char>(ch) ^ key);
}

std::string AuthService::xor_copy(const std::string& in, std::uint8_t key)
{
    std::string out = in;
    xor_inplace(out, key);
    return out;
}

std::string AuthService::bytes_to_hex(const std::string& s)
{
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < s.size(); ++i)
    {
        const auto byte = static_cast<unsigned int>(
            static_cast<unsigned char>(s[i]));
        oss << std::setw(2) << byte;
        if (i + 1 < s.size())
            oss << ' ';
    }
    return oss.str();
}

bool AuthService::read_u32(std::ifstream& in, std::uint32_t& v)
{
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return static_cast<bool>(in);
}

bool AuthService::read_u8(std::ifstream& in, std::uint8_t& v)
{
    int c = in.get();
    if (c == EOF) return false;
    v = static_cast<std::uint8_t>(c);
    return true;
}

bool AuthService::read_blob(std::ifstream& in, std::string& s, std::uint32_t len)
{
    s.resize(len);
    in.read(s.data(), static_cast<std::streamsize>(len));
    return static_cast<bool>(in);
}

bool AuthService::loadAuthRecord(
    const std::string& filename,
    std::uint8_t& key_out,
    std::string& user_enc_out,
    std::string& password_enc_out)
{
    std::ifstream in(filename, std::ios::binary);
    if (!in)
    {
        std::cerr << "cannot open: " << filename << "\n";
        return false;
    }

    std::uint32_t file_magic = 0;
    if (!read_u32(in, file_magic))
    {
        std::cerr << "read magic failed\n";
        return false;
    }
    if (file_magic != magic)
    {
        std::cerr << "magic mismatch\n";
        return false;
    }

    std::uint8_t file_version = 0;
    if (!read_u8(in, file_version))
    {
        std::cerr << "read version failed\n";
        return false;
    }
    if (file_version != version)
    {
        std::cerr << "version mismatch\n";
        return false;
    }

    std::uint32_t u_len = 0;
    if (!read_u32(in, u_len))
    {
        std::cerr << "read username length failed\n";
        return false;
    }

    if (!read_blob(in, user_enc_out, u_len))
    {
        std::cerr << "read username blob failed\n";
        return false;
    }

    std::uint32_t p_len = 0;
    if (!read_u32(in, p_len))
    {
        std::cerr << "read password length failed\n";
        return false;
    }

    if (!read_blob(in, password_enc_out, p_len))
    {
        std::cerr << "read password blob failed\n";
        return false;
    }

    in.seekg(0, std::ios::end);
    const auto size = in.tellg();
    if (size < 1)
    {
        std::cerr << "file empty\n";
        return false;
    }

    in.seekg(-1, std::ios::end);
    std::uint8_t key_obf_byte = 0;
    if (!read_u8(in, key_obf_byte))
    {
        std::cerr << "read tail key failed\n";
        return false;
    }

    key_out = static_cast<std::uint8_t>(key_obf_byte ^ key_mask);
    return true;
}

bool AuthService::idVerify(std::string i_user_name, std::string i_password)
{
    if (i_user_name.empty() || i_password.empty())
        return false;

    if (_key_path.trimmed().isEmpty())
    {
        std::cerr << "auth key path is not configured\n";
        return false;
    }

    std::string key_path = _key_path.toStdString();
    std::cout << key_path << std::endl;

    std::uint8_t key = 0;
    std::string u_enc_stored;
    std::string p_enc_stored;
    if (!loadAuthRecord(key_path, key, u_enc_stored, p_enc_stored))
    {
        std::cerr << "loadAuthRecord failed\n";
        return false;
    }

    const std::string u_enc_input = xor_copy(i_user_name, key);

    const std::string p_enc_input = xor_copy(i_password, key);

    const bool user_ok = (u_enc_input == u_enc_stored);
    const bool pass_ok = (p_enc_input == p_enc_stored);
    const bool verify_ok = user_ok && pass_ok;

    return verify_ok;
}
