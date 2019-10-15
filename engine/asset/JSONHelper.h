#if !defined(JSONHELPER_H)

namespace JSONHelper
{
    Yostr GetString(const Value& value)
    {
        Yostr matstring = {};
        matstring.Length = value.GetStringLength();
        matstring = CreateStringFromLength((char*)value.GetString(), value.GetStringLength(), &StringsHandler::transient_string_memory);
        return matstring;
    }

    float GetFloat(const Value& values)
    {
        float results[1];
        int value_index = 0;

        for (auto& value : values.GetArray())
        {
            if(!value.IsDouble() || value_index > 3)
            {
                //Must be a float here if not better chweck whqt went wrong
                Assert(false);
            }
            results[value_index] = (float)value.GetDouble();
            value_index++;
        }
        float result = results[0];
        return result;
    }
    
    float4 GetFloat4(const Value& values)
    {
        float results[4];
        int value_index = 0;
        for (auto& value : values.GetArray())
        {
            if(!value.IsDouble() || value_index > 3)
            {
                //Must be a float here if not better chweck whqt went wrong
                Assert(false);
            }
            results[value_index] = (float)value.GetDouble();
            value_index++;
        }
        float4 result = float4(results[0],results[1],results[2],results[3]);
        return result;
    }
};

#define JSONHELPER_H
#endif
