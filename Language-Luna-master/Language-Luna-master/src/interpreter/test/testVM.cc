#include <string>
#include <iostream>

#include "gtest/gtest.h"
#include "virtual_machine.h"

class TestFrame : public ::testing::Test {
protected:
	//*.luo file root_path
	std::string ROOT_PATH;

	TestFrame(){
		//macro defination ROOTPATH was generated by cmake system
		ROOT_PATH = *new std::string(ROOTPATH);
		ROOT_PATH.append("/examples");
	}
};

TEST_F (TestFrame, LoadInstructions1){
	::testing::internal::CaptureStdout();
	EXPECT_ANY_THROW({
		VirtualMachine vm(ROOT_PATH.append("/invalidfile").c_str());
	});
	std::string capturedStdout = ::testing::internal::GetCapturedStdout().c_str();
}

TEST_F (TestFrame, LoadInstructions2){
	EXPECT_NO_THROW({VirtualMachine vm(ROOT_PATH.append("/example0.luo").c_str());
	});
}