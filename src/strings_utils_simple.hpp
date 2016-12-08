
#include <string>
#include <sstream>

// ====================================================================
// join string
// ====================================================================

// 1 arg call, normal:
template<class TSTREAM, class T1> void append_to_oss(TSTREAM & stream, const T1 & arg1) {
	stream << arg1;
}
// 1 arg call, with-separator:
template<class TSEP, class TSTREAM, class T1> void append_to_oss_sep(const TSEP& sep, TSTREAM & stream, const T1 & arg1) {
	stream << sep << arg1;
}

// 1+n arg call, normal:
template<class TSTREAM, class T1, class ...Ts> void append_to_oss(TSTREAM & stream, const T1 & arg1, const Ts & ... args ) {
	append_to_oss(stream, arg1);
	append_to_oss(stream, args...);
}
// 1+n arg call, with-separator:
template<class TSEP, class TSTREAM, class T1, class ...Ts>
void append_to_oss_sep(const TSEP& sep, TSTREAM & stream, const T1 & arg1, const Ts & ... args ) {
	append_to_oss_sep(sep, stream, arg1);
	append_to_oss_sep(sep, stream, args...);
}

// top-level call: normal:
template<class ...Ts> std::string join_string(const Ts & ... args ) {
	std::ostringstream oss;
	append_to_oss(oss , args... );
	return oss.str();
}
// top-level call: with-separator:
template<class T1, class ...Ts> std::string join_string_sep(const T1 & arg1, const Ts & ... args ) {
	std::ostringstream oss;
	append_to_oss    (      oss , arg1);
	append_to_oss_sep(", ", oss , args... );
	return oss.str();
}

