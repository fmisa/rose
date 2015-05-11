#ifndef ROSE_BinaryAnalysis_String_H
#define ROSE_BinaryAnalysis_String_H

#include <MemoryMap.h>
#include <sawyer/Optional.h>

namespace rose {
namespace BinaryAnalysis {

/** %Analysis for finding strings in memory.
 *
 *  This analysis looks for various kinds of strings in specimen memory.  A string is a sequence of characters encoded in one
 *  of a variety of ways in memory.  For instance, NUL-terminated ASCII is a common encoding from C compilers.  The characters
 *  within the string must all satisfy some valid-character predicate.
 *
 *  The terms used in this analysis are based on the Unicode standard, and are defined here in terms of string encoding
 *  (translation of a string as printed to a sequence of octets). Although this analysis can encode strings, its main purpose
 *  is decoding strings from an octet stream into a sequence of code points.
 *
 *  Unicode and its parallel standard, the ISO/IEC 10646 Universal Character Set, together constitute a modern, unified
 *  character encoding. Rather than mapping characters directly to octets (bytes), they separately define what characters are
 *  available, their numbering, how those numbers are encoded as a series of "code units" (limited-size numbers), and finally
 *  how those units are encoded as a stream of octets. The idea behind this decomposition is to establish a universal set of
 *  characters that can be encoded in a variety of ways. To describe this model correctly one needs more precise terms than
 *  "character set" and "character encoding." The terms used in the modern model follow:
 *
 *  A character repertoire is the full set of abstract characters that a system supports. The repertoire may be closed, i.e. no
 *  additions are allowed without creating a new standard (as is the case with ASCII and most of the ISO-8859 series), or it
 *  may be open, allowing additions (as is the case with Unicode and to a limited extent the Windows code pages). The
 *  characters in a given repertoire reflect decisions that have been made about how to divide writing systems into basic
 *  information units. The basic variants of the Latin, Greek and Cyrillic alphabets can be broken down into letters, digits,
 *  punctuation, and a few special characters such as the space, which can all be arranged in simple linear sequences that are
 *  displayed in the same order they are read. Even with these alphabets, however, diacritics pose a complication: they can be
 *  regarded either as part of a single character containing a letter and diacritic (known as a precomposed character), or as
 *  separate characters. The former allows a far simpler text handling system but the latter allows any letter/diacritic
 *  combination to be used in text. Ligatures pose similar problems. Other writing systems, such as Arabic and Hebrew, are
 *  represented with more complex character repertoires due to the need to accommodate things like bidirectional text and
 *  glyphs that are joined together in different ways for different situations.
 *
 *  A coded character set (CCS) specifies how to represent a repertoire of characters using a number of (typically
 *  non-negative) integer values called code points. For example, in a given repertoire, a character representing the capital
 *  letter "A" in the Latin alphabet might be assigned to the integer 65, the character for "B" to 66, and so on. A complete
 *  set of characters and corresponding integers is a coded character set. Multiple coded character sets may share the same
 *  repertoire; for example ISO/IEC 8859-1 and IBM code pages 037 and 500 all cover the same repertoire but map them to
 *  different codes. In a coded character set, each code point only represents one character, i.e., a coded character set is a
 *  function.
 *
 *  A character encoding form (CEF) specifies the conversion of a coded character set's integer codes into a set of
 *  limited-size integer code values that facilitate storage in a system that represents numbers in binary form using a fixed
 *  number of bits (i.e. practically any computer system). For example, a system that stores numeric information in 16-bit
 *  units would only be able to directly represent integers from 0 to 65,535 in each unit, but larger integers could be
 *  represented if more than one 16-bit unit could be used. This is what a CEF accommodates: it defines a way of mapping a
 *  single code point from a range of, say, 0 to 1.4 million, to a series of one or more code values from a range of, say, 0 to
 *  65,535.
 *
 *  The simplest CEF system is simply to choose large enough units that the values from the coded character set can be encoded
 *  directly (one code point to one code value). This works well for coded character sets that fit in 8 bits (as most legacy
 *  non-CJK encodings do) and reasonably well for coded character sets that fit in 16 bits (such as early versions of
 *  Unicode). However, as the size of the coded character set increases (e.g. modern Unicode requires at least 21
 *  bits/character), this becomes less and less efficient, and it is difficult to adapt existing systems to use larger code
 *  values. Therefore, most systems working with later versions of Unicode use either UTF-8, which maps Unicode code points to
 *  variable-length sequences of octets, or UTF-16, which maps Unicode code points to variable-length sequences of 16-bit
 *  words.
 *
 *  Next, a character encoding scheme (CES) specifies how the fixed-size integer code values should be mapped into an octet
 *  sequence suitable for saving on an octet-based file system or transmitting over an octet-based network. With Unicode, a
 *  simple character encoding scheme is used in most cases, simply specifying whether the bytes for each integer should be in
 *  big-endian or little-endian order (even this isn't needed with UTF-8). However, there are also compound character encoding
 *  schemes, which use escape sequences to switch between several simple schemes (such as ISO/IEC 2022), and compressing
 *  schemes, which try to minimise the number of bytes used per code unit (such as SCSU, BOCU, and Punycode).
 *
 *  Once the code points of a string are encoded as octets, the string as a whole needs some description to demarcate it from
 *  surrounding data. ROSE currently supports two styles of demarcation: length-encoded strings and terminated strings.  A
 *  length-encoded string's code point octets are preceded by octets that encode the string length, usually in terms of the
 *  number of code points. Decoding such a string consists of decoding the length and then decoding code points until the
 *  required number of code points have been obtained.  On the other hand, terminated strings are demarcated from surrounding
 *  data by a special code point such as the NUL character for ASCII strings. Decoding a terminated string consists of decoding
 *  code points until a terminator is found, then discarding the terminator.
 *
 * @section ex1 Example 1
 *
 *  This example shows how to find all strings in memory that is readable but not writable using a list of common encodings
 *  such as C-style NUL-terminated printable ASCII, zero terminated UTF-16 little-endian, 2-byte little-endian length encoded
 *  ASCII, etc.
 *
 * @code
 *  #include <rose/BinaryString.h>
 *  using namespace rose::BinaryAnalysis::Strings;
 *  MemoryMap map = ...;            // initialized elsewhere
 *
 *  StringFinder finder;            // holds settings
 *  finder.minLength(5);            // no strings shorter than 5 characters
 *  finder.maxLength(65536);        // ignore very long strings
 *  finder.includeCommonEncoders(); // how to match strings
 *
 *  std::vector<EncodedString> strings = finder.find(map.require(MemoryMap::READABLE).prohibit(MemoryMap::WRITABLE));
 *  BOOST_FOREACH (const EncodedString &string, strings) {
 *      std::cout <<"string at " <<string.address() <<" for " <<string.size() <<" bytes\n";
 *      std::cout <<"encoding: " <<string.encoder()->name() <<"\n";
 *      std::cout <<"narrow value: \"" <<StringUtility::cEscape(string.narrow()) <<"\"\n"; // std::string
 *      std::cout <<"wide value: " <<string.wide() <<"\n"; // std::wstring
 *  }
 * @endcode
 *
 * @section ex2 Example 2
 *
 *  This analysis is tuned for searching for strings at unknown locations while trying to decode multiple encodings
 *  simultaneously. If all you want to do is read a single string from a known location having a known encoding then you're
 *  probabily better off reading it directly from the MemoryMap. This analysis can be used for that, but it's probably
 *  overkill. In any case, here's the overkill version to find a 2-byte little endian length-encoded UTF-8 string:
 *
 * @code
 *  #include <rose/BinaryString.h>
 *  using namespace rose::BinaryAnalysis::Strings;
 *  MemoryMap map = ...;            // initialized elsewhere
 *  rose_addr_t stringVa = ...;     // starting address of string
 *
 *  StringFinder finder;            // holds settings
 *  finder.minLength(0);            // no strings shorter than 5 characters
 *  finder.maxLength(65536);        // ignore very long strings
 *  finder.encoder(lengthEncodedString(basicLengthEncoder(2, ByteOrder::ORDER_LSB), // 2-byte little-endian length
 *                                     utf8CharacterEncodingForm(),                 // UTF-8 encoding
 *                                     basicCharacterEncodingScheme(1),             // 1:1 mapping to octets
 *                                     anyCodePoint());                             // allow any characters
 *
 *  std::wstring s;                                   
 *  BOOST_FOREACH (const EncodedString &string, finder.find(map.at(stringVa))) {
 *      s = string.wide();
 *      break;
 *  }
 * @endcode
 *
 * @section ex3 Example 3
 *
 *  The encoders can also be used to decode directly from a stream of octets. For instance, lets say you have a vector of
 *  octets that map 1:1 to code values, and then you want to decode the code values as a UTF-8 stream to get some code
 *  points. All decoders are implemented as state machines to make it efficient to send the same octets to many decoders
 *  without having to rescan/reread from a memory map.  The UTF-8 decoder decodes one octet at a time and when it enters the
 *  FINAL_STATE or COMPLETED_STATE then a decoded code value can be consumed.
 *
 * @code
 *  #include <rose/BinaryString.h>
 *  using namespace rose::BinaryAnalysis::Strings;
 *  std::vector<Octet> octets = ...; // initialized elsewhere
 *
 *  // Instantiate the encoder/decoder. These things are all reference
 *  // counted so there's no need to explicitly free them.
 *  Utf8CharacterEncodingForm::Ptr utf8 = utf8CharacterEncodingForm();
 * 
 *  CodePoints codePoints;
 *  BOOST_FOREACH (Octet octet, octets) {
 *      CodeValue codeValue = octet; // 1:1 translation
 *      if (isDone(utf8->decode(codeValue))) {
 *          codePoints.push_back(utf8->consume());
 *      } else if (utf8->state() == ERROR_STATE) {
 *          utf8->reset(); // skip this code value
 *      }
 *  }
 * @endcode */
namespace Strings {

typedef uint8_t Octet;                                  /**< One byte in a sequence that encodes a code value. */
typedef std::vector<Octet> Octets;                      /**< A sequence of octets. */
typedef unsigned CodeValue;                             /**< One value in a sequence that encodes a code point. */
typedef std::vector<CodeValue> CodeValues;              /**< A sequence of code values. */
typedef unsigned CodePoint;                             /**< One character in a coded character set. */
typedef std::vector<CodePoint> CodePoints;              /**< A sequence of code points, i.e., a string. */

/** Decoder state. Negative values are reserved.
 *
 *  A decoder must follow these rules when transitioning from one state to another:
 *
 *  @li A decoder is in the INITIAL_STATE when it is constructed and after calling @c reset.
 *
 *  @li If the decoder is in an ERROR_STATE then @c decode does not change the state.
 *
 *  @li If the decoder is in the FINAL_STATE then @c decode transitions to ERROR_STATE.
 *
 *  @li If the decoder is in FINAL_STATE or COMPLETED state then @c consume transitions to INITIAL_STATE.
 *
 *  All other transitions are user defined. */
enum State {
    FINAL_STATE      = -1,                              /**< Final state where nothing more can be decoded. */
    COMPLETED_STATE  = -2,                              /**< Completed state, but not a final state. */
    INITIAL_STATE    = -3,                              /**< Initial state just after a reset. */
    ERROR_STATE      = -4,                              /**< Decoder is in an error condition. */
    USER_DEFINED_0   = 0,                               /**< First user-defined value. */
    USER_DEFINED_MAX = 128,                             /**< Maximum user-defined value. */
};

/** Returns true for COMPLETED_STATE or FINAL_STATE. */
bool isDone(State st);

/** Defines mapping between code points and code values.
 *
 *  A code point represents one character of a coded character set, such as one character of approximately 1.4 million
 *  distinct Unicode characters.  The CharacterEncodingForm (CEF) is responsible for converting a code point to a sequence
 *  of one or more code values, or vice versa.  Each code value, which may be multiple bytes, is eventually encoded into a
 *  sequence of octets by the @ref CharacterEncodingScheme (CES). */
class CharacterEncodingForm: public Sawyer::SharedObject {
protected:
    State state_;
public:
    CharacterEncodingForm(): state_(INITIAL_STATE) {}
    virtual ~CharacterEncodingForm() {}

    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<CharacterEncodingForm> Ptr;

