#ifndef COINSIGN_ERROR_H
#define COINSIGN_ERROR_H

#include "libs01.hpp"

class coinsign_error: public std::runtime_error {
public:
    explicit coinsign_error (int errcode = 66,
                             const std::string& message = "coinsign error") :
                                                                              std::runtime_error(message),
                                                                              m_code(errcode),
                                                                              if_str_out(false) {
        setstr_out();
    }
    explicit coinsign_error (int errcode,
                                const char * message) :
                                                        std::runtime_error(message),
                                                        m_code(errcode),
                                                        if_str_out(false) {
        setstr_out();
    }
    coinsign_error (const coinsign_error &ce) :
                                                std::runtime_error(ce),
                                                m_code(ce.get_code()),
                                                if_str_out(false) {
        std::cout << "coinsign error : copy constructor!" << std::endl;
        setstr_out();
    }
    coinsign_error (coinsign_error &&ce) noexcept :
                                                    std::runtime_error(std::move(ce)),
                                                    m_code(ce.get_code()),
                                                    if_str_out(false) {
        std::cout << "coinsign error : move constructor!" << std::endl;
        setstr_out();
    }


    virtual ~coinsign_error () = default;
    bool setstr_out() {
        if(!if_str_out) {
            std::stringstream out;
            out << "Error " << m_code << ": " << std::runtime_error::what();
            m_str_out = out.str();
            if_str_out = true;
            return true;
        }
        return false;
    }

    int get_code () const {
        return m_code;
    }
    virtual const char* what () const noexcept {
        if(if_str_out) {
            return m_str_out.c_str();
        }
        return NULL;
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

    /// This string should be initial only once before the first run of what()
    std::string m_str_out;
    bool if_str_out;	///< Tell is str_out is set. Prevents print uninitialized str_out.
};

#endif // COINSIGN_ERROR_H
