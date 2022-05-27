#ifndef UICOM_HPP
#define UICOM_HPP



#include <string>


class UiCom{
public:
    static const std::string TAG;
    static void sendCount(int count);
    static void sendLocations(std::string locations);
};

#endif // UICOM_HPP