    /** Create a new encoder from this one. */
    virtual Ptr clone() const = 0;

    /** Name of encoder. */
    virtual std::string name() const = 0;

    /** Encode a code point into a sequence of one or more code values.
     *
     *  For instance, an ecoder for UTF-16 will encode a code point into one or more values in the range 0 through (2^16)-1. */
    virtual CodeValues encode(CodePoint) = 0;

    /** Decoder state. */
    State state() const { return state_; }

    /** Decode one code value.
     *
     *  Processes a single code value and updates the decoder state machine. Returns the decoder's new state. See documentation
     *  for @ref State for restrictions on state transitions. */
    virtual State decode(CodeValue) = 0;

    /** Consume a decoded code point.
     *
     *  The decoder must be in the FINAL_STATE or COMPLETED_STATE, and upon return will be in the INITIAL_STATE. */
    virtual CodePoint consume() = 0;

    /** Reset the decoder state machine. */
    virtual void reset() = 0;
};

/** A no-op character encoding form.
 *
 *  Encodes code points to code values and vice versa such that code points are equal to code values. */
class NoopCharacterEncodingForm: public CharacterEncodingForm {
    CodePoint cp_;
protected:
    NoopCharacterEncodingForm(): cp_(0) {}
public:
    typedef Sawyer::SharedPointer<NoopCharacterEncodingForm> Ptr;
    static Ptr instance() { return Ptr(new NoopCharacterEncodingForm); }
    virtual CharacterEncodingForm::Ptr clone() const ROSE_OVERRIDE { return Ptr(new NoopCharacterEncodingForm(*this)); }
    virtual std::string name() const ROSE_OVERRIDE { return "no-op"; }
    virtual CodeValues encode(CodePoint cp) ROSE_OVERRIDE;
    virtual State decode(CodeValue) ROSE_OVERRIDE;
    virtual CodePoint consume() ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;
};

/** Returns a new no-op character encoding form. */
NoopCharacterEncodingForm::Ptr noopCharacterEncodingForm();

/** UTF-8 character encoding form.
 *
 *  Encodes each code point as one to six 8-bit code values. */
class Utf8CharacterEncodingForm: public CharacterEncodingForm {
    CodePoint cp_;
protected:
    Utf8CharacterEncodingForm(): cp_(0) {}
public:
    typedef Sawyer::SharedPointer<Utf8CharacterEncodingForm> Ptr;
    static Ptr instance() { return Ptr(new Utf8CharacterEncodingForm); }
    virtual CharacterEncodingForm::Ptr clone() const ROSE_OVERRIDE { return Ptr(new Utf8CharacterEncodingForm(*this)); }
    virtual std::string name() const ROSE_OVERRIDE { return "UTF-8"; }
    virtual CodeValues encode(CodePoint cp) ROSE_OVERRIDE;
    virtual State decode(CodeValue) ROSE_OVERRIDE;
    virtual CodePoint consume() ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;
};

/** Returns a new UTF-8 character encoding form. */
Utf8CharacterEncodingForm::Ptr utf8CharacterEncodingForm();

/** UTF-16 character encoding form.
 *
 *  Encodes each code point as one or two 16-bit code values. */
class Utf16CharacterEncodingForm: public CharacterEncodingForm {
    CodePoint cp_;
protected:
    Utf16CharacterEncodingForm(): cp_(0) {}
public:
    typedef Sawyer::SharedPointer<Utf16CharacterEncodingForm> Ptr;
    static Ptr instance() { return Ptr(new Utf16CharacterEncodingForm); }
    virtual CharacterEncodingForm::Ptr clone() const ROSE_OVERRIDE { return Ptr(new Utf16CharacterEncodingForm(*this)); }
    virtual std::string name() const ROSE_OVERRIDE { return "UTF-16"; }
    virtual CodeValues encode(CodePoint cp) ROSE_OVERRIDE;
    virtual State decode(CodeValue) ROSE_OVERRIDE;
    virtual CodePoint consume() ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;
};

/** Returns a new UTF-16 character encoding form. */
Utf16CharacterEncodingForm::Ptr utf16CharacterEncodingForm();

/** Defines the mapping between code values and octets.
 *
 *  A code value (one or more of which compose a code point, or a single character in a coded character set), is encoded as
 *  one or more octets.  For instance, a UTF-16 code value will be converted to two octets in big or little endian order
 *  depending on the character encoding scheme. */
class CharacterEncodingScheme: public Sawyer::SharedObject {
protected:
    State state_;
public:
    CharacterEncodingScheme(): state_(INITIAL_STATE) {}
    virtual ~CharacterEncodingScheme() {}

    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<CharacterEncodingScheme> Ptr;

