#include<cstring>
#include<iostream>
#include<string>
#include<iconv.h>
#include<memory>
#include<boost/mpl/has_xxx.hpp>
#include<langinfo.h>
#include<boost/preprocessor.hpp>
#include<exception>
#include<stdexcept>
#include<vector>

namespace iconvpp {
  namespace detail {
    BOOST_MPL_HAS_XXX_TRAIT_DEF(code_name)
  }

  template< typename StringType, bool has_code_name = detail::has_code_name< StringType >::value >
    struct get_code_name  {
    };

  template< typename StringType >
    struct get_code_name<StringType,true> {
      static std::string value() {
        static const std::string name( StringType::code_name::value );
        return name;
      }
    };

  template<>
    struct get_code_name< std::u32string, false > {
      static std::string value() {
        static const std::string name( "UCS-4LE" );
        return name;
      }
    };

  template<>
    struct get_code_name< std::u16string, false > {
      static std::string value() {
        static const std::string name( "UCS-2" );
        return name;
      }
    };

  template<>
    struct get_code_name< std::string, false > {
      static std::string value() {
        const std::string name( nl_langinfo( CODESET ) );
        return name;
      }
    };


  template< size_t block_size >
    std::pair< std::vector<char>, std::vector<char> > convert_block( iconv_t cd, const std::vector<char> &input ) {
      std::vector<char> output( block_size );
      size_t input_left_size = input.size();
      size_t output_left_size = output.size();
      while( 1 ) {
        char *current_input_head = const_cast< char* >( &input[ input.size() - input_left_size ] );
        char *current_output_head = &output[ output.size() - output_left_size ];
        const size_t result = iconv( cd, &current_input_head, &input_left_size,
            &current_output_head, &output_left_size );
        if( static_cast<int>( result ) != -1 ) { // iconv succeed
          output.resize( output.size() - output_left_size );
          return std::pair< std::vector<char>, std::vector<char> >( output, std::vector<char>() );
        }
        else { // iconv failed for some reason
          switch( errno ) {
            case E2BIG: // output buffer was too small then extend it and try again.
              {
                output.resize( output.size() + block_size );
                output_left_size += block_size;
                break;
              }
            case EINVAL: // input ended in the middle of multibyte charcter then return partial charcter sequence. 
              {
                output.resize( output.size() - output_left_size );
                std::vector<char>::const_iterator invalid_char_head = input.begin();
                std::advance( invalid_char_head, input.size() - input_left_size );
                return std::pair< std::vector<char>, std::vector<char> >( output, std::vector<char>( invalid_char_head, input.end() ) );
              }
            default: // other erros are not fixable in this function so just throw
              throw std::runtime_error( strerror( errno ) );
          }
        }
      }
      throw std::runtime_error( "This code must not run" );
    }

  template< typename From, typename To >
    class converter {
      public:
        static const size_t block_size = 256u;
        converter() : cd( iconv_open( get_code_name<To>::value().c_str(), get_code_name<From>::value().c_str() ) ) {
          if( cd == reinterpret_cast<iconv_t>(-1) )
            throw std::runtime_error(strerror(errno));
        }
        std::vector< char > get_block(
            std::vector<char>::const_iterator prefix_begin, std::vector<char>::const_iterator prefix_end,
            const From &from_string, typename From::size_type head_pos ) {
          typename From::size_type block_size = std::min( from_string.size() * sizeof( typename From::value_type ) - head_pos, block_size );
          std::vector< char > block( prefix_begin, prefix_end );
          block.resize( block.size() + block_size );
          std::vector< char >::iterator output_head = block.begin();
          std::advance( output_head, std::distance( prefix_begin, prefix_end ) );
          std::copy( reinterpret_cast<const char*>( from_string.c_str() ) + head_pos, reinterpret_cast<const char*>( from_string.c_str() ) + head_pos + block_size, output_head );
          return block;
        }
        To set_block( const std::vector<char> &block ) {
          To new_string;
          new_string.resize( block.size() / sizeof( typename To::value_type ) );
          std::vector< typename To::value_type > temp(
              block.size() / sizeof( typename To::value_type )
              );
          std::copy( block.begin(), block.end(), reinterpret_cast<char*>( &temp[0] ) );
          std::copy( temp.begin(), temp.end(), new_string.begin() );
          return new_string;
        }
        To operator()( const From &from_string ) {
          To to_string;
          typename From::size_type head_pos = 0;
          std::vector< char > partial_sequence;
          while( 1 ) {
            const std::vector< char > input_block = get_block( partial_sequence.begin(), partial_sequence.end(), from_string, head_pos );
            head_pos += input_block.size() - partial_sequence.size();
            const std::pair< std::vector< char >, std::vector< char > > output_block = convert_block< block_size >( cd, input_block );
            const To temp = set_block( output_block.first );
            partial_sequence = output_block.second;
            to_string += temp;
            if( head_pos == from_string.size() * sizeof( typename From::value_type ) )
              break;

          }
          return to_string;
        }
      private:
        iconv_t cd; 
    };
  namespace detail {
    template< typename From, typename To >
      class string_cast_internal {
        public:
          static To exec( From input ) {
            iconvpp::converter< From, To > iconv_instance;
            return iconv_instance( input );
          }
      };
    template< typename To >
      class string_cast_internal< const char*, To > {
        public:
          static To exec( const char *input ) {
            iconvpp::converter< std::string, To > iconv_instance;
            return iconv_instance( input );
          }
      };
    template< typename To >
      class string_cast_internal< const char32_t*, To > {
        public:
          static To exec( const char32_t *input ) {
            iconvpp::converter< std::u32string, To > iconv_instance;
            return iconv_instance( input );
          }
      };
    template< typename To >
      class string_cast_internal< const char16_t*, To > {
        public:
          static To exec( const char16_t *input ) {
            iconvpp::converter< std::u16string, To > iconv_instance;
            return iconv_instance( input );
          }
      };
  }

  template< typename To, typename From >
    To string_cast( From input ) {
      return detail::string_cast_internal< From, To >::exec( input );
    }
}

