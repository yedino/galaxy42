#include "c_json_serializer.hpp"

bool c_json_serializer::serialize(ijson_serializable* obj, std::string &output) {
   if (obj == NULL)
      return false;

   Json::Value serialize_root;
   obj->json_serialize(serialize_root);

   Json::StyledWriter writer;
   output = writer.write(serialize_root);

   return true;
}

bool c_json_serializer::deserialize(ijson_serializable* obj, std::string &input) {
   if (obj == NULL)
      return false;

   Json::Value deserialize_root;
   Json::Reader reader;

   if (!reader.parse(input, deserialize_root))
      return false;

   obj->json_deserialize(deserialize_root);

   return true;
}