    /** Create a new copy of this encoder. */
    virtual Ptr clone() const = 0;

    /** Name of encoder. */
    virtual std::string name() const = 0;

    /** Encode a code value into a sequence of octets. For instance, an encoder for UTF-16 will encode a code value into
     *  two octets. */
    virtual Octets encode(CodeValue) = 0;

    /** Decoder state. */
    State state() const { return state_; }

    /** Decode one octet.
     *
     *  Processes a single octet and updates the decoder state machine. Returns the decoder's new state. See documentation for
     *  @ref State for restrictions on state transitions. */
    virtual State decode(Octet) = 0;

    /** Consume a decoded code value.
     *
     *  The decoder must be in the FINAL_STATE or COMPLETED_STATE and upon return will be in the INITIAL_STATE. */
    virtual CodeValue consume() = 0;

    /** Reset the decoder state machine. */
    virtual void reset() = 0;
};

/** Basic character encoding scheme.
 *
 *  This character encoding scheme converts code value to a sequence of octets in big- or little-endian order, and vice
 *  versa. It needs to know the number of octets per code value, and the byte order of the octets per code value is larger
 *  than one. */
class BasicCharacterEncodingScheme: public CharacterEncodingScheme {
    size_t octetsPerValue_;
    ByteOrder::Endianness sex_;
    CodeValue cv_;
protected:
    BasicCharacterEncodingScheme(size_t octetsPerValue, ByteOrder::Endianness sex)
        : octetsPerValue_(octetsPerValue), sex_(sex), cv_(0) {
        ASSERT_require(1==octetsPerValue || sex!=ByteOrder::ORDER_UNSPECIFIED);
        ASSERT_require(octetsPerValue <= sizeof(CodeValue));
    }
public:
    static Ptr instance(size_t octetsPerValue, ByteOrder::Endianness sex = ByteOrder::ORDER_UNSPECIFIED) {
        return Ptr(new BasicCharacterEncodingScheme(octetsPerValue, sex));
    }
    virtual Ptr clone() const ROSE_OVERRIDE {
        return Ptr(new BasicCharacterEncodingScheme(*this));
    }
    virtual std::string name() const ROSE_OVERRIDE;
    virtual Octets encode(CodeValue) ROSE_OVERRIDE;
    virtual State decode(Octet) ROSE_OVERRIDE;
    virtual CodeValue consume() ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;
};

/** Returns a new basic character encoding scheme. */
BasicCharacterEncodingScheme::Ptr basicCharacterEncodingScheme(size_t octetsPerValue,
                                                               ByteOrder::Endianness sex = ByteOrder::ORDER_UNSPECIFIED);

/** Encoding for the length of a string.
 *
 *  Strings that are length-encoded must specify a length encoding scheme that gives the length of the string measured in
 *  code points. */
class LengthEncodingScheme: public Sawyer::SharedObject {
protected:
    State state_;
public:
    LengthEncodingScheme(): state_(INITIAL_STATE) {}
    virtual ~LengthEncodingScheme() {}

    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<LengthEncodingScheme> Ptr;

