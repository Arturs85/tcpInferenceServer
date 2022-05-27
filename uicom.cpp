#include "uicom.hpp"
#include <signal.h>

#include <iostream>
#include <thread>


const std::string UiCom::TAG = "[uicom] ";


void UiCom::sendCount(int count)
{
  std::string com1 = "curl -X PATCH -H \"Content-Type: application/json\" -d '{\"name\":\"simple2\",\"fields\":{\"singleField\":{\"integerValue\":";
  std::string com2 = "}}}' 'https://content-firestore.googleapis.com/v1beta1/projects/ceilingcounterui/databases/(default)/documents/containera/simple2?currentDocument.exists=true&alt=json'";
 std::string com = com1+std::to_string(count)+com2;
 system(com.data());
}
