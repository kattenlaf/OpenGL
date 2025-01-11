#include "Test.h"

namespace test {
	// Pass test pointer by reference to update current test
	TestMenu::TestMenu(Test*& currentTestPointer) : m_CurrentTest(currentTestPointer) {

	}

	void TestMenu::OnImGuiRender() {
		for (auto& test : m_Tests) {
			if (ImGui::Button(test.first.c_str())) {
				m_CurrentTest = test.second();
			}
		}
	}
}