    /** Create a new copy of this encoder. */
    virtual Ptr clone() const = 0;

    /** Name of encoder. */
    virtual std::string name() const = 0;

    /** Encode a length into a sequence of octets. */
    virtual Octets encode(size_t) = 0;

    /** Decoder state. */
    State state() const { return state_; }

    /** Decode one octet.
     *
     *  Processes a single octet and updates the decoder state machine.  Returns the decoder's new state. See documentation for
     *  @ref State for restrictions on state transitions. */
    virtual State decode(Octet) = 0;

    /** Consume a decoded length.
     *
     *  The decoder must be in the FINAL_STATE or COMPLETE_STATE, and upon return will be in the INITIAL_STATE. */
    virtual size_t consume() = 0;

    /** Reset the decoder state machine. */
    virtual void reset() = 0;
};

/** Basic length encoding scheme.
 *
 *  This length encoding scheme converts a length to a sequence of octets in big- or little-endian order, and vice
 *  versa. It needs to know the number of octets per length value, and the byte order of the octets if the length is
 *  greater than one. */
class BasicLengthEncodingScheme: public LengthEncodingScheme {
    size_t octetsPerValue_;
    ByteOrder::Endianness sex_;
    size_t length_;
protected:
    BasicLengthEncodingScheme(size_t octetsPerValue, ByteOrder::Endianness sex)
        : octetsPerValue_(octetsPerValue), sex_(sex), length_(0) {
        ASSERT_require(1==octetsPerValue || sex!=ByteOrder::ORDER_UNSPECIFIED);
        ASSERT_require(octetsPerValue <= sizeof(size_t));
    }
public:
    static Ptr instance(size_t octetsPerValue, ByteOrder::Endianness sex = ByteOrder::ORDER_UNSPECIFIED) {
        return Ptr(new BasicLengthEncodingScheme(octetsPerValue, sex));
    }
    virtual Ptr clone() const ROSE_OVERRIDE {
        return Ptr(new BasicLengthEncodingScheme(*this));
    }
    virtual std::string name() const ROSE_OVERRIDE;
    virtual Octets encode(size_t) ROSE_OVERRIDE;
    virtual State decode(Octet) ROSE_OVERRIDE;
    virtual size_t consume() ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;
};

/** Returns a new basic length encoding scheme. */
BasicLengthEncodingScheme::Ptr basicLengthEncodingScheme(size_t octetsPerValue,
                                                         ByteOrder::Endianness sex = ByteOrder::ORDER_UNSPECIFIED);

/** Valid code point predicate.
 *
 *  This predicate tests that the specified code point is valid for a string. */
class CodePointPredicate: public Sawyer::SharedObject {
public:
    virtual ~CodePointPredicate() {}

    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<CodePointPredicate> Ptr;

