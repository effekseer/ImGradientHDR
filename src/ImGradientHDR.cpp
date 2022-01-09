#include "ImGradientHDR.h"

#include <imgui.h>
#include <vector>
#include <algorithm>

std::array<float, 4> ImGradientHDRState::GetColor(float x) const
{
	auto getColor = [&](float x)
	{
		if (ColorCount == 0)
		{
			return std::array<float, 3>{1.0f, 1.0f, 1.0f};
		}

		if (x < ColorPositions[0])
		{
			return Colors[0];
		}

		if (ColorPositions[ColorCount - 1] <= x)
		{
			return Colors[ColorCount - 1];
		}

		for (int i = 0; i < ColorCount - 1; i++)
		{
			if (ColorPositions[i] <= x && x < ColorPositions[i + 1])
			{
				const auto area = ColorPositions[i + 1] - ColorPositions[i];
				const auto alpha = (x - ColorPositions[i]) / area;
				const auto r = Colors[i + 1][0] * alpha + Colors[i][0] * (1.0f - alpha);
				const auto g = Colors[i + 1][1] * alpha + Colors[i][1] * (1.0f - alpha);
				const auto b = Colors[i + 1][2] * alpha + Colors[i][2] * (1.0f - alpha);
				return std::array<float, 3>{ r, g, b };
			}
		}

		return std::array<float, 3>{1.0f, 1.0f, 1.0f};
	};

	auto getAlpha = [&](float x)
	{
		if (AlphaCount == 0)
		{
			return 1.0f;
		}

		if (x < AlphaPositions[0])
		{
			return 1.0f;
		}

		if (AlphaPositions[ColorCount - 1] <= x)
		{
			return AlphaPositions[ColorCount - 1];
		}

		for (int i = 0; i < AlphaCount - 1; i++)
		{
			if (AlphaPositions[i] <= x && x < AlphaPositions[i + 1])
			{
				const auto area = AlphaPositions[i + 1] - AlphaPositions[i];
				const auto alpha = (x - AlphaPositions[i]) / area;
				return AlphaPositions[i + 1] * alpha + AlphaPositions[i] * (1.0f - alpha);
			}
		}

		return 1.0f;
	};

	const auto c = getColor(x);
	return std::array<float, 4>{c[0], c[1], c[2], getAlpha(x)};
}

bool ImGradientHDR(ImGradientHDRState& state)
{
	const auto originPos = ImGui::GetCursorScreenPos();

	auto drawList = ImGui::GetWindowDrawList();

	const int height = 20;

	ImGui::InvisibleButton("Bar", { 100,height });

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		// TODO Add marker
	}

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

		for (size_t i = 0; i < xkeys.size() - 1; i++)
		{
			const auto c1 = state.GetColor(xkeys[i]);
			const auto c2 = state.GetColor(xkeys[i + 1]);

			const auto colorAU32 = ImGui::ColorConvertFloat4ToU32({ c1[0], c1[1], c1[2], c1[3] });
			const auto colorBU32 = ImGui::ColorConvertFloat4ToU32({ c2[0], c2[1], c2[2], c2[3] });

			drawList->AddRectFilledMultiColor(ImVec2(originPos.x + xkeys[i] * 100, originPos.y),
				ImVec2(originPos.x + xkeys[i + 1] * 100, originPos.y + height),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}
	}

	for (int i = 0; i < state.ColorCount; i++)
	{
		// TODO move marker
		const auto x = state.ColorPositions[i] * 100;
		const auto c = state.Colors[i];

		drawList->AddTriangleFilled(
			{ originPos.x + x, originPos.y + height },
			{ originPos.x + x - 10, originPos.y + height + 10 },
			{ originPos.x + x + 10, originPos.y + height + 10 },
			ImGui::ColorConvertFloat4ToU32({ c[0], c[1], c[2], 1.0f }));
	}

	for (int i = 0; i < state.AlphaCount; i++)
	{
		// TODO add marker
	}

	return true;
}
