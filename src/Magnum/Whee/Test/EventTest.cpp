/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <sstream>
#include <Corrade/TestSuite/Tester.h>
#include <Corrade/Utility/DebugStl.h>

#include "Magnum/Whee/Event.h"

namespace Magnum { namespace Whee { namespace Test { namespace {

struct EventTest: TestSuite::Tester {
    explicit EventTest();

    void debugPointer();

    void pointer();
};

EventTest::EventTest() {
    addTests({&EventTest::debugPointer,

              &EventTest::pointer});
}

void EventTest::debugPointer() {
    std::ostringstream out;
    Debug{&out} << Pointer::MouseMiddle << Pointer(0xde);
    CORRADE_COMPARE(out.str(), "Whee::Pointer::MouseMiddle Whee::Pointer(0xde)\n");
}

void EventTest::pointer() {
    PointerEvent event{Pointer::MouseMiddle};
    CORRADE_COMPARE(event.type(), Pointer::MouseMiddle);
    CORRADE_COMPARE(event.position(), Vector2{});
    CORRADE_VERIFY(!event.isAccepted());

    event.setPosition({1.0f, 2.0f});
    CORRADE_COMPARE(event.position(), (Vector2{1.0f, 2.0f}));

    event.setAccepted();
    CORRADE_VERIFY(event.isAccepted());

    event.setAccepted(false);
    CORRADE_VERIFY(!event.isAccepted());
}

}}}}

CORRADE_TEST_MAIN(Magnum::Whee::Test::EventTest)