    /** Name of predicate. */
    virtual std::string name() const = 0;

    /** Predicate. */
    virtual bool isValid(CodePoint) = 0;
};

/** ASCII valid code points.
 *
 *  Returns true if the code point is a printable US-ASCII character.  Printable characters are seven-bit code points for
 *  which C's @c isprint predicate returns true (anything but control characters). */
class PrintableAscii: public CodePointPredicate {
protected:
    PrintableAscii() {}
public:
    static Ptr instance() {
        return Ptr(new PrintableAscii);
    }
    virtual std::string name() const ROSE_OVERRIDE { return "printable ASCII"; }
    virtual bool isValid(CodePoint) ROSE_OVERRIDE;
};

/** Returns a new printable ASCII predicate. */
PrintableAscii::Ptr printableAscii();

/** Matches any code point.
 *
 *  Returns true for all code points. */
class AnyCodePoint: public CodePointPredicate {
protected:
    AnyCodePoint() {}
public:
    static Ptr instance() { return Ptr(new AnyCodePoint); }
    virtual std::string name() const ROSE_OVERRIDE { return "any code point"; }
    virtual bool isValid(CodePoint) ROSE_OVERRIDE { return true; }
};

/** Returns a new predicate that matches all code points. */
AnyCodePoint::Ptr anyCodePoint();

/** String encoding scheme.
 *
 *  A string encoding scheme indicates how a string (sequence of code points) is encoded as a sequence of octets and vice
 *  versa. */
class StringEncodingScheme: public Sawyer::SharedObject {
protected:
    State state_;                                       // decoding state
    CodePoints codePoints_;                             // unconsumed code points
    size_t nCodePoints_;                                // number of code points decoded since reset
    CharacterEncodingForm::Ptr cef_;
    CharacterEncodingScheme::Ptr ces_;
    CodePointPredicate::Ptr cpp_;

protected:
    StringEncodingScheme(): state_(INITIAL_STATE), nCodePoints_(0) {}

    StringEncodingScheme(const CharacterEncodingForm::Ptr &cef, const CharacterEncodingScheme::Ptr &ces,
                         const CodePointPredicate::Ptr &cpp)
        : cef_(cef), ces_(ces), cpp_(cpp) {}

public:
    virtual ~StringEncodingScheme() {}

    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<StringEncodingScheme> Ptr;

    /** Name of encoding */
    virtual std::string name() const = 0;

    /** Create a new copy of this encoder. */
    virtual Ptr clone() const = 0;

    /** Encode a string into a sequence of octets. */
    virtual Octets encode(const CodePoints&) = 0;

    /** Decoder state. */
    State state() const { return state_; }

    /** Decode one octet.
     *
     *  Processes a single octet and updates the decoder state machine.  Returns the new state. See documentation for @ref
     *  State for restrictions on state transitions. */
    virtual State decode(Octet) = 0;

    /** Consume pending decoded code points.
     *
     *  Returns code points that haven't been consume yet, and then removes them from the decoder.  This can be called from any
     *  state because we want the caller to be able to consume code points as they're decoded, which is a little bit different
     *  than how @c consume methods operate in the decoders that return scalar values. A @ref reset will discard pending code
     *  points. */
    CodePoints consume();

    /** Return pending decoded code points without consuming them. */
    const CodePoints& codePoints() const { return codePoints_; }

    /** Number of code points decoded since reset. */
    size_t length() const { return nCodePoints_; }

    /** Reset the state machine to an initial state. */
    virtual void reset();

