#include "ImGradientHDR.h"

#include <imgui.h>
#include <vector>
#include <algorithm>

bool ImGradientHDRState::AddColorMarker(float x, std::array<float, 3> color, float intencity)
{
	if (ColorCount >= MarkerMax)
	{
		return false;
	}

	const auto lb = std::lower_bound(
		Colors.begin(),
		Colors.begin() + ColorCount,
		ColorMarker{ x, color, intencity },
		[&](const ColorMarker& a, const ColorMarker& b) -> bool { return a.Position < b.Position; });

	if (lb != Colors.end())
	{
		const auto ind = lb - Colors.begin();
		std::copy(Colors.begin() + ind, Colors.begin() + ColorCount, Colors.begin() + ind + 1);
		*(Colors.begin() + ind) = ColorMarker{ x, color, intencity };
		ColorCount++;
	}

	return true;
}

std::array<float, 4> ImGradientHDRState::GetColor(float x) const
{
	auto getColor = [&](float x)
	{
		if (ColorCount == 0)
		{
			return std::array<float, 3>{1.0f, 1.0f, 1.0f};
		}

		if (x < Colors[0].Position)
		{
			return Colors[0].Color;
		}

		if (Colors[ColorCount - 1].Position <= x)
		{
			return Colors[ColorCount - 1].Color;
		}

		for (int i = 0; i < ColorCount - 1; i++)
		{
			if (Colors[i].Position <= x && x < Colors[i + 1].Position)
			{
				const auto area = Colors[i + 1].Position - Colors[i].Position;
				const auto alpha = (x - Colors[i].Position) / area;
				const auto r = Colors[i + 1].Color[0] * alpha + Colors[i].Color[0] * (1.0f - alpha);
				const auto g = Colors[i + 1].Color[1] * alpha + Colors[i].Color[1] * (1.0f - alpha);
				const auto b = Colors[i + 1].Color[2] * alpha + Colors[i].Color[2] * (1.0f - alpha);
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

		if (x < Alphas[0].Position)
		{
			return 1.0f;
		}

		if (Alphas[ColorCount - 1].Position <= x)
		{
			return Alphas[ColorCount - 1].Alpha;
		}

		for (int i = 0; i < AlphaCount - 1; i++)
		{
			if (Alphas[i].Position <= x && x < Alphas[i + 1].Position)
			{
				const auto area = Alphas[i + 1].Position - Alphas[i].Position;
				const auto alpha = (x - Alphas[i].Position) / area;
				return Alphas[i + 1].Alpha * alpha + Alphas[i].Alpha * (1.0f - alpha);
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

	const auto margin = 5;
	const auto width = ImGui::GetContentRegionAvail().x - margin * 2;
	const int height = 20;

	ImGui::InvisibleButton("Bar", { width,height });

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
	{
		float x = (ImGui::GetIO().MousePos.x - originPos.x) / width;
		const auto c = state.GetColor(x);
		state.AddColorMarker(x, { c[0], c[1], c[2] }, 1.0f);
	}

	drawList->AddRectFilled(ImVec2(originPos.x - 2, originPos.y - 2),
		ImVec2(originPos.x + width + 2, originPos.y + height + 2),
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
			xkeys.emplace_back(state.Colors[i].Position);
		}

		for (int32_t i = 0; i < state.AlphaCount; i++)
		{
			xkeys.emplace_back(state.Alphas[i].Position);
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

			drawList->AddRectFilledMultiColor(ImVec2(originPos.x + xkeys[i] * width, originPos.y),
				ImVec2(originPos.x + xkeys[i + 1] * width, originPos.y + height),
				colorAU32, colorBU32, colorBU32, colorAU32);
		}
	}

	for (int i = 0; i < state.ColorCount; i++)
	{
		// TODO move marker
		const auto x = state.Colors[i].Position * width;
		const auto c = state.Colors[i].Color;

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
