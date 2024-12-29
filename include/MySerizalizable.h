#ifndef MYSERIALIZABLE_H
#define MYSERIALIZABLE_H
#include <utility/uuid.h>
struct UUIDFactory {
  static uuids::uuid generateUUID() {
    static UUIDFactory factory = UUIDFactory();
    int                clockCnt = std::chrono::system_clock::now().time_since_epoch().count();
    auto               id = factory._generator(std::to_string(clockCnt).c_str());
    return id;
  }

  UUIDFactory() : _generator(uuids::uuid::from_string("47183823-2574-4bfd-b411-99ed177d3e43").value()) {}
  uuids::uuid_name_generator _generator;
};
struct MySerializable {
  MySerializable() : _uuid(UUIDFactory::generateUUID()) {}
  uuids::uuid _uuid;
};

#endif // MYSERIALIZABLE_H