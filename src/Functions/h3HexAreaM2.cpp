#include "config_functions.h"

#if USE_H3

#include <Columns/ColumnsNumber.h>
#include <DataTypes/DataTypesNumber.h>
#include <Functions/FunctionFactory.h>
#include <Functions/IFunction.h>
#include <IO/WriteHelpers.h>
#include <Common/typeid_cast.h>
#include <base/range.h>

#include <constants.h>
#include <h3api.h>


namespace DB
{
namespace ErrorCodes
{
    extern const int ILLEGAL_TYPE_OF_ARGUMENT;
    extern const int ARGUMENT_OUT_OF_BOUND;
}

namespace
{

class FunctionH3HexAreaM2 : public IFunction
{
public:
    static constexpr auto name = "h3HexAreaM2";

    static FunctionPtr create(ContextPtr) { return std::make_shared<FunctionH3HexAreaM2>(); }

    std::string getName() const override { return name; }

    size_t getNumberOfArguments() const override { return 1; }
    bool useDefaultImplementationForConstants() const override { return true; }
    bool isSuitableForShortCircuitArgumentsExecution(const DataTypesWithConstInfo & /*arguments*/) const override { return false; }

    DataTypePtr getReturnTypeImpl(const DataTypes & arguments) const override
    {
        const auto * arg = arguments[0].get();
        if (!WhichDataType(arg).isUInt8())
            throw Exception(
                ErrorCodes::ILLEGAL_TYPE_OF_ARGUMENT,
                "Illegal type {} of argument {} of function {}. Must be UInt8",
                arg->getName(), 1, getName());

        return std::make_shared<DataTypeFloat64>();
    }

    ColumnPtr executeImpl(const ColumnsWithTypeAndName & arguments, const DataTypePtr &, size_t input_rows_count) const override
    {
        const auto * col_hindex = arguments[0].column.get();

        auto dst = ColumnVector<Float64>::create();
        auto & dst_data = dst->getData();
        dst_data.resize(input_rows_count);

        for (size_t row = 0; row < input_rows_count; ++row)
        {
            const UInt64 resolution = col_hindex->getUInt(row);
            if (resolution > MAX_H3_RES)
                throw Exception(
                    ErrorCodes::ARGUMENT_OUT_OF_BOUND,
                    "The argument 'resolution' ({}) of function {} is out of bounds because the maximum resolution in H3 library is ",
                    resolution, getName(), MAX_H3_RES);

            Float64 res = getHexagonAreaAvgM2(resolution);

            dst_data[row] = res;
        }

        return dst;
    }
};

}

void registerFunctionH3HexAreaM2(FunctionFactory & factory)
{
    factory.registerFunction<FunctionH3HexAreaM2>();
}

}

#endif