    /** Property: Character encoding format.
     *
     *  The character encoding format is responsible for converting each code point to a sequence of code values. For instance,
     *  a UTF-16 encoding will convert each code point (a number between zero and about 1.2 million) into a sequence of
     *  16-bit code values.  Each code value will eventually be converted to a pair of octets by the character encoding
     *  scheme.
     *
     * @{ */
    CharacterEncodingForm::Ptr characterEncodingForm() const { return cef_; }
    void characterEncodingForm(const CharacterEncodingForm::Ptr &cef) { cef_ = cef; }
    /** @} */

    /** Property: Character encoding scheme.
     *
     *  The character encoding scheme is responsible for converting each code value to a sequence of one or more octets. The
     *  code value is part of a sequence of code values generated by the character encoding format for a single code point. For
     *  instance, a character encoding scheme for UTF-16 will need to know whether the octets are stored in bit- or
     *  little-endian order.
     *
     * @{ */
    CharacterEncodingScheme::Ptr characterEncodingScheme() const { return ces_; }
    void characterEncodingScheme(const CharacterEncodingScheme::Ptr &ces) { ces_ = ces; }
    /** @} */

    /** Property: Code point predicate.
     *
     *  The code point predicate tests whether a specific code point is allowed as part of a string.  For instance, when
     *  decoding NUL-terminated ASCII strings one might want to consider only those strings that contain printable characters
     *  and white space in order to limit the number of false positives when searching for strings in memory.
     *
     * @{ */
    CodePointPredicate::Ptr codePointPredicate() const { return cpp_; }
    void codePointPredicate(const CodePointPredicate::Ptr &cpp) { cpp_ = cpp; }
    /** @} */
};

/** Length-prefixed string encoding scheme.
 *
 *  A string encoding where the octets for the characters are prefixed with an encoded length. */
class LengthEncodedString: public StringEncodingScheme {
    LengthEncodingScheme::Ptr les_;
    Sawyer::Optional<size_t> declaredLength_;           // decoded length
protected:
    LengthEncodedString(const LengthEncodingScheme::Ptr &les, const CharacterEncodingForm::Ptr &cef,
                        const CharacterEncodingScheme::Ptr &ces, const CodePointPredicate::Ptr &cpp)
        : StringEncodingScheme(cef, ces, cpp), les_(les) {}
public:
    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<LengthEncodedString> Ptr;

    static Ptr instance(const LengthEncodingScheme::Ptr &les, const CharacterEncodingForm::Ptr &cef,
                        const CharacterEncodingScheme::Ptr &ces, const CodePointPredicate::Ptr &cpp) {
        return Ptr(new LengthEncodedString(les, cef, ces, cpp));
    }
    virtual StringEncodingScheme::Ptr clone() const ROSE_OVERRIDE {
        LengthEncodingScheme::Ptr les = les_->clone();
        CharacterEncodingForm::Ptr cef = cef_->clone();
        CharacterEncodingScheme::Ptr ces = ces_->clone();
        CodePointPredicate::Ptr cpp = cpp_;             // not cloned since they have no state
        LengthEncodedString *inst = new LengthEncodedString(les, cef, ces, cpp);
        inst->declaredLength_ = declaredLength_;
        return Ptr(inst);
    }
    virtual std::string name() const ROSE_OVERRIDE;
    virtual Octets encode(const CodePoints&) ROSE_OVERRIDE;
    virtual State decode(Octet) ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;

    /** Returns the declared length, if any.
     *
     *  The declared length is the value of the decoded length prefix, not necessarily the number of code points that have been
     *  decoded.  This can be called from any state except it will always return nothing in the INITIAL_STATE. Therefore, this
     *  method should be called prior to the @ref consume call. */
    Sawyer::Optional<size_t> declaredLength() const { return declaredLength_; }

    /** Property: Lengh encoding scheme.
     *
     *  The length encoding scheme is responsible for encoding the string length as a sequence of octets.
     *
     * @{ */
    LengthEncodingScheme::Ptr lengthEncodingScheme() const { return les_; }
    void lengthEncodingScheme(const LengthEncodingScheme::Ptr &les) { les_ = les; }
    /** @} */
};

/** Returns a new length-prefixed string encoder. */
LengthEncodedString::Ptr lengthEncodedString(const LengthEncodingScheme::Ptr &les, const CharacterEncodingForm::Ptr &cef,
                                             const CharacterEncodingScheme::Ptr &ces, const CodePointPredicate::Ptr &cpp);

/** Returns a new encoder for length-encoded printable ASCII strings. A byte order must be specified for length encodings
 *  larger than a single byte. */
LengthEncodedString::Ptr lengthEncodedPrintableAscii(size_t lengthSize,
                                                     ByteOrder::Endianness order = ByteOrder::ORDER_UNSPECIFIED);

/** Returns a new encoder for multi-byte length-encoded printable ASCII strings. */
LengthEncodedString::Ptr lengthEncodedPrintableAsciiWide(size_t lengthSize, ByteOrder::Endianness order, size_t charSize);

/** Terminated string encoding scheme.
 *
 *  A string whose character octets are followed by octets for a special code point that marks the end of the string but is
 *  not included as part of the string's characters.  An example is C-style NUL-terminated ASCII. */
class TerminatedString: public StringEncodingScheme {
    CodePoints terminators_;
    Sawyer::Optional<CodePoint> terminated_;            // decoded termination
protected:
    TerminatedString(const CharacterEncodingForm::Ptr &cef, const CharacterEncodingScheme::Ptr &ces,
                     const CodePointPredicate::Ptr &cpp, const CodePoints &terminators)
        : StringEncodingScheme(cef, ces, cpp), terminators_(terminators) {}
public:
    /** Shared ownership pointer. */
    typedef Sawyer::SharedPointer<TerminatedString> Ptr;

