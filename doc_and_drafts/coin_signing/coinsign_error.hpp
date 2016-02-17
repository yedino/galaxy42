#ifndef COINSIGN_ERROR_H
#define COINSIGN_ERROR_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <sstream>

class coinsign_error: public std::runtime_error {
public:
    explicit coinsign_error (int errcode = 66,
                             const std::string& message = "coinsign error") :
                                                                              std::runtime_error(message),
                                                                              m_code(errcode),
                                                                              if_str_out(false)
    { }
    explicit coinsign_error (int errcode,
                                const char * message) :
                                                        std::runtime_error(message),
                                                        m_code(errcode),
                                                        if_str_out(false)
    { }
    coinsign_error (const coinsign_error &ce) :
                                                m_code(ce.get_code()),
                                                if_str_out(false),
                                                std::runtime_error(ce) {
        std::cout << "coinsign error : copy constructor!" << std::endl;
    }
    coinsign_error (coinsign_error &&ce) noexcept :
                                                    m_code(ce.get_code()),
                                                    if_str_out(false),
                                                    std::runtime_error(std::move(ce)) {
        std::cout << "coinsign error : move constructor!" << std::endl;
    }


    virtual ~coinsign_error () = default;

    int get_code () const {
        return m_code;
    }
    virtual const char* what () noexcept{
        if(!if_str_out) {
            std::stringstream out;
            out << "Error " << m_code << ": " << std::runtime_error::what();
            this->m_str_out = out.str();
            this->if_str_out = true;
        }
        return m_str_out.c_str();
    }

private:

    /// \brief m_code -- code of error
    ///
    /// 66 : default error
    ///
    /// 10 : TOKEN VALIDATE FAIL - bad expected sender"
    /// 11 : TOKEN VALIDATE FAIL - bad sign"
    ///
    /// 12 : DEPRECATED TOKEN - bad token date
    /// 13 : MINT CHECK FAIL - bad mint public key"
    ///
    /// 14 : DOUBLE SPENDING - chaeter not found"
    /// 15 : DOUBLE SPENDING - found cheater"

    int m_code;

    /// This string should be initial only once at the first run of what(),
    std::string m_str_out;
    bool if_str_out;		///< warning ^^^ to prevent double setting error message
};

#endif // COINSIGN_ERROR_H
