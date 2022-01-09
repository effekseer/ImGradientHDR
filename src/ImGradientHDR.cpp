#include "ImGradientHDR.h"

#include <imgui.h>
#include <vector>
#include <algorithm>

bool ImGradientHDR(ImGradientHDRState& state)
{
	const auto originPos = ImGui::GetCursorScreenPos();

	auto drawList = ImGui::GetWindowDrawList();

	const int height = 20;

	ImGui::InvisibleButton("Bar", { 100,height });

	drawList->AddRectFilled(ImVec2(originPos.x - 2, originPos.y - 2),
		ImVec2(originPos.x + 100 + 2, originPos.y + height + 2),
		IM_COL32(100, 100, 100, 255));

	if (state.ColorCount == 0 && state.AlphaCount == 0)
	{

	}
	else
	{
		std::vector<float> xkeys;
		xkeys.reserve(16);

		for (int32_t i = 0; i < state.ColorCount; i++)
		{
			xkeys.emplace_back(state.ColorPositions[i]);
		}

		for (int32_t i = 0; i < state.AlphaCount; i++)
		{
			xkeys.emplace_back(state.AlphaPositions[i]);
		}

		xkeys.emplace_back(0.0f);
		xkeys.emplace_back(1.0f);

		auto result = std::unique(xkeys.begin(), xkeys.end());
		xkeys.erase(result, xkeys.end());

		std::sort(xkeys.begin(), xkeys.end());

		auto getColor = [&](float x)
		{
			if (state.ColorCount == 0)
			{
				return std::array<float, 3>{1.0f, 1.0f, 1.0f};
			}

			if (x < state.ColorPositions[0])
			{
				return state.Colors[0];
			}

			if (state.ColorPositions[state.ColorCount - 1] <= x)
			{
				return state.Colors[state.ColorCount - 1];
			}

			for (int i = 0; i < state.ColorCount - 1; i++)
			{
				if (state.ColorPositions[i] <= x && x < state.ColorPositions[i + 1])
				{
					const auto area = state.ColorPositions[i + 1] - state.ColorPositions[i];
					const auto alpha = (x - state.ColorPositions[i]) / area;
					const auto r = state.Colors[i + 1][0] * alpha + state.Colors[i][0] * (1.0f - alpha);
					const auto g = state.Colors[i + 1][1] * alpha + state.Colors[i][1] * (1.0f - alpha);
					const auto b = state.Colors[i + 1][2] * alpha + state.Colors[i][2] * (1.0f - alpha);
					return std::array<float, 3>{ r, g, b };
				}
			}

			return std::array<float, 3>{1.0f, 1.0f, 1.0f};
		};

		auto getAlpha = [&](float x)
		{
			if (state.AlphaCount == 0)
			{
				return 1.0f;
			}

			if (x < state.AlphaPositions[0])
			{
				return 1.0f;
			}

			if (state.AlphaPositions[state.ColorCount - 1] <= x)
			{
				return state.AlphaPositions[state.ColorCount - 1];
			}

			for (int i = 0; i < state.AlphaCount - 1; i++)
			{
				if (state.AlphaPositions[i] <= x && x < state.AlphaPositions[i + 1])
				{
					const auto area = state.AlphaPositions[i + 1] - state.AlphaPositions[i];
					const auto alpha = (x - state.AlphaPositions[i]) / area;
					return state.AlphaPositions[i + 1] * (1.0f - alpha) + state.AlphaPositions[i] * alpha;
				}
			}

			return 1.0f;
		};

		auto getRgba = [&](float x)
		{
			const auto c = getColor(x);
			return std::array<float, 4>{c[0], c[1], c[2], getAlpha(x)};
		};

		for (size_t i = 0; i < xkeys.size() - 1; i++)
		{
			const auto c1 = getRgba(xkeys[i]);
			const auto c2 = getRgba(xkeys[i + 1]);

			const auto colorAU32 = ImGui::ColorConvertFloat4ToU32({ c1[0], c1[1], c1[2], c1[3] });
			const auto colorBU32 = ImGui::ColorConvertFloat4ToU32({ c2[0], c2[1], c2[2], c2[3] });

			drawList->AddRectFilledMultiColor(ImVec2(originPos.x + xkeys[i] * 100, originPos.y),
				ImVec2(originPos.x + xkeys[i + 1] * 100, originPos.y + height),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}
	}

	return true;
}
