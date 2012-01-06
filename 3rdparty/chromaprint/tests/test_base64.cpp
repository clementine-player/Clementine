#include <gtest/gtest.h>
#include "base64.h"
#include "test_utils.h"

using namespace std;
using namespace Chromaprint;

TEST(Base64, Base64Encode)
{
	ASSERT_EQ("eA", Base64Encode("x"));
	ASSERT_EQ("eHg", Base64Encode("xx"));
	ASSERT_EQ("eHh4", Base64Encode("xxx"));
	ASSERT_EQ("eHh4eA", Base64Encode("xxxx"));
	ASSERT_EQ("eHh4eHg", Base64Encode("xxxxx"));
	ASSERT_EQ("eHh4eHh4", Base64Encode("xxxxxx"));
	ASSERT_EQ("_-4", Base64Encode("\xff\xee"));
}

TEST(Base64, Base64Decode)
{
	ASSERT_EQ("x", Base64Decode("eA"));
	ASSERT_EQ("xx", Base64Decode("eHg"));
	ASSERT_EQ("xxx", Base64Decode("eHh4"));
	ASSERT_EQ("xxxx", Base64Decode("eHh4eA"));
	ASSERT_EQ("xxxxx", Base64Decode("eHh4eHg"));
	ASSERT_EQ("xxxxxx", Base64Decode("eHh4eHh4"));
	ASSERT_EQ("\xff\xee", Base64Decode("_-4"));
}

TEST(Base64, Base64EncodeLong)
{
	char original[] = {
		1, 0, 1, 207, 17, 181, 36, 18, 19, 37, 65, 15, 31, 197, 149, 161, 63, 33, 22,
		60, 141, 27, 202, 35, 184, 47, 254, 227, 135, 135, 11, 58, 139, 208, 65, 127,
		52, 167, 241, 31, 99, 182, 25, 159, 96, 70, 71, 160, 251, 168, 75, 132, 185,
		112, 230, 193, 133, 252, 42, 126, 66, 91, 121, 60, 135, 79, 24, 185, 210, 28,
		199, 133, 255, 240, 113, 101, 67, 199, 23, 225, 181, 160, 121, 140, 67, 123,
		161, 229, 184, 137, 30, 205, 135, 119, 70, 94, 252, 71, 120, 150
	};
	char encoded[] = "AQABzxG1JBITJUEPH8WVoT8hFjyNG8ojuC_-44eHCzqL0EF_NKfxH2O2GZ9gRkeg-6hLhLlw5sGF_Cp-Qlt5PIdPGLnSHMeF__BxZUPHF-G1oHmMQ3uh5biJHs2Hd0Ze_Ed4lg";
	ASSERT_EQ(encoded, Base64Encode(string(original, NELEMS(original))));
}

TEST(Base64, Base64DecodeLong)
{
	char original[] = {
		1, 0, 1, 207, 17, 181, 36, 18, 19, 37, 65, 15, 31, 197, 149, 161, 63, 33, 22,
		60, 141, 27, 202, 35, 184, 47, 254, 227, 135, 135, 11, 58, 139, 208, 65, 127,
		52, 167, 241, 31, 99, 182, 25, 159, 96, 70, 71, 160, 251, 168, 75, 132, 185,
		112, 230, 193, 133, 252, 42, 126, 66, 91, 121, 60, 135, 79, 24, 185, 210, 28,
		199, 133, 255, 240, 113, 101, 67, 199, 23, 225, 181, 160, 121, 140, 67, 123,
		161, 229, 184, 137, 30, 205, 135, 119, 70, 94, 252, 71, 120, 150
	};
	char encoded[] = "AQABzxG1JBITJUEPH8WVoT8hFjyNG8ojuC_-44eHCzqL0EF_NKfxH2O2GZ9gRkeg-6hLhLlw5sGF_Cp-Qlt5PIdPGLnSHMeF__BxZUPHF-G1oHmMQ3uh5biJHs2Hd0Ze_Ed4lg";
	ASSERT_EQ(string(original, NELEMS(original)), Base64Decode(string(encoded)));
}
