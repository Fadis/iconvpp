#include<iostream>
#include<string>
#include<langinfo.h>
#include<boost/preprocessor.hpp>
#include<iconv++/iconv++.hpp>

#define SETUP_LOCALE_FROM_ENV( z, index, seq ) \
  if BOOST_PP_LPAREN() getenv BOOST_PP_LPAREN() \
"LC_" BOOST_PP_STRINGIZE( BOOST_PP_SEQ_ELEM( index, seq ) ) \
BOOST_PP_RPAREN() BOOST_PP_RPAREN() \
setlocale BOOST_PP_LPAREN() \
BOOST_PP_CAT( LC_, BOOST_PP_SEQ_ELEM( index, seq ) ) \
BOOST_PP_COMMA() getenv BOOST_PP_LPAREN() \
"LC_" BOOST_PP_STRINGIZE( BOOST_PP_SEQ_ELEM( index, seq ) ) \
BOOST_PP_RPAREN() BOOST_PP_RPAREN() ;

void setup_locales() {
  BOOST_PP_REPEAT( 7, SETUP_LOCALE_FROM_ENV, \
      (ALL)(COLLATE)(CTYPE)(CTYPE)(MESSAGES)\
      (MONETARY)(NUMERIC)(TIME) )
}

class eucjpstring : public std::string {
  public:
    typedef struct {
      const static char value[];
    } code_name;
};

const char eucjpstring::code_name::value[] = "eucjp";

int main() {
  setup_locales();
  std::string system_string;
  std::getline( std::cin, system_string );
  std::cout << iconvpp::string_cast< eucjpstring >( system_string ) << std::endl;
}
