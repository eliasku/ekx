#include <unit.h>

#include <ek/serialize/serialize.hpp>

using namespace ek;

enum class TestFlagsCpp : uint8_t {
    One = 1,
    Two = 2,
    Three = 88
};

struct GlyphModel {
    int advanceWidth;

    template<typename S>
    void serialize(IO<S>& io) {
        io(advanceWidth);
    }
};

struct FontModel {
    uint16_t unitsPerEm;

    template<typename S>
    void serialize(IO<S>& io) {
        io(unitsPerEm);
    }
};

class Bar {
public:
    /* 4 */ float f1 = 2.0f;
    /* 4 */ int i4 = 4;
    /* 8 */ double d3 = 3;

    template<typename Stream>
    void serialize(IO<Stream>& io) {
        io(f1, i4, d3);
    }
};

template<typename T>
class Vector2 {
public:
    union {
        struct {
            T x;
            T y;
        };
        T values[2];
    };

    Vector2() = default;

    Vector2(T px, T py) : x{px}, y{py} {
    }
};

template<typename T>
struct declared_as_pod_type<Vector2<T>> : public std::true_type {
};

SUITE(cxx_serialize) {

    IT("Basic") {
        Bar bar{};

        output_memory_stream writeStream{200};
        IO writer{writeStream};

        auto ff = TestFlagsCpp::Three;
        writer(bar, ff);

        // Bar: 16
        // TestFlagsCpp: 1
        REQUIRE_EQ(writeStream.size(), 16 + 1);

        input_memory_stream read1{writeStream.data(), writeStream.size()};
        IO io{read1};

        Bar barLoaded{};
        TestFlagsCpp f{};
        io(barLoaded, f);
        REQUIRE_EQ(barLoaded.f1, 2.0f); // near
        REQUIRE_EQ(barLoaded.d3, 3); // near
        REQUIRE_EQ(barLoaded.i4, 4);
        REQUIRE(f == TestFlagsCpp::Three);

        input_memory_stream read2{writeStream.data(), writeStream.size()};
        IO<input_memory_stream> io2{read2};

        Bar barCorrupted{};
        io2(barCorrupted);
        REQUIRE_EQ(barLoaded.f1, (2.0f)); //
        REQUIRE_EQ(barLoaded.d3, (3)); //
        REQUIRE_EQ(barLoaded.i4, 4);
    }

    IT("Basic2") {

        GlyphModel glyph{-3};

        output_memory_stream writeStream{200};
        IO out{writeStream};
        out(glyph);

        GlyphModel s;

        input_memory_stream readStream{writeStream.data(), writeStream.size()};
        IO in{readStream};
        in(s);

        REQUIRE_EQ(s.advanceWidth, -3);
    }

    IT("Nesting") {
        GlyphModel glyph1{-3};
        GlyphModel glyph2{-3};
        FontModel font{2048u};

        output_memory_stream outs{200};
        IO out{outs};
        out(font);

        FontModel s;

        input_memory_stream readStream{outs.data(), outs.size()};
        IO in{readStream};
        in(s);
        REQUIRE_EQ(s.unitsPerEm, 2048);
    }

    IT("Enum") {
        output_memory_stream outs{200};
        IO writer{outs};

        TestFlagsCpp a{TestFlagsCpp::One};
        TestFlagsCpp b{TestFlagsCpp::Two};
        TestFlagsCpp c{TestFlagsCpp::Three};
        writer(a, b, c);

        REQUIRE_EQ(outs.size(), 3);

        input_memory_stream ins{outs.data(), outs.size()};
        IO io{ins};

        TestFlagsCpp i1;
        TestFlagsCpp i2;
        TestFlagsCpp i3;

        io(i1, i2, i3);

        REQUIRE(i1 == TestFlagsCpp::One);
        REQUIRE(i2 == TestFlagsCpp::Two);
        REQUIRE(i3 == TestFlagsCpp::Three);
    }
}
