#ifndef C_JSON_SERIALIZER_H
#define C_JSON_SERIALIZER_H

#include "../libs01.hpp"
#include "jsoncpp/json/json.h"

class ijson_serializable {
  public:
    virtual ~ijson_serializable() = default;
    virtual void json_serialize(Json::Value &root) const = 0;
    virtual void json_deserialize(Json::Value &root) = 0;
};

class c_json_serializer {
  public:
    static bool serialize(const ijson_serializable &obj, std::string& output);
    static bool deserialize(ijson_serializable &obj, const std::string &input);
  private:
    c_json_serializer() = default;
};

#endif // C_JSON_SERIALIZER_H