    static Ptr instance(const CharacterEncodingForm::Ptr &cef, const CharacterEncodingScheme::Ptr &ces,
                        const CodePointPredicate::Ptr &cpp, const CodePoints &terminators) {
        return Ptr(new TerminatedString(cef, ces, cpp, terminators));
    }
    static Ptr instance(const CharacterEncodingForm::Ptr &cef, const CharacterEncodingScheme::Ptr &ces,
                        const CodePointPredicate::Ptr &cpp, CodePoint terminator = 0) {
        return Ptr(new TerminatedString(cef, ces, cpp, CodePoints(1, terminator)));
    }
    virtual StringEncodingScheme::Ptr clone() const ROSE_OVERRIDE {
        CharacterEncodingForm::Ptr cef = cef_->clone();
        CharacterEncodingScheme::Ptr ces = ces_->clone();
        CodePointPredicate::Ptr cpp = cpp_;             // not cloned since they have no state
        TerminatedString *inst = new TerminatedString(cef, ces, cpp, terminators_);
        inst->terminated_ = terminated_;
        return Ptr(inst);
    }
    virtual std::string name() const ROSE_OVERRIDE;
    virtual Octets encode(const CodePoints&) ROSE_OVERRIDE;
    virtual State decode(Octet) ROSE_OVERRIDE;
    virtual void reset() ROSE_OVERRIDE;

    /** Returns the decoded termination character, if any.
     *
     *  This can be called from any state except it will always return nothing in the INITIAL_STATE. Therefore, this method
     *  should be called prior to the @ref consume call. */
    Sawyer::Optional<CodePoint> terminated() const { return terminated_; }

    /** Property: string termination code points.
     *
     *  A list of code points (characters) that cause a string to be terminated.  When decoding a string, if a terminating code
     *  point is encountered then the string ends at the previous code point even if the terminating code point also satisfies
     *  the code point predicate.
     *
     * @{ */
    const CodePoints& terminators() const { return terminators_; }
    CodePoints& terminators() { return terminators_; }
    /** @} */
};

/** Returns a new encoder for NUL-terminated printable ASCII strings. */
TerminatedString::Ptr nulTerminatedPrintableAscii();

/** Returns a new encoder for multi-byte NUL-terminated printable ASCII strings. */
TerminatedString::Ptr nulTerminatedPrintableAsciiWide(size_t charSize, ByteOrder::Endianness order);

/** An encoder plus interval.
 *
 *  Represents a string by specifying the encoding and an interval of virtual addresses where the encoded octets are
 *  stored. */
class EncodedString {
    StringEncodingScheme::Ptr encoder_;             // how string is encoded
    AddressInterval where_;                         // where encoded string is located
public:
    EncodedString() {}
    EncodedString(const StringEncodingScheme::Ptr &encoder, const AddressInterval &where)
        : encoder_(encoder), where_(where) {}

    /** Information about the string encoding. */
    StringEncodingScheme::Ptr encoder() const { return encoder_; }

    /** Where the string is located in memory. */
    const AddressInterval& where() const { return where_; }

    /** Starting address of string in memory. */
    rose_addr_t address() const { return where_.least(); }

    /** Size of encoded string in bytes. */
    size_t size() const { return where_.size(); }

    /** Length of encoded string in code points. */
    size_t length() const { return encoder_->length(); }

    /** Code points associated with the string.
     *
     *  If code points have been consumed then they may be partly or fully absent from the decoder. */
    const CodePoints& codePoints() const { return encoder_->codePoints(); }

    /** Return code points as a C++ std::string.
     *
     *  This truncates each code point to eight bits. */
    std::string narrow() const;

    /** Return code points as a C++ std::wstring. */
    std::wstring wide() const;

    /** Decodes the string from memory.
     *
     *  A string need not store its code points, in which case this method can decode them from memory.  The memory should be
     *  the same as when the string was originally found, otherwise an std::runtime_error might be thrown. */
    void decode(const MemoryMap&);
};

/** Class holding settings and methods for finding strings. */
class StringFinder {
    std::vector<StringEncodingScheme::Ptr> encoders_;
    size_t minLength_, maxLength_;
    size_t maxOverlap_;
    bool discardCodePoints_;
    bool keepOnlyLongest_;
public:
    StringFinder()
        : minLength_(5), maxLength_(-1), maxOverlap_(8), discardCodePoints_(false), keepOnlyLongest_(true) {}

    /** Property: minimum string length.
     *
     *  Strings will be found only if they contain at least some minimum number of characters.
     *
     * @{ */
    size_t minLength() const { return minLength_; }
    void minLength(size_t n) { minLength_ = n; }
    /** @} */

    /** Property: maximum string length.
     *
     *  Strings will be found only if they don't contain more than some maximum number of characters.
     *
     * @{ */
    size_t maxLength() const { return maxLength_; }
    void maxLength(size_t n) { maxLength_ = n; }
    /** @} */

