
#pragma once

#include <string>
#include <stdexcept>


class runtime_error_subtype : public std::runtime_error {
	public:
		runtime_error_subtype();

		template <typename T_SUBTYPE>
		static T_SUBTYPE make_subtype(const std::string & msg) {
			T_SUBTYPE subtype;
			subtype.set_msg(msg);
			return subtype;
        }
		virtual const char * what() const noexcept override;

		protected:
			void set_msg(const std::string &msg);

		private:
			std::string m_msg; ///< my message (from subtype)
};


// throw runtime_error_subtype::make_subtype<tuntap_error_devtun>(".....")
#define _throw_error_sub(TYPE, MSG) do { _throw_error( runtime_error_subtype::make_subtype<TYPE>(MSG) ); } while(0)

