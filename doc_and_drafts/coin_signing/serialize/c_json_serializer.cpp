#include "c_json_serializer.hpp"

bool c_json_serializer::serialize(const ijson_serializable &obj, std::string &output) {
   Json::Value serialize_root;
   obj.json_serialize(serialize_root);

   Json::StyledWriter writer;
   output = writer.write(serialize_root);

   return true;
}

bool c_json_serializer::deserialize(ijson_serializable &obj, const std::string &input) {
   Json::Value deserialize_root;
   Json::Reader reader;

   if (!reader.parse(input, deserialize_root))
      return false;

   obj.json_deserialize(deserialize_root);

   return true;
}
