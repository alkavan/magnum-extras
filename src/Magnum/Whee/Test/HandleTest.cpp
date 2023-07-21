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

#include "Magnum/Whee/Handle.h"

namespace Magnum { namespace Whee { namespace Test { namespace {

struct HandleTest: TestSuite::Tester {
    explicit HandleTest();

    void layer();
    void layerInvalid();
    void debugLayer();

    void layerData();
    void layerDataInvalid();
    void debugLayerData();

    void data();
    void dataInvalid();
    void debugData();

    void node();
    void nodeInvalid();
    void debugNode();
};

HandleTest::HandleTest() {
    addTests({&HandleTest::layer,
              &HandleTest::layerInvalid,
              &HandleTest::debugLayer,

              &HandleTest::layerData,
              &HandleTest::layerDataInvalid,
              &HandleTest::debugLayerData,

              &HandleTest::data,
              &HandleTest::dataInvalid,
              &HandleTest::debugData,

              &HandleTest::node,
              &HandleTest::nodeInvalid,
              &HandleTest::debugNode});
}

void HandleTest::layer() {
    CORRADE_COMPARE(LayerHandle::Null, LayerHandle{});
    CORRADE_COMPARE(layerHandle(0, 0), LayerHandle{});
    CORRADE_COMPARE(layerHandle(0xab, 0x12), LayerHandle(0x12ab));
    CORRADE_COMPARE(layerHandle(0xff, 0xff), LayerHandle(0xffff));
    CORRADE_COMPARE(layerHandleId(LayerHandle::Null), 0);
    CORRADE_COMPARE(layerHandleId(LayerHandle(0x12ab)), 0xab);
    CORRADE_COMPARE(layerHandleGeneration(LayerHandle::Null), 0);
    CORRADE_COMPARE(layerHandleGeneration(LayerHandle(0x12ab)), 0x12);

    constexpr LayerHandle handle = layerHandle(0xab, 0x12);
    constexpr UnsignedInt id = layerHandleId(handle);
    constexpr UnsignedInt generation = layerHandleGeneration(handle);
    CORRADE_COMPARE(handle, LayerHandle(0x12ab));
    CORRADE_COMPARE(id, 0xab);
    CORRADE_COMPARE(generation, 0x12);
}

void HandleTest::layerInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    layerHandle(0x100, 0x1);
    layerHandle(0x1, 0x100);
    CORRADE_COMPARE(out.str(),
        "Whee::layerHandle(): expected index to fit into 8 bits and generation into 8, got 0x100 and 0x1\n"
        "Whee::layerHandle(): expected index to fit into 8 bits and generation into 8, got 0x1 and 0x100\n");
}

void HandleTest::debugLayer() {
    std::ostringstream out;
    Debug{&out} << LayerHandle::Null << layerHandle(0x12, 0xab);
    CORRADE_COMPARE(out.str(), "Whee::LayerHandle::Null Whee::LayerHandle(0x12, 0xab)\n");
}

void HandleTest::layerData() {
    CORRADE_COMPARE(LayerDataHandle::Null, LayerDataHandle{});
    CORRADE_COMPARE(layerDataHandle(0, 0), LayerDataHandle::Null);
    CORRADE_COMPARE(layerDataHandle(0xabcde, 0x123), LayerDataHandle(0x123abcde));
    CORRADE_COMPARE(layerDataHandle(0xfffff, 0xfff), LayerDataHandle(0xffffffff));
    CORRADE_COMPARE(layerDataHandleId(LayerDataHandle::Null), 0);
    CORRADE_COMPARE(layerDataHandleId(LayerDataHandle(0x123abcde)), 0xabcde);
    CORRADE_COMPARE(layerDataHandleGeneration(LayerDataHandle::Null), 0);
    CORRADE_COMPARE(layerDataHandleGeneration(LayerDataHandle(0x123abcde)), 0x123);

    constexpr LayerDataHandle handle = layerDataHandle(0xabcde, 0x123);
    constexpr UnsignedInt id = layerDataHandleId(handle);
    constexpr UnsignedInt generation = layerDataHandleGeneration(handle);
    CORRADE_COMPARE(handle, LayerDataHandle(0x123abcde));
    CORRADE_COMPARE(id, 0xabcde);
    CORRADE_COMPARE(generation, 0x123);
}

void HandleTest::layerDataInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    layerDataHandle(0x100000, 0x1);
    layerDataHandle(0x1, 0x1000);
    CORRADE_COMPARE(out.str(),
        "Whee::layerDataHandle(): expected index to fit into 20 bits and generation into 12, got 0x100000 and 0x1\n"
        "Whee::layerDataHandle(): expected index to fit into 20 bits and generation into 12, got 0x1 and 0x1000\n");
}

void HandleTest::debugLayerData() {
    std::ostringstream out;
    Debug{&out} << LayerDataHandle::Null << layerDataHandle(0x12345, 0xabc);
    CORRADE_COMPARE(out.str(), "Whee::LayerDataHandle::Null Whee::LayerDataHandle(0x12345, 0xabc)\n");
}

void HandleTest::data() {
    CORRADE_COMPARE(DataHandle::Null, DataHandle{});
    CORRADE_COMPARE(dataHandle(LayerHandle::Null, 0, 0), DataHandle::Null);
    CORRADE_COMPARE(dataHandle(LayerHandle(0x12ab), 0x34567, 0xcde), DataHandle(0x12abcde34567));
    CORRADE_COMPARE(dataHandle(LayerHandle(0xffff), 0xfffff, 0xfff), DataHandle(0xffffffffffff));
    CORRADE_COMPARE(dataHandle(LayerHandle::Null, LayerDataHandle::Null), DataHandle::Null);
    CORRADE_COMPARE(dataHandle(LayerHandle(0x12ab), LayerDataHandle(0xcde34567)), DataHandle(0x12abcde34567));
    CORRADE_COMPARE(dataHandleLayer(DataHandle::Null), LayerHandle::Null);
    CORRADE_COMPARE(dataHandleLayer(DataHandle(0x12abcde34567)), LayerHandle(0x12ab));
    CORRADE_COMPARE(dataHandleData(DataHandle::Null), LayerDataHandle::Null);
    CORRADE_COMPARE(dataHandleData(DataHandle(0x12abcde34567)), LayerDataHandle(0xcde34567));
    CORRADE_COMPARE(dataHandleLayerId(DataHandle::Null), 0);
    CORRADE_COMPARE(dataHandleLayerId(DataHandle(0x12abcde34567)), 0xab);
    CORRADE_COMPARE(dataHandleLayerGeneration(DataHandle::Null), 0);
    CORRADE_COMPARE(dataHandleLayerGeneration(DataHandle(0x12abcde34567)), 0x12);
    CORRADE_COMPARE(dataHandleId(DataHandle::Null), 0);
    CORRADE_COMPARE(dataHandleId(DataHandle(0x12abcde34567)), 0x34567);
    CORRADE_COMPARE(dataHandleGeneration(DataHandle::Null), 0);
    CORRADE_COMPARE(dataHandleGeneration(DataHandle(0x12abcde34567)), 0xcde);

    constexpr DataHandle handle1 = dataHandle(LayerHandle(0x12ab), 0x34567, 0xcde);
    constexpr DataHandle handle2 = dataHandle(LayerHandle(0x12ab), LayerDataHandle(0xcde34567));
    constexpr LayerHandle layer = dataHandleLayer(handle1);
    constexpr LayerDataHandle data = dataHandleData(handle1);
    constexpr UnsignedInt layerId = dataHandleLayerId(handle1);
    constexpr UnsignedInt layerGeneration = dataHandleLayerGeneration(handle1);
    constexpr UnsignedInt id = dataHandleId(handle1);
    constexpr UnsignedInt generation = dataHandleGeneration(handle1);
    CORRADE_COMPARE(handle1, DataHandle(0x12abcde34567));
    CORRADE_COMPARE(handle2, DataHandle(0x12abcde34567));
    CORRADE_COMPARE(layer, LayerHandle(0x12ab));
    CORRADE_COMPARE(data, LayerDataHandle(0xcde34567));
    CORRADE_COMPARE(layerId, 0xab);
    CORRADE_COMPARE(layerGeneration, 0x12);
    CORRADE_COMPARE(id, 0x34567);
    CORRADE_COMPARE(generation, 0xcde);
}

void HandleTest::dataInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    dataHandle(LayerHandle::Null, 0x100000, 0x1);
    dataHandle(LayerHandle::Null, 0x1, 0x1000);
    CORRADE_COMPARE(out.str(),
        "Whee::dataHandle(): expected index to fit into 20 bits and generation into 12, got 0x100000 and 0x1\n"
        "Whee::dataHandle(): expected index to fit into 20 bits and generation into 12, got 0x1 and 0x1000\n");
}

void HandleTest::debugData() {
    std::ostringstream out;
    Debug{&out} << DataHandle::Null << dataHandle(LayerHandle::Null, layerDataHandle(0xabcde, 0x12)) << dataHandle(layerHandle(0x34, 0x56), LayerDataHandle::Null) << dataHandle(layerHandle(0x34, 0x56), 0xabcde, 0x12);
    CORRADE_COMPARE(out.str(), "Whee::DataHandle::Null Whee::DataHandle(Null, {0xabcde, 0x12}) Whee::DataHandle({0x34, 0x56}, Null) Whee::DataHandle({0x34, 0x56}, {0xabcde, 0x12})\n");
}

void HandleTest::node() {
    CORRADE_COMPARE(NodeHandle::Null, NodeHandle{});
    CORRADE_COMPARE(nodeHandle(0, 0), NodeHandle::Null);
    CORRADE_COMPARE(nodeHandle(0xabcde, 0x123), NodeHandle(0x123abcde));
    CORRADE_COMPARE(nodeHandle(0xfffff, 0xfff), NodeHandle(0xffffffff));
    CORRADE_COMPARE(nodeHandleId(NodeHandle::Null), 0);
    CORRADE_COMPARE(nodeHandleId(NodeHandle(0x123abcde)), 0xabcde);
    CORRADE_COMPARE(nodeHandleGeneration(NodeHandle::Null), 0);
    CORRADE_COMPARE(nodeHandleGeneration(NodeHandle(0x123abcde)), 0x123);

    constexpr NodeHandle handle = nodeHandle(0xabcde, 0x123);
    constexpr UnsignedInt id = nodeHandleId(handle);
    constexpr UnsignedInt generation = nodeHandleGeneration(handle);
    CORRADE_COMPARE(handle, NodeHandle(0x123abcde));
    CORRADE_COMPARE(id, 0xabcde);
    CORRADE_COMPARE(generation, 0x123);
}

void HandleTest::nodeInvalid() {
    CORRADE_SKIP_IF_NO_DEBUG_ASSERT();

    std::ostringstream out;
    Error redirectError{&out};
    nodeHandle(0x100000, 0x1);
    nodeHandle(0x1, 0x1000);
    CORRADE_COMPARE(out.str(),
        "Whee::nodeHandle(): expected index to fit into 20 bits and generation into 12, got 0x100000 and 0x1\n"
        "Whee::nodeHandle(): expected index to fit into 20 bits and generation into 12, got 0x1 and 0x1000\n");
}

void HandleTest::debugNode() {
    std::ostringstream out;
    Debug{&out} << NodeHandle::Null << nodeHandle(0x12345, 0xabc);
    CORRADE_COMPARE(out.str(), "Whee::NodeHandle::Null Whee::NodeHandle(0x12345, 0xabc)\n");
}

}}}}

CORRADE_TEST_MAIN(Magnum::Whee::Test::HandleTest)