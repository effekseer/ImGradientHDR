#include "ImGradientHDR.h"

#include <imgui.h>

bool ImGradientHDR(ImGradientHDRState& state)
{
	const auto originPos = ImGui::GetCursorScreenPos();

	auto drawList = ImGui::GetWindowDrawList();

	ImGui::InvisibleButton("Bar", { 100,100 });

	drawList->AddRectFilled(ImVec2(originPos.x - 2, originPos.y - 2),
		ImVec2(originPos.x + 100 + 2, originPos.y + 100 + 2),
		IM_COL32(100, 100, 100, 255));

	return true;
}
