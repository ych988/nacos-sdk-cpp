#include <iostream>
#include "NacosString.h"
#include "src/debug/DebugAssertion.h"

using namespace std;
using namespace nacos;

//Use a special variable NULLSTR as null in Java to pass null parameter
//But "" (zero-length string) will still be regarded as null
bool testStringEqual() {
    cout << "In testStringEqual" << endl;
    NacosString s2 = "";
    NacosString s3 = "Nacos";
    SHOULD_BE_TRUE(isNull_nacos(NULLSTR), "NULLSTR should be NULL");
    SHOULD_BE_TRUE(isNull_nacos(s2), "\"\" should be NULL");
    SHOULD_BE_FALSE(isNull_nacos(s3), "\"Nacos\" should not be NULL");

    return true;
}