    /** Property: allow overlapping strings.
     *
     *  The number of strings that can overlap at a single address per encoder.  For instance, for C-style NUL-terminated ASCII
     *  strings encoded as bytes, if memory contains the consecutive values 'a', 'n', 'i', 'm', 'a', 'l', '\0' then up to seven
     *  strings are possible: "animal", "nimal", "imal", "mal", "al", "l", and "".  If the maximum overlap is set to three then
     *  only "animal", "nimal", and "imal" are found. Setting the maximum overlap to zero has the same effect as setting it to
     *  one: no overlapping is allowed.  The overlap limits are applied before results are pruned based on length, so if the
     *  minimum legnth is four, the "imal" and shorter strings won't be found even though they are decoded under the covers.
     *
     *  A maximum overlap of at least two is recommended if two-byte-per-character encoding is used when detecting
     *  NUL-terminated ASCII strings. The reason is that one decoder will be active at one address while another decoder is
     *  desired for the next address; then if the first address proves to not be part of a string, the second address can still
     *  be detected as a string.  Similarly, a maximum overlap of at least four is recommended for four-byte-per-character
     *  encodings.  Length-encoded strings will have similar issues.
     *
     * @{ */
    size_t maxOverlap() const { return maxOverlap_; }
    void maxOverlap(size_t n) { maxOverlap_ = std::max(n, size_t(1)); }
    /** @} */

    /** Property: discard code points.
     *
     *  If this property is set, then the process of decoding strings does not actually store the code points (characters) of
     *  the string.  This is useful when searching for lots of strings to reduce the amount of memory required. A string can be
     *  decoded again later if the code points are needed.
     *
     * @{ */
    bool discardCodePoints() const { return discardCodePoints_; }
    void discardCodePoints(bool b) { discardCodePoints_ = b; }
    /** @} */

    /** Property: keep only longest non-overlapping strings.
     *
     *  If set, then only the longest detected strings are kept.  The algorithm sorts all detected strings by decreasing
     *  length, then removes any string whose memory addresses overlap with any prior string in the list.
     *
     * @{ */
    bool keepOnlyLongest() const { return keepOnlyLongest_; }
    void keepOnlyLongest(bool b) { keepOnlyLongest_ = b; }
    /** @} */

    /** Property: list of string encodings.
     *
     *  When searching for strings, this analysis must know what kinds of strings to look for, and does that with a vector of
     *  pointers to encoders.  The default is an empty vector, in which no strings will be found.
     *
     * @{ */
    void encoder(const StringEncodingScheme::Ptr &encoder) {
        ASSERT_not_null(encoder);
        encoders_.push_back(encoder);
    }
    const std::vector<StringEncodingScheme::Ptr>& encoders() const {
        return encoders_;
    }
    std::vector<StringEncodingScheme::Ptr>& encoders() {
        return encoders_;
    }
    /** @} */

    /** Inserts common encodings.
     *
     *  Inserts the following string encodings into the analysis:
     *
     *  @li NUL-terminated, byte-encoded, printable ASCII characters.
     *  @li NUL-terminated, 16-bit encoded, printable ASCII characters.
     *  @li NUL-terminated, 32-bit encoded, printable ASCII characters.
     *  @li 2-byte length-prefixed, byte encoded, printable ASCII characters.
     *  @li 4-byte length-prefixed, byte encoded, printable ASCII characters.
     *  @li 2-byte length-prefixed, 16-bit encoded, printable ASCII characters.
     *  @li 4-byte length-prefixed, 16-bit encoded, printable ASCII characters.
     *  @li 4-byte length-prefixed, 32-bit encoded, printable ASCII characters.
     *
     *  The specified endianness is used for all multi-byte values. */
    void insertCommonEncoders(ByteOrder::Endianness);

    /** Finds strings by searching memory.
     *
     *  The memory constraints indicate where to search for strings, and the properties of this StringFinder class determine
     *  how to find strings. Specifically, this class must have at least one encoding registered in order to find anything (see
     *  @ref encoders).
     *
     *  The search progresses by looking at each possible starting address using each registered encoding. The algorithm reads
     *  each byte from memory only one time, simultaneously attempting all encoders.  If the MemoryMap constraint contains an
     *  anchor point (e.g., @ref MemoryMap::at) then only strings starting at the specified address are returned.
     *
     *  Example 1: Find all C-style, NUL-terminated, ASCII strings contaiing only printable characters (no control characters)
     *  and containing at least five characters but not more than 31 (not counting the NUL terminator).  Make sure that the
     *  string is in memory that is readable but not writable, and don't allow strings to overlap one another (i.e., "foobar"
     *  and "bar" cannot share their last for bytes):
     *
     * @code
     *  using namespace rose::BinaryAnalysis::String;
     *  MemoryMap map = ...;
     *  StringFinder sf;
     *  sf.encoder(nulTerminatedPrintableAscii());
     *  sf.minLength(5);
     *  sf.maxLength(31);
     *  sf.allowOverlap(false);
     *  std::vector<EncodedString> strings = sf.find(map.require(MemoryMap::READABLE).prohibit(MemoryMap::WRITABLE));
     * @endcode */
    std::vector<EncodedString> find(const MemoryMap::ConstConstraints&, Sawyer::Container::MatchFlags flags=0);
};

} // namespace
} // namespace
} // namespace

#